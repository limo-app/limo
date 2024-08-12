#include "editautotagsdialog.h"
#include "addautotagdialog.h"
#include "core/tagconditionnode.h"
#include "tablepushbutton.h"
#include "ui_editautotagsdialog.h"
#include <QMenu>
#include <QRegularExpressionValidator>
#include <ranges>

namespace str = std::ranges;


EditAutoTagsDialog::EditAutoTagsDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::EditAutoTagsDialog)
{
  add_tag_action_ = std::make_unique<QAction>("Add");
  add_tag_action_->setIcon(QIcon::fromTheme("list-add"));
  connect(add_tag_action_.get(), &QAction::triggered, this, &EditAutoTagsDialog::onTagAdded);
  remove_tag_action_ = std::make_unique<QAction>("Remove");
  remove_tag_action_->setIcon(QIcon::fromTheme("user-trash"));
  connect(remove_tag_action_.get(), &QAction::triggered, this, &EditAutoTagsDialog::onTagRemoved);
  rename_tag_action_ = std::make_unique<QAction>("Rename");
  rename_tag_action_->setIcon(QIcon::fromTheme("preferences-other"));
  connect(rename_tag_action_.get(), &QAction::triggered, this, &EditAutoTagsDialog::onTagRenamed);
  auto tag_menu = new QMenu();
  tag_menu->addActions(
    { add_tag_action_.get(), rename_tag_action_.get(), remove_tag_action_.get() });

  ui->setupUi(this);
  setWindowTitle("Edit Auto Tags");
  ui->connection_cb->addItem("ANY must match");
  ui->connection_cb->addItem("ALL must match");
  ui->connection_cb->addItem("Advanced");
  connect(
    ui->condition_table, &QTableWidget::cellChanged, this, &EditAutoTagsDialog::onConditionEdited);
  connect(
    ui->tag_cb, &QComboBox::currentTextChanged, this, &EditAutoTagsDialog::updateConditionTable);

  ui->tag_tool_button->setDefaultAction(add_tag_action_.get());
  ui->tag_tool_button->setMenu(tag_menu);

  const QRegularExpression regex(R"([\d NOTADRnotadr()]*)");
  auto key_validator = new QRegularExpressionValidator(regex, this);
  ui->expression_line_edit->setValidator(key_validator);
  ui->expression_line_edit->setValidationMode(ValidatingLineEdit::VALID_CUSTOM);
  auto expression_validator = [this](const QString& exp)
  {
    return TagConditionNode::expressionIsValid(exp.toStdString(),
                                               this->ui->condition_table->rowCount() - 1);
  };
  ui->expression_line_edit->setCustomValidator(expression_validator);
}

EditAutoTagsDialog::~EditAutoTagsDialog()
{
  delete ui;
}

void EditAutoTagsDialog::setupDialog(
  int app_id,
  const std::map<std::string, std::pair<std::string, std::vector<TagCondition>>>& auto_tags)
{
  tags_with_updated_conditions_.clear();
  connection_index_map_.clear();
  edit_actions_.clear();
  app_id_ = app_id;
  auto_tags_ = auto_tags;
  ui->tag_cb->clear();
  for(const auto& [tag, _] : auto_tags)
    ui->tag_cb->addItem(tag.c_str());
  if(!auto_tags.empty())
    ui->tag_cb->setCurrentIndex(0);
  enableInteraction(!auto_tags.empty());
  updateConditionTable();
  updateExpressionBox();
  updateOkButton();
  dialog_completed_ = false;
}

void EditAutoTagsDialog::closeEvent(QCloseEvent* event)
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit dialogClosed();
  QDialog::closeEvent(event);
}

void EditAutoTagsDialog::updateOkButton()
{
  bool enable = ui->condition_table->rowCount() > 1;
  if(ui->connection_cb->currentIndex() == connection_cb_advanced_index)
    enable = enable && ui->expression_line_edit->hasValidText();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
  ui->tag_cb->setEnabled(enable);
  add_tag_action_->setEnabled(enable);
  if(!enable)
    ui->tag_tool_button->setDefaultAction(remove_tag_action_.get());
  else
    ui->tag_tool_button->setDefaultAction(add_tag_action_.get());
  if(ui->tag_cb->count() == 0)
  {
    add_tag_action_->setEnabled(true);
    ui->tag_tool_button->setDefaultAction(add_tag_action_.get());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  }
}

void EditAutoTagsDialog::enableInteraction(bool enable)
{
  ui->connection_cb->setEnabled(enable);
  ui->condition_table->setEnabled(enable);
  ui->expression_line_edit->setEnabled(enable);
  remove_tag_action_->setEnabled(enable);
  rename_tag_action_->setEnabled(enable);
}

void EditAutoTagsDialog::updateConditionTable()
{
  ui->condition_table->blockSignals(true);
  ui->condition_table->setRowCount(0);
  if(auto_tags_.empty())
    return;

  auto conditions = auto_tags_[ui->tag_cb->currentText().toStdString()].second;
  ui->condition_table->setRowCount(conditions.size() + 1);
  for(const auto& [i, condition] : str::enumerate_view(conditions))
  {
    const auto& [invert, type, use_regex, search_string] = condition;

    auto remove_button = new TablePushButton(i, action_col);
    remove_button->setIcon(QIcon::fromTheme("user-trash"));
    remove_button->setToolTip("Remove condition");
    remove_button->adjustSize();
    connect(
      remove_button, &TablePushButton::clickedAt, this, &EditAutoTagsDialog::onConditionRemoved);
    ui->condition_table->setCellWidget(i, action_col, remove_button);

    auto id_item = new QTableWidgetItem(QString::number(i));
    id_item->setFlags(id_item->flags() & ~Qt::ItemIsEditable);
    ui->condition_table->setItem(i, id_col, id_item);

    auto target_cb = new QComboBox();
    target_cb->addItem("File name");
    target_cb->addItem("Full path");
    target_cb->setCurrentIndex(type == TagCondition::Type::file_name ? target_file_index
                                                                     : target_path_index);
    target_cb->installEventFilter(this);
    connect(
      target_cb, &QComboBox::currentTextChanged, this, &EditAutoTagsDialog::onConditionEdited);
    ui->condition_table->setCellWidget(i, target_col, target_cb);

    auto invert_cb = new QComboBox();
    invert_cb->addItem("Does match");
    invert_cb->addItem("Does not match");
    invert_cb->setCurrentIndex(invert ? invert_true_index : invert_false_index);
    invert_cb->installEventFilter(this);
    connect(
      invert_cb, &QComboBox::currentTextChanged, this, &EditAutoTagsDialog::onConditionEdited);
    ui->condition_table->setCellWidget(i, invert_col, invert_cb);

    auto matcher_cb = new QComboBox();
    matcher_cb->addItem("String");
    matcher_cb->addItem("Regex");
    matcher_cb->setCurrentIndex(use_regex ? matcher_regex_index : matcher_string_index);
    matcher_cb->installEventFilter(this);
    connect(
      matcher_cb, &QComboBox::currentTextChanged, this, &EditAutoTagsDialog::onConditionEdited);
    ui->condition_table->setCellWidget(i, matcher_col, matcher_cb);

    auto string_item = new QTableWidgetItem(search_string.c_str());
    string_item->setFlags(string_item->flags() | Qt::ItemIsEditable);
    ui->condition_table->setItem(i, string_col, string_item);
  }
  auto add_condition_button = new QPushButton();
  add_condition_button->setIcon(QIcon::fromTheme("list-add"));
  add_condition_button->setToolTip("Add condition");
  add_condition_button->adjustSize();
  connect(add_condition_button, &QPushButton::clicked, this, &EditAutoTagsDialog::onConditionAdded);
  ui->condition_table->setCellWidget(
    ui->condition_table->rowCount() - 1, action_col, add_condition_button);
  for(int i = 2; i < ui->condition_table->columnCount(); i++)
  {
    auto dummy_item = new QTableWidgetItem();
    dummy_item->setFlags(dummy_item->flags() & ~Qt::ItemIsEditable);
    ui->condition_table->setItem(ui->condition_table->rowCount() - 1, i, dummy_item);
  }
  ui->condition_table->blockSignals(false);
  ui->condition_table->setColumnWidth(action_col, 55);
  ui->condition_table->resizeColumnToContents(id_col);
  ui->condition_table->setColumnWidth(target_col, 85);
  ui->condition_table->setColumnWidth(invert_col, 120);
  ui->condition_table->setColumnWidth(matcher_col, 70);
}

bool EditAutoTagsDialog::eventFilter(QObject* object, QEvent* event)
{
  if(event->type() == QEvent::Wheel)
    return true;
  return QObject::eventFilter(object, event);
}

QString EditAutoTagsDialog::createExpression(QString op, int length)
{
  QStringList expression;
  expression.reserve(length);
  for(int i = 0; i < length; i++)
    expression.append(QString::number(i));
  return expression.join(op);
}

void EditAutoTagsDialog::updateExpressionBox()
{
  const QString tag = ui->tag_cb->currentText();
  if(connection_index_map_.contains(tag) &&
     connection_index_map_[tag] != connection_cb_advanced_index)
  {
    ui->connection_cb->setCurrentIndex(connection_index_map_[tag]);
    return;
  }

  const int num_operators = auto_tags_[ui->tag_cb->currentText().toStdString()].second.size();
  const QString expression = auto_tags_[ui->tag_cb->currentText().toStdString()].first.c_str();
  ui->expression_line_edit->setText("");
  if(expression == "" || expression == createExpression("or", num_operators))
    ui->connection_cb->setCurrentIndex(connection_cb_any_index);
  else if(expression == createExpression("and", num_operators))
    ui->connection_cb->setCurrentIndex(connection_cb_all_index);
  else
  {
    ui->connection_cb->setCurrentIndex(connection_cb_advanced_index);
    ui->expression_line_edit->setText(expression);
    ui->expression_line_edit->updateValidation();
  }
  if(!connection_index_map_.contains(tag))
    connection_index_map_[tag] = ui->connection_cb->currentIndex();
}

void EditAutoTagsDialog::onConditionRemoved(int row, int col)
{
  const std::string current_tag = ui->tag_cb->currentText().toStdString();
  auto& conditions = auto_tags_[current_tag].second;
  if(row >= conditions.size())
    return;
  conditions.erase(conditions.begin() + row);
  tags_with_updated_conditions_.insert(current_tag);
  updateConditionTable();
  updateOkButton();
}

void EditAutoTagsDialog::onConditionAdded()
{
  const std::string current_tag = ui->tag_cb->currentText().toStdString();
  auto_tags_[ui->tag_cb->currentText().toStdString()].second.emplace_back(
    false, TagCondition::Type::file_name, false, "");
  tags_with_updated_conditions_.insert(current_tag);
  updateConditionTable();
  ui->expression_line_edit->updateValidation();
  updateOkButton();
}

void EditAutoTagsDialog::onConditionEdited()
{
  const std::string current_tag = ui->tag_cb->currentText().toStdString();
  tags_with_updated_conditions_.insert(current_tag);
  std::vector<TagCondition> conditions;
  for(int i = 0; i < ui->condition_table->rowCount() - 1; i++)
  {
    const bool invert =
      static_cast<QComboBox*>(ui->condition_table->cellWidget(i, invert_col))->currentIndex() ==
          invert_true_index
        ? true
        : false;
    const TagCondition::Type type =
      static_cast<QComboBox*>(ui->condition_table->cellWidget(i, target_col))->currentIndex() ==
          target_file_index
        ? TagCondition::Type::file_name
        : TagCondition::Type::path;
    const bool use_regex =
      static_cast<QComboBox*>(ui->condition_table->cellWidget(i, matcher_col))->currentIndex() ==
          matcher_regex_index
        ? true
        : false;
    const std::string search_string =
      ui->condition_table->item(i, string_col)->text().toStdString();
    conditions.emplace_back(invert, type, use_regex, search_string);
  }
  auto_tags_[current_tag].second = conditions;
}

void EditAutoTagsDialog::on_connection_cb_currentIndexChanged(int index)
{
  const QString tag = ui->tag_cb->currentText();
  if(index == connection_cb_any_index)
  {
    ui->expression_label->setHidden(true);
    ui->expression_line_edit->setHidden(true);
    ui->help_label->setText("Tag will be applied if ANY of the following conditions is met.");
  }
  else if(index == connection_cb_all_index)
  {
    ui->expression_label->setHidden(true);
    ui->expression_line_edit->setHidden(true);
    ui->help_label->setText("Tag will be applied if ALL of the following conditions are met.");
  }
  else
  {
    ui->expression_label->setHidden(false);
    ui->expression_line_edit->setHidden(false);
    ui->expression_line_edit->setText(
      auto_tags_[ui->tag_cb->currentText().toStdString()].first.c_str());
    ui->help_label->setText(
      "Tag will be applied if the expression defined above is met.\n"
      "Expressions consit of condition IDs connected by the operators [and, or, not].\n"
      "Example for 3 conditions: 1 and not(0 or not 1)");
    ui->expression_line_edit->updateValidation();
  }
  if(!ignore_index_changes_)
    tags_with_updated_conditions_.insert(tag.toStdString());
  connection_index_map_[tag] = index;
  updateOkButton();
}

void EditAutoTagsDialog::onTagRemoved()
{
  if(auto_tags_.empty())
    return;
  const std::string tag = ui->tag_cb->currentText().toStdString();
  ui->tag_cb->removeItem(ui->tag_cb->currentIndex());
  auto_tags_.erase(tag);
  edit_actions_.emplace_back(tag, EditAutoTagAction::ActionType::remove);
  updateOkButton();
}

void EditAutoTagsDialog::on_tag_cb_currentIndexChanged(int index)
{
  ignore_index_changes_ = true;
  enableInteraction(index != -1);
  updateConditionTable();
  updateExpressionBox();
  updateOkButton();
  ui->expression_line_edit->updateValidation();
  ignore_index_changes_ = false;
}

void EditAutoTagsDialog::onTagAdded()
{
  QStringList tags;
  for(int i = 0; i < ui->tag_cb->count(); i++)
    tags.append(ui->tag_cb->itemText(i));
  auto dialog = AddAutoTagDialog(tags);
  int ret = dialog.exec();
  if(ret != QDialog::Accepted)
    return;
  QString new_tag = dialog.getName();
  ui->tag_cb->addItem(new_tag);
  auto_tags_[new_tag.toStdString()] = { "", {} };
  ui->tag_cb->setCurrentIndex(ui->tag_cb->count() - 1);
  edit_actions_.emplace_back(new_tag.toStdString(), EditAutoTagAction::ActionType::add);
  connection_index_map_[new_tag] = connection_cb_any_index;
}

void EditAutoTagsDialog::onTagRenamed()
{
  QStringList tags;
  for(int i = 0; i < ui->tag_cb->count(); i++)
    tags.append(ui->tag_cb->itemText(i));
  const QString current_name = ui->tag_cb->currentText();
  auto dialog = AddAutoTagDialog(std::move(tags), current_name);
  int ret = dialog.exec();
  const QString new_name = dialog.getName();
  if(ret != QDialog::Accepted || new_name == current_name)
    return;
  auto_tags_[new_name.toStdString()] = auto_tags_[current_name.toStdString()];
  auto_tags_.erase(current_name.toStdString());
  ui->tag_cb->setItemText(ui->tag_cb->currentIndex(), new_name);
  if(tags_with_updated_conditions_.contains(current_name.toStdString()))
  {
    tags_with_updated_conditions_.insert(new_name.toStdString());
    tags_with_updated_conditions_.erase(current_name.toStdString());
  }
  if(connection_index_map_.contains(current_name))
  {
    connection_index_map_[new_name] = connection_index_map_[current_name];
    connection_index_map_.erase(current_name);
  }
  edit_actions_.emplace_back(current_name.toStdString(), new_name.toStdString());
}

void EditAutoTagsDialog::on_expression_line_edit_textEdited(const QString& expression)
{
  updateOkButton();
  if(!ui->expression_line_edit->hasValidText())
    return;
  const std::string current_tag = ui->tag_cb->currentText().toStdString();
  tags_with_updated_conditions_.insert(current_tag);
  auto_tags_[current_tag].first = ui->expression_line_edit->text().toStdString();
}

void EditAutoTagsDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  for(const auto& [tag, pair] : auto_tags_)
  {
    if(!tags_with_updated_conditions_.contains(tag))
      continue;
    const auto& [expression, conditions] = auto_tags_[tag];
    if(connection_index_map_.contains(tag.c_str()) &&
       connection_index_map_[tag.c_str()] == connection_cb_all_index)
    {
      edit_actions_.emplace_back(
        tag, createExpression("and", conditions.size()).toStdString(), conditions);
    }
    else if(connection_index_map_.contains(tag.c_str()) &&
            connection_index_map_[tag.c_str()] == connection_cb_any_index)
    {
      edit_actions_.emplace_back(
        tag, createExpression("or", conditions.size()).toStdString(), conditions);
    }
    else
    {
      if(!TagConditionNode::expressionIsValid(expression, conditions.size()))
        continue;
      edit_actions_.emplace_back(tag, expression, conditions);
    }
  }
  if(!edit_actions_.empty() && !auto_tags_.empty())
    emit tagsEdited(app_id_, edit_actions_);
  else
    emit dialogClosed();
}

void EditAutoTagsDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  emit dialogClosed();
}
