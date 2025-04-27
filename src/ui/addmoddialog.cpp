#include "addmoddialog.h"
#include "../core/installer.h"
#include "../core/log.h"
#include "../core/pathutils.h"
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

namespace pu = path_utils;
namespace sfs = std::filesystem;
namespace str = std::ranges;
namespace stv = std::views;


AddModDialog::AddModDialog(ModListModel* mod_list_model,
                           DeployerListModel* deployer_list_model,
                           QWidget* parent) :
  mod_list_model_(mod_list_model), deployer_list_model_(deployer_list_model), QDialog(parent),
  ui(new Ui::AddModDialog)
{
  ui->setupUi(this);
  fomod_dialog_ = std::make_unique<FomodDialog>();
  connect(
    fomod_dialog_.get(), &FomodDialog::addModAccepted, this, &AddModDialog::onFomodDialogComplete);
  connect(
    fomod_dialog_.get(), &FomodDialog::addModAborted, this, &AddModDialog::onFomodDialogAborted);
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
  qDebug() << node->text(0);
  if(cur_depth < root_level)
  {
    node->setForeground(0, COLOR_REMOVE_);
    node->setExpanded(true);
  }
  else
    node->setForeground(0, COLOR_KEEP_);
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

int AddModDialog::detectRootLevel(int deployer) const
{
  bool is_case_invariant = case_invariant_deployers_[deployer];
  sfs::path deployer_path = deployer_paths_[deployer].toStdString();

  auto cur_item = ui->content_tree->invisibleRootItem();
  if(cur_item->childCount() != 1)
    return 0;

  std::string path_string = deployer_path.string();
  if(path_string.ends_with("/"))
    path_string = path_string.substr(0, path_string.size() - 1);
  if(path_string.empty())
    return 0;
  std::string last_path_component = *std::prev(std::filesystem::path(path_string).end());
  if(is_case_invariant)
    last_path_component = pu::toLowerCase(last_path_component);

  int cur_level = 0;
  int deployer_target_level = -1;
  while(true)
  {
    const auto cur_text = is_case_invariant ? cur_item->text(0).toLower().toStdString()
                                            : cur_item->text(0).toStdString();
    if(deployer_target_level == -1 && cur_text == last_path_component)
      deployer_target_level = cur_level;
    for(int i = 0; i < cur_item->childCount(); i++)
    {
      if(pu::pathExists(
           cur_item->child(i)->text(0).toStdString(), deployer_path, is_case_invariant))
        return cur_level;
    }

    if(cur_item->childCount() != 1)
      break;

    cur_item = cur_item->child(0);
    cur_level++;
  }

  if(cur_item->childCount() == 0)
    return deployer_target_level == -1 ? 0 : deployer_target_level;

  return 0;
}

bool AddModDialog::setupDialog(const QStringList& deployers,
                               int cur_deployer,
                               const QStringList& deployer_paths,
                               const std::vector<bool>& autonomous_deployers,
                               const std::vector<bool>& case_invariant_deployers,
                               const QString& app_version,
                               const ImportModInfo& info)
{
  groups_.clear();
  const auto& mod_infos = mod_list_model_->getModInfo();
  for(const auto& mod_info : mod_infos)
  {
    std::string prefix = "";
    if(mod_info.group != -1 && !mod_info.is_active_group_member)
      prefix = "[INACTIVE] ";
    groups_ << (prefix + mod_info.mod.name + " [" + std::to_string(mod_info.mod.id) + "]").c_str();
  }

  import_mod_info_ = info;
  ui->content_tree->clear();
  ui->root_level_box->setValue(0);
  ui->name_text->setFocus();
  deployer_paths_ = deployer_paths;
  case_invariant_deployers_ = case_invariant_deployers;
  app_version_ = app_version;
  ui->group_combo_box->setCurrentIndex(ADD_TO_GROUP_INDEX);
  ui->deployer_list->setEnabled(true);
  ui->group_field->clear();
  ui->group_field->setEnabled(false);
  ui->group_field->updateValidation();
  completer_ = std::make_unique<QCompleter>(groups_);
  completer_->setCaseSensitivity(Qt::CaseInsensitive);
  completer_->setFilterMode(Qt::MatchContains);
  ui->group_field->setCompleter(completer_.get());
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  ui->group_check->setCheckState(Qt::Unchecked);
  int mod_index = -1;
  if(info.target_group_id != -1)
  {
    auto iter = str::find_if(mod_infos,
                             [mod_id = info.target_group_id](const ModInfo& info)
                             { return mod_id == info.mod.id; });
    if(iter != mod_infos.end())
    {
      mod_index = iter - mod_infos.begin();
      ui->group_field->setText(groups_[mod_index]);
      ui->group_check->setCheckState(Qt::Checked);
      ui->group_combo_box->setCurrentIndex(REPLACE_MOD_INDEX);
    }
  }

  std::regex name_regex(R"(-\d+((?:-[\dA-Za-z]+)+)-\d+(?:\(\d+\))?\.(?:zip|7z|rar)$)");
  std::smatch match;
  std::string name_str = info.local_source.filename().string();
  if(mod_index >= 0 && mod_index < mod_infos.size())
  {
    ui->name_text->setText(mod_infos[mod_index].mod.name.c_str());
    ui->version_text->setText(mod_infos[mod_index].mod.version.c_str());
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
    ui->name_text->setText(name_str.c_str());
  }

  if(!info.remote_file_name.empty())
    ui->name_text->setText(info.remote_file_name.c_str());
  if(!info.remote_file_version.empty())
    ui->version_text->setText(info.remote_file_version.c_str());

  if(!info.name_overwrite.empty())
    ui->name_text->setText(info.name_overwrite.c_str());
  if(!info.version_overwrite.empty())
    ui->version_text->setText(info.version_overwrite.c_str());

  ui->installer_box->clear();
  int root_level = 0;
  std::string prefix;
  std::string detected_type;
  try
  {
    auto signature = Installer::detectInstallerSignature(info.current_path);
    std::tie(root_level, prefix, detected_type) = signature;
  }
  catch(std::runtime_error& error)
  {
    showError(error);
    emit addModAborted(info.current_path.c_str());
    return false;
  }
  if(detected_type == Installer::FOMODINSTALLER)
  {
    auto [name, version] =
      fomod::FomodInstaller::getMetaData(info.current_path / prefix);
    if(!name.empty() && info.name_overwrite.empty())
      ui->name_text->setText(name.c_str());
    if(!version.empty() && info.version_overwrite.empty())
      ui->version_text->setText(version.c_str());
  }

  const std::string mod_name = ui->name_text->text().toStdString();
  auto remote_source_or_name_matches =
    [&mod_info = std::as_const(info), &mod_name](const auto& pair)
  {
    const Mod mod = std::get<1>(pair).mod;
    return !mod_info.remote_source.empty() && mod_info.remote_source == mod.remote_source ||
           mod.name == mod_name ||
           mod.remote_mod_id != -1 && mod_info.remote_mod_id == mod.remote_mod_id;
  };

  if(info.target_group_id == -1)
  {
    int group_index = -1;
    int max_match_quality = -1;
    for(const auto& [i, mod_info] :
        mod_infos | stv::enumerate | stv::filter(remote_source_or_name_matches))
    {
      int match_quality = 0;
      if(info.remote_type == mod_info.mod.remote_type)
      {
        if(mod_info.mod.remote_mod_id != -1 && mod_info.mod.remote_mod_id == info.remote_mod_id)
          match_quality += 16;
        if(mod_info.mod.remote_file_id != -1 && mod_info.mod.remote_file_id == info.remote_file_id)
          match_quality += 8;
      }
      if(info.remote_source == mod_info.mod.remote_source)
        match_quality += 4;
      if(mod_name == mod_info.mod.name)
        match_quality += 2;
      if(mod_info.is_active_group_member)
        match_quality += 1;
      if(match_quality > max_match_quality)
      {
        max_match_quality = match_quality;
        group_index = i;
      }
    }
    if(group_index != -1)
      ui->group_field->setText(groups_[group_index]);
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
  settings.beginGroup(QString::number(info.app_id));
  int size = settings.beginReadArray("selected_deployers");
  for(int i = 0; i < size; i++)
  {
    settings.setArrayIndex(i);
    selected_deployers.insert(settings.value("selected").toInt());
  }
  settings.endArray();

  try
  {
    auto mod_file_paths = Installer::getArchiveFileNames(info.current_path);
    int max_depth = 0;
    for(const auto& path : mod_file_paths)
      max_depth = std::max(addTreeNode(ui->content_tree, path), max_depth);
    ui->root_level_box->setMaximum(std::max(max_depth - 1, 0));
    ui->root_level_box->setValue(root_level);
  }
  catch(std::runtime_error& error)
  {
    showError(error);
    emit addModAborted(info.current_path.c_str());
    return false;
  }

  ui->fomod_deployer_box->clear();
  bool root_level_checked = false;
  for(int i = 0; i < deployers.size(); i++)
  {
    const bool is_target = selected_deployers.contains(i) | (i == cur_deployer);
    if(!autonomous_deployers[i])
    {
      ui->fomod_deployer_box->addItem(deployers[i]);
      if(!root_level_checked && is_target && detected_type != Installer::FOMODINSTALLER)
      {
        root_level_checked = true;
        root_level = detectRootLevel(i);
        ui->root_level_box->setValue(root_level);
      }
    }
    auto item = new QListWidgetItem(deployers[i], ui->deployer_list);
    item->setCheckState(is_target ? Qt::Checked : Qt::Unchecked);
    item->setHidden(autonomous_deployers[i]);
  }

  int fomod_target_deployer = settings.value("fomod_target_deployer", -1).toInt();
  if(fomod_target_deployer >= 0 && fomod_target_deployer < ui->fomod_deployer_box->count())
    ui->fomod_deployer_box->setCurrentIndex(fomod_target_deployer);
  else if(cur_deployer >= 0 && cur_deployer < ui->fomod_deployer_box->count())
    ui->fomod_deployer_box->setCurrentIndex(cur_deployer);
  settings.endGroup();

  dialog_completed_ = false;
  return true;
}

void AddModDialog::closeEvent(QCloseEvent* event)
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit addModAborted(import_mod_info_.current_path.c_str());
  QDialog::reject();
}

void AddModDialog::reject()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit addModAborted(import_mod_info_.current_path.c_str());
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
    group = mod_list_model_->getModInfo()[groups_.indexOf(group_name)].mod.id;
  std::vector<int> deployers;
  auto settings = QSettings(QCoreApplication::applicationName());
  settings.beginGroup(QString::number(import_mod_info_.app_id));
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
  import_mod_info_.action_type = ImportModInfo::ActionType::install;
  import_mod_info_.name = ui->name_text->text().toStdString();
  import_mod_info_.version = ui->version_text->text().toStdString();
  import_mod_info_.installer = Installer::INSTALLER_TYPES[ui->installer_box->currentIndex()];
  import_mod_info_.deployers = deployers;
  import_mod_info_.target_group_id = group;
  import_mod_info_.installer_flags = options;
  import_mod_info_.root_level = ui->root_level_box->value();
  import_mod_info_.files = {};
  import_mod_info_.replace_mod = replace_mod;
  if(Installer::INSTALLER_TYPES[ui->installer_box->currentIndex()] == Installer::FOMODINSTALLER)
  {
    bool case_invariant = true;
    for(int i = 0; i < ui->deployer_list->count(); i++)
    {
      if(ui->deployer_list->item(i)->checkState() == Qt::Checked && !case_invariant_deployers_[i])
      {
        case_invariant = false;
        break;
      }
    }
    fomod_dialog_->setupDialog(
      import_mod_info_.current_path / path_prefix_.toStdString(),
      deployer_paths_[ui->fomod_deployer_box->currentIndex()].toStdString(),
      app_version_,
      import_mod_info_,
      import_mod_info_.app_id,
      case_invariant);
    if(!fomod_dialog_->hasSteps())
    {
      import_mod_info_.files = fomod_dialog_->getResult();
      emit addModAccepted(import_mod_info_.app_id, import_mod_info_);
    }
    else
      fomod_dialog_->show();
  }
  else
    emit addModAccepted(import_mod_info_.app_id, import_mod_info_);
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
  emit addModAborted(import_mod_info_.current_path.c_str());
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

void AddModDialog::onFomodDialogComplete(int app_id, ImportModInfo info)
{
  emit addModAccepted(app_id, info);
}

void AddModDialog::onFomodDialogAborted()
{
  emit addModAborted(import_mod_info_.current_path.c_str());
}
