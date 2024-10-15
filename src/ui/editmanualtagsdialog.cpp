#include "editmanualtagsdialog.h"
#include "tablepushbutton.h"
#include "ui_editmanualtagsdialog.h"
#include <QMessageBox>
#include <ranges>
#include <regex>

namespace str = std::ranges;


EditManualTagsDialog::EditManualTagsDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::EditManualTagsDialog)
{
  ui->setupUi(this);
  setWindowTitle("Edit Manual Tags");
  connect(
    ui->tag_table, &QTableWidget::cellChanged, this, &EditManualTagsDialog::onTableCellEdited);

  // This is a hacky way of disabling the ability to accept the dialog by pressing enter.
  // Reason: Prevent user from accidentally closing the dialog while editing a table cell.
  auto ok_button = ui->buttonBox->button(QDialogButtonBox::Ok);
  ok_button->setDefault(false);
  ok_button->setAutoDefault(false);
  ui->buttonBox->addButton(QDialogButtonBox::Reset);
  auto dummy_button = ui->buttonBox->button(QDialogButtonBox::Reset);
  dummy_button->setText("");
  dummy_button->setDefault(true);
  dummy_button->setAutoDefault(true);
  dummy_button->setHidden(true);
}

EditManualTagsDialog::~EditManualTagsDialog()
{
  delete ui;
}

void EditManualTagsDialog::setupDialog(int app_id,
                                       const QStringList& tag_names,
                                       const std::vector<int> num_mods_per_tag)
{
  app_id_ = app_id;
  actions_.clear();
  tag_names_ = tag_names;
  num_mods_per_tag_ = num_mods_per_tag;
  for(int i = num_mods_per_tag_.size(); i < tag_names_.size(); i++)
    num_mods_per_tag_.push_back(0);
  updateTable();
  dialog_completed_ = false;
}

void EditManualTagsDialog::closeEvent(QCloseEvent* event)
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit dialogClosed();
  QDialog::reject();
}

void EditManualTagsDialog::reject()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit dialogClosed();
  QDialog::reject();
}

void EditManualTagsDialog::updateTable()
{
  ui->tag_table->blockSignals(true);
  ui->tag_table->setRowCount(0);
  ui->tag_table->setRowCount(tag_names_.size() + 1);
  for(const auto& [i, tuple] : str::enumerate_view(str::zip_view(tag_names_, num_mods_per_tag_)))
  {
    const auto& [name, mods] = tuple;

    auto remove_button = new TablePushButton(i, ACTION_COL);
    remove_button->setIcon(QIcon::fromTheme("user-trash"));
    remove_button->setToolTip("Remove Tag");
    remove_button->adjustSize();
    connect(remove_button, &TablePushButton::clickedAt, this, &EditManualTagsDialog::onTagRemoved);
    ui->tag_table->setCellWidget(i, ACTION_COL, remove_button);

    auto name_item = new QTableWidgetItem(name);
    name_item->setFlags(name_item->flags() | Qt::ItemIsEditable);
    ui->tag_table->setItem(i, NAME_COL, name_item);

    auto mod_item = new QTableWidgetItem(QString::number(mods));
    mod_item->setFlags(name_item->flags() & ~Qt::ItemIsEditable);
    ui->tag_table->setItem(i, NUM_MODS_COL, mod_item);
  }
  const int last_row = ui->tag_table->rowCount() - 1;
  auto add_button = new QPushButton(ui->tag_table);
  add_button->setIcon(QIcon::fromTheme("list-add"));
  add_button->setToolTip("Add Tag");
  add_button->adjustSize();
  connect(add_button, &QPushButton::clicked, this, &EditManualTagsDialog::onTagAdded);
  ui->tag_table->setCellWidget(last_row, ACTION_COL, add_button);
  auto name_item = new QTableWidgetItem("");
  name_item->setFlags(name_item->flags() & ~Qt::ItemIsEditable);
  ui->tag_table->setItem(last_row, NAME_COL, name_item);
  auto mod_item = new QTableWidgetItem("");
  mod_item->setFlags(name_item->flags() & ~Qt::ItemIsEditable);
  ui->tag_table->setItem(last_row, NUM_MODS_COL, mod_item);

  ui->tag_table->setColumnWidth(ACTION_COL, 55);
  ui->tag_table->resizeColumnToContents(NAME_COL);
  ui->tag_table->blockSignals(false);
}

void EditManualTagsDialog::onTagRemoved(int row, int col)
{
  tag_names_.removeAt(row);
  num_mods_per_tag_.erase(num_mods_per_tag_.begin() + row);
  actions_.emplace_back(ui->tag_table->item(row, NAME_COL)->text().toStdString(),
                        EditManualTagAction::ActionType::remove);
  updateTable();
}

void EditManualTagsDialog::onTagAdded()
{
  int new_tag_id = 1;
  std::regex name_regex(R"(New Tag (\d+))");
  std::smatch match;
  for(const auto& tag_name : tag_names_)
  {
    const std::string name_string = tag_name.toStdString();
    if(std::regex_search(name_string, match, name_regex))
      new_tag_id = std::max(new_tag_id, std::stoi(match[1].str()) + 1);
  }
  tag_names_.append("New Tag " + QString::number(new_tag_id));
  num_mods_per_tag_.push_back(0);
  actions_.emplace_back(tag_names_.last().toStdString(), EditManualTagAction::ActionType::add);
  updateTable();
}

void EditManualTagsDialog::onTableCellEdited(int row, int col)
{
  if(row < 0 || row >= tag_names_.size())
    return;
  auto new_name = ui->tag_table->item(row, col)->text();
  if(tag_names_.contains(new_name))
  {
    QMessageBox::critical(
      this,
      "Renaming Failed",
      std::format("A tag with the name '{}' already exists.", new_name.toStdString()).c_str());
    updateTable();
  }
  actions_.emplace_back(
    tag_names_[row].toStdString(), EditManualTagAction::ActionType::rename, new_name.toStdString());
  tag_names_[row] = new_name;
  ui->tag_table->resizeColumnToContents(NAME_COL);
}

void EditManualTagsDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  if(!actions_.empty())
    emit manualTagsEdited(app_id_, actions_);

  for(auto action : actions_)
  {
    QString type_str;
    auto type = action.getType();
    if(type == EditManualTagAction::ActionType::add)
      type_str = "Add";
    else if(type == EditManualTagAction::ActionType::rename)
      type_str = "Rename";
    else
      type_str = "Remove";
  }
}

void EditManualTagsDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  emit dialogClosed();
}
