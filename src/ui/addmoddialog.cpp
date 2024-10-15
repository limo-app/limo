#include "addmoddialog.h"
#include "../core/installer.h"
#include "../core/log.h"
#include "fomoddialog.h"
#include "qdebug.h"
#include "ui_addmoddialog.h"
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QTreeWidget>
#include <ranges>
#include <regex>
#include <set>

namespace sfs = std::filesystem;
namespace str = std::ranges;


AddModDialog::AddModDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AddModDialog)
{
  ui->setupUi(this);
  fomod_dialog_ = std::make_unique<FomodDialog>();
  connect(
    fomod_dialog_.get(), &FomodDialog::addModAccepted, this, &AddModDialog::onFomodDialogComplete);
  connect(fomod_dialog_.get(), &FomodDialog::addModAborted, this, &AddModDialog::onFomodDialogAborted);
  auto options_frame = new QFrame();
  auto grid = new QGridLayout();
  options_frame->setLayout(grid);
  ui->options_container->setWidget(options_frame);
  for(int i = 0; i < Installer::OPTION_GROUPS.size(); i++)
  {
    auto button_group = new QButtonGroup;
    option_groups_.append(button_group);
    auto box = new QGroupBox();
    auto layout = new QVBoxLayout();
    bool is_first = true;
    for(const auto option : Installer::OPTION_GROUPS[i])
    {
      auto button = new QRadioButton(Installer::OPTION_NAMES.at(option).c_str());
      button->setToolTip(Installer::OPTION_DESCRIPTIONS.at(option).c_str());
      if(is_first)
      {
        button->setChecked(true);
        is_first = false;
      }
      button_group->addButton(button, option);
      layout->addWidget(button);
    }
    box->setLayout(layout);
    grid->addWidget(box, i / 2, i % 2);
  }
  auto group_validator = [groups = &groups_](QString s) { return groups->contains(s); };
  ui->group_field->setCustomValidator(group_validator);
  ui->group_field->setValidationMode(ValidatingLineEdit::VALID_CUSTOM);
}

AddModDialog::~AddModDialog()
{
  delete ui;
}

void AddModDialog::updateOkButton()
{
  ui->buttonBox->button(QDialogButtonBox::Ok)
    ->setEnabled(ui->name_text->hasValidText() && ui->version_text->hasValidText() &&
                 ui->group_field->hasValidText());
}

void AddModDialog::colorTreeNodes(QTreeWidgetItem* node, int cur_depth, int root_level)
{
  auto color = cur_depth < root_level ? COLOR_REMOVE_ : COLOR_KEEP_;
  node->setForeground(0, color);
  for(int i = 0; i < node->childCount(); i++)
    colorTreeNodes(node->child(i), cur_depth + 1, root_level);
}

void AddModDialog::showError(const std::runtime_error& error)
{
  std::string message = std::string("Could not open source files: ") + error.what();
  Log::error(message);
  QMessageBox* error_box =
    new QMessageBox(QMessageBox::Critical, "Error", message.c_str(), QMessageBox::Ok);
  error_box->exec();
}

bool AddModDialog::setupDialog(const QString& name,
                               const QStringList& deployers,
                               int cur_deployer,
                               const QStringList& groups,
                               const std::vector<int>& mod_ids,
                               const QString& path,
                               const QStringList& deployer_paths,
                               int app_id,
                               const std::vector<bool>& autonomous_deployers,
                               const QString& app_version,
                               const QString& local_source,
                               const QString& remote_source,
                               int mod_id,
                               const QStringList& mod_names,
                               const QStringList& mod_versions)
{
  ui->name_text->setFocus();
  app_id_ = app_id;
  mod_ids_ = mod_ids;
  mod_path_ = path;
  deployer_paths_ = deployer_paths;
  groups_ = groups;
  app_version_ = app_version;
  local_source_ = local_source;
  remote_source_ = remote_source;
  ui->group_combo_box->setCurrentIndex(ADD_TO_GROUP_INDEX);
  ui->deployer_list->setEnabled(true);
  ui->group_field->clear();
  ui->group_field->setEnabled(false);
  ui->group_field->updateValidation();
  completer_ = std::make_unique<QCompleter>(groups);
  completer_->setCaseSensitivity(Qt::CaseInsensitive);
  completer_->setFilterMode(Qt::MatchContains);
  ui->group_field->setCompleter(completer_.get());
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  ui->group_check->setCheckState(Qt::Unchecked);
  int mod_index = -1;
  if(mod_id != -1)
  {
    auto iter = str::find(mod_ids, mod_id);
    if(iter != mod_ids.end())
    {
      mod_index = iter - mod_ids.begin();
      ui->group_field->setText(groups[mod_index]);
      ui->group_check->setCheckState(Qt::Checked);
      ui->group_combo_box->setCurrentIndex(REPLACE_MOD_INDEX);
    }
  }
  std::regex name_regex(R"(-\d+((?:-[\dA-Za-z]+)+)-\d+\.(?:zip|7z|rar)$)");
  std::smatch match;
  std::string name_str = name.toStdString();
  if(mod_index >= 0 && mod_index < mod_names.size())
  {
    ui->name_text->setText(mod_names[mod_index]);
    ui->version_text->setText(mod_versions[mod_index]);
  }
  else if(std::regex_search(name_str, match, name_regex))
  {
    ui->name_text->setText(match.prefix().str().c_str());
    std::string version_str = match[1].str();
    if(!version_str.empty())
      version_str.erase(version_str.begin());
    std::replace(version_str.begin(), version_str.end(), '-', '.');
    ui->version_text->setText(version_str.c_str());
  }
  else
  {
    ui->version_text->setText("1.0");
    ui->name_text->setText(name);
  }
  ui->installer_box->clear();
  int root_level = 0;
  std::string prefix;
  std::string detected_type;
  try
  {
    auto signature = Installer::detectInstallerSignature(path.toStdString());
    std::tie(root_level, prefix, detected_type) = signature;
  }
  catch(std::runtime_error& error)
  {
    showError(error);
    emit addModAborted(mod_path_);
    return false;
  }
  if(detected_type == Installer::FOMODINSTALLER)
  {
    auto [name, version] =
      fomod::FomodInstaller::getMetaData(sfs::path(mod_path_.toStdString()) / prefix);
    if(!name.empty())
      ui->name_text->setText(name.c_str());
    if(!version.empty())
      ui->version_text->setText(version.c_str());
  }
  path_prefix_ = prefix.c_str();
  int target_idx = 0;
  for(int i = 0; const auto& installer : Installer::INSTALLER_TYPES)
  {
    if(installer == detected_type)
    {
      target_idx = i;
      ui->installer_box->addItem(("[Auto detected] " + installer).c_str());
    }
    else
      ui->installer_box->addItem(installer.c_str());
    i++;
  }
  ui->installer_box->setCurrentIndex(target_idx);
  ui->deployer_list->clear();
  std::set<int> selected_deployers;
  auto settings = QSettings(QCoreApplication::applicationName());
  settings.beginGroup(QString::number(app_id));
  int size = settings.beginReadArray("selected_deployers");
  for(int i = 0; i < size; i++)
  {
    settings.setArrayIndex(i);
    selected_deployers.insert(settings.value("selected").toInt());
  }
  settings.endArray();

  ui->fomod_deployer_box->clear();
  for(int i = 0; i < deployers.size(); i++)
  {
    if(!autonomous_deployers[i])
      ui->fomod_deployer_box->addItem(deployers[i]);
    auto item = new QListWidgetItem(deployers[i], ui->deployer_list);
    item->setCheckState((selected_deployers.contains(i) | (i == cur_deployer)) ? Qt::Checked
                                                                               : Qt::Unchecked);
    item->setHidden(autonomous_deployers[i]);
  }

  int fomod_target_deployer = settings.value("fomod_target_deployer", -1).toInt();
  if(fomod_target_deployer >= 0 && fomod_target_deployer < ui->fomod_deployer_box->count())
    ui->fomod_deployer_box->setCurrentIndex(fomod_target_deployer);
  else if(cur_deployer >= 0 && cur_deployer < ui->fomod_deployer_box->count())
    ui->fomod_deployer_box->setCurrentIndex(cur_deployer);
  settings.endGroup();

  try
  {
    auto paths = Installer::getArchiveFileNames(path.toStdString());
    int max_depth = 0;
    ui->content_tree->clear();
    for(const auto& path : paths)
      max_depth = std::max(addTreeNode(ui->content_tree, path), max_depth);
    ui->root_level_box->setMaximum(std::max(max_depth - 1, 0));
    ui->root_level_box->setValue(root_level);
    on_root_level_box_valueChanged(root_level);
  }
  catch(std::runtime_error& error)
  {
    showError(error);
    emit addModAborted(mod_path_);
    return false;
  }
  dialog_completed_ = false;
  return true;
}

void AddModDialog::closeEvent(QCloseEvent* event)
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit addModAborted(mod_path_);
  QDialog::reject();
}

void AddModDialog::reject()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit addModAborted(mod_path_);
  QDialog::reject();
}

sfs::path AddModDialog::removeRoot(const sfs::path& source)
{
  sfs::path result;
  bool is_first = true;
  for(auto it = source.begin(); it != source.end(); it++)
  {
    if(!is_first)
      result /= *it;
    is_first = false;
  }
  return result;
}

int AddModDialog::addTreeNode(QTreeWidgetItem* parent, const sfs::path& cur_path)
{
  if(cur_path.empty())
    return 0;
  QString cur_text{ (*(cur_path.begin())).c_str() };
  for(int i = 0; i < parent->childCount(); i++)
  {
    auto cur_child = parent->child(i);
    if(cur_child->text(0) == cur_text)
      return addTreeNode(cur_child, removeRoot(cur_path)) + 1;
  }
  auto child = new QTreeWidgetItem(parent);
  child->setText(0, cur_text);
  child->setForeground(0, COLOR_KEEP_);
  return addTreeNode(child, removeRoot(cur_path)) + 1;
}

int AddModDialog::addTreeNode(QTreeWidget* tree, const sfs::path& cur_path)
{
  if(cur_path.empty())
    return 0;
  QString cur_text{ (*(cur_path.begin())).c_str() };
  for(int i = 0; i < tree->topLevelItemCount(); i++)
  {
    auto cur_item = tree->topLevelItem(i);
    if(cur_item->text(0) == cur_text)
      return addTreeNode(cur_item, removeRoot(cur_path)) + 1;
  }
  auto item = new QTreeWidgetItem(tree);
  item->setText(0, cur_text);
  item->setForeground(0, COLOR_KEEP_);
  return addTreeNode(item, removeRoot(cur_path)) + 1;
}

void AddModDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  int options = 0;
  for(const auto group : static_cast<const QList<QButtonGroup*>>(option_groups_))
    options |= group->checkedId();
  const bool replace_mod = ui->group_combo_box->currentIndex() == REPLACE_MOD_INDEX;
  int group = -1;
  const QString group_name = ui->group_field->text();
  if(ui->group_check->isChecked() && groups_.contains(group_name))
    group = mod_ids_[groups_.indexOf(group_name)];
  std::vector<int> deployers;
  auto settings = QSettings(QCoreApplication::applicationName());
  settings.beginGroup(QString::number(app_id_));
  settings.beginWriteArray("selected_deployers");
  int settings_index = 0;
  for(int i = 0; i < ui->deployer_list->count(); i++)
  {
    if(ui->deployer_list->item(i)->checkState() == Qt::Checked)
    {
      settings.setArrayIndex(settings_index++);
      settings.setValue("selected", i);
      deployers.push_back(i);
    }
  }
  settings.endArray();
  settings.setValue("fomod_target_deployer", ui->fomod_deployer_box->currentIndex());
  settings.endGroup();
  std::vector<std::pair<sfs::path, sfs::path>> fomod_files{};
  AddModInfo info{ ui->name_text->text().toStdString(),
                   ui->version_text->text().toStdString(),
                   Installer::INSTALLER_TYPES[ui->installer_box->currentIndex()],
                   mod_path_.toStdString(),
                   deployers,
                   group,
                   options,
                   ui->root_level_box->value(),
                   fomod_files,
                   replace_mod,
                   local_source_.toStdString(),
                   remote_source_.toStdString() };
  if(Installer::INSTALLER_TYPES[ui->installer_box->currentIndex()] == Installer::FOMODINSTALLER)
  {
    fomod_dialog_->setupDialog(
      sfs::path(mod_path_.toStdString()) / path_prefix_.toStdString(),
      deployer_paths_[ui->fomod_deployer_box->currentIndex()].toStdString(),
      app_version_,
      info,
      app_id_);
    if(!fomod_dialog_->hasSteps())
    {
      info.files = fomod_dialog_->getResult();
      emit addModAccepted(app_id_, info);
    }
    fomod_dialog_->show();
  }
  else
    emit addModAccepted(app_id_, info);
}

void AddModDialog::on_group_check_stateChanged(int state)
{
  ui->group_field->setEnabled(state == Qt::Checked);
  ui->deployer_list->setEnabled(ui->group_combo_box->currentIndex() == ADD_TO_GROUP_INDEX ||
                                state == Qt::Unchecked);
  ui->group_field->updateValidation();
  updateOkButton();
}

void AddModDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit addModAborted(mod_path_);
}

void AddModDialog::on_name_text_textChanged(const QString& text)
{
  updateOkButton();
}

void AddModDialog::on_version_text_textChanged(const QString& text)
{
  updateOkButton();
}

void AddModDialog::on_root_level_box_valueChanged(int value)
{
  for(int i = 0; i < ui->content_tree->topLevelItemCount(); i++)
    colorTreeNodes(ui->content_tree->topLevelItem(i), 0, value);
}

void AddModDialog::on_installer_box_currentIndexChanged(int index)
{
  if(ui->installer_box->count() > 0 &&
     Installer::INSTALLER_TYPES[ui->installer_box->currentIndex()] == Installer::FOMODINSTALLER)
  {
    ui->fomod_deployer_box->setVisible(true);
    ui->fomod_label->setVisible(true);
    ui->options_container->setEnabled(false);
  }
  else
  {
    ui->fomod_deployer_box->setVisible(false);
    ui->fomod_label->setVisible(false);
    ui->options_container->setEnabled(true);
  }
}

void AddModDialog::on_group_field_textChanged(const QString& arg1)
{
  updateOkButton();
}

void AddModDialog::on_group_combo_box_currentIndexChanged(int index)
{
  ui->deployer_list->setEnabled(index == ADD_TO_GROUP_INDEX ||
                                ui->group_check->checkState() == Qt::Unchecked);
}

void AddModDialog::onFomodDialogComplete(int app_id, AddModInfo info)
{
  emit addModAccepted(app_id, info);
}

void AddModDialog::onFomodDialogAborted()
{
  emit addModAborted(mod_path_);
}
