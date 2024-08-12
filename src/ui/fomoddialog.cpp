#include "fomoddialog.h"
#include "colors.h"
#include "fomodcheckbox.h"
#include "fomodradiobutton.h"
#include "ui_fomoddialog.h"
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>

namespace sfs = std::filesystem;


FomodDialog::FomodDialog(QWidget* parent) : QDialog(parent), ui(new Ui::FomodDialog)
{
  ui->setupUi(this);
  installer_ = std::make_unique<fomod::FomodInstaller>();
  next_button_ = new QPushButton(this);
  back_button_ = new QPushButton(this);
  back_button_->setText("Back");
  ui->buttonBox->addButton(back_button_, QDialogButtonBox::ApplyRole);
  ui->buttonBox->addButton(next_button_, QDialogButtonBox::ApplyRole);
  connect(next_button_, &QPushButton::pressed, this, &FomodDialog::onNextButtonPressed);
  connect(back_button_, &QPushButton::pressed, this, &FomodDialog::onBackButtonPressed);
}

FomodDialog::~FomodDialog()
{
  delete ui;
}

void FomodDialog::setupDialog(const sfs::path& config_file,
                              const sfs::path& target_path,
                              const QString& app_version,
                              const AddModInfo& info,
                              int app_id)
{
  dialog_completed_ = false;
  add_mod_info_ = info;
  app_id_ = app_id;
  installer_->init(config_file, target_path, app_version.toStdString());
  back_button_->setVisible(false);
  has_no_steps_ = installer_->hasNoSteps();
  updateInstallStep();
  updateNextButton();
}

std::vector<std::pair<sfs::path, sfs::path>> FomodDialog::getResult() const
{
  return result_;
}

bool FomodDialog::hasSteps() const
{
  return !has_no_steps_;
}

QAbstractButton* FomodDialog::makeButton(fomod::PluginGroup::Type type,
                                         const QString& text,
                                         const QString& description,
                                         const QString& image_path) const
{
  if(type == fomod::PluginGroup::exactly_one || type == fomod::PluginGroup::at_most_one)
    return new FomodRadioButton(
      text, description, image_path, ui->description_label, ui->image_label);
  return new FomodCheckBox(text, description, image_path, ui->description_label, ui->image_label);
}

void FomodDialog::updateInstallStep(
  std::optional<std::pair<std::vector<std::vector<bool>>, fomod::InstallStep>> prev_step)
{
  if(has_no_steps_)
  {
    result_ = installer_->getInstallationFiles({});
    return;
  }
  std::optional<fomod::InstallStep> step;
  if(!prev_step)
  {
    step = installer_->step(getSelection());
    if(!step)
      return;
  }
  delete ui->group_area->takeWidget();
  qDeleteAll(button_groups_.begin(), button_groups_.end());
  button_groups_.clear();
  group_types_.clear();
  none_groups_.clear();
  if(!prev_step)
    cur_step_ = *step;
  else
    cur_step_ = prev_step->second;
  auto frame = new QFrame();
  ui->group_area->setWidget(frame);
  auto group_layout = new QVBoxLayout();
  frame->setLayout(group_layout);
  auto name_label = new QLabel();
  name_label->setText(QString("## ") + cur_step_.name.c_str());
  name_label->setTextFormat(Qt::MarkdownText);
  group_layout->addWidget(name_label);
  int group_idx = 1;
  for(const auto& group : cur_step_.groups)
  {
    auto button_group = new QButtonGroup;
    auto box = new QGroupBox();
    QString group_title;
    switch(group.type)
    {
      case fomod::PluginGroup::all:
        group_title = " [Select All]";
        break;
      case fomod::PluginGroup::any:
        group_title = " [Select Any]";
        break;
      case fomod::PluginGroup::at_least_one:
        group_title = " [Select At Least One]";
        break;
      default:
        group_title = " [Select One]";
    }
    box->setTitle(group.name.c_str() + group_title);
    auto box_layout = new QVBoxLayout();
    box->setLayout(box_layout);
    if(group.type == group.at_most_one)
    {
      auto button = new FomodRadioButton("None", "", "", ui->description_label, ui->image_label);
      box_layout->addWidget(button);
      button_group->addButton(button, -1);
      none_groups_.insert(group_idx - 1);
    }
    auto unavailable_palette = QApplication::palette();
    auto required_palette = QApplication::palette();
    unavailable_palette.setColor(QPalette::WindowText, colors::RED);
    required_palette.setColor(QPalette::WindowText, colors::ORANGE);
    int plugin_idx = 0;
    for(const auto& plugin : group.plugins)
    {
      auto button = makeButton(
        group.type, plugin.name.c_str(), plugin.description.c_str(), plugin.image_path.c_str());
      connect(button, &QPushButton::clicked, this, &FomodDialog::onPluginSelected);
      QString plugin_type;
      if(plugin.type == fomod::PluginType::recommended)
      {
        plugin_type = "[Recommended] ";
        button->setChecked(true);
      }
      else if(plugin.type == fomod::PluginType::required)
      {
        plugin_type = "[Required] ";
        button->setChecked(true);
        button->setPalette(required_palette);
      }
      else if(plugin.type == fomod::PluginType::not_usable)
      {
        plugin_type = "[Not Available] ";
        button->setPalette(unavailable_palette);
      }
      else
        plugin_type = "";
      if(!plugin.potential_types.empty())
      {
        QString tooltip_text = "Requirements:";
        for(const auto& [type, dependency] : plugin.potential_types)
        {
          tooltip_text +=
            ("\n" + fomod::PLUGIN_TYPE_NAMES[type] + ": " + dependency.toString()).c_str();
        }
        button->setToolTip(tooltip_text);
      }
      button->setText(plugin_type + button->text());
      if(prev_step && !(prev_step->first.empty()))
        button->setChecked(prev_step->first[group_idx - 1][plugin_idx]);
      box_layout->addWidget(button);
      button_group->addButton(button, plugin_idx);
      if(!(group.type == fomod::PluginGroup::at_most_one ||
           group.type == fomod::PluginGroup::exactly_one))
        button_group->setExclusive(false);
      plugin_idx++;
    }
    button_groups_.append(button_group);
    group_types_.append(group.type);
    group_layout->addWidget(box);
    group_idx++;
  }
  group_layout->addStretch();
  next_button_->setEnabled(selectionIsValid());
  updateNextButton();
}

bool FomodDialog::selectionIsValid()
{
  for(int i = 0; i < button_groups_.size(); i++)
  {
    auto type = group_types_[i];
    if(type == fomod::PluginGroup::any)
      continue;
    int num_selected = 0;
    for(auto button : static_cast<const QList<QAbstractButton*>>(button_groups_[i]->buttons()))
    {
      if(button->isChecked())
        num_selected++;
    }
    if(type == fomod::PluginGroup::at_least_one && num_selected == 0 ||
       type == fomod::PluginGroup::at_most_one && num_selected > 1 ||
       type == fomod::PluginGroup::all && num_selected != button_groups_[i]->buttons().size() ||
       type == fomod::PluginGroup::exactly_one && num_selected != 1)
      return false;
  }
  return true;
}

std::vector<std::vector<bool>> FomodDialog::getSelection()
{
  std::vector<std::vector<bool>> selection;
  for(int group_idx = 0; auto group : static_cast<const QList<QButtonGroup*>>(button_groups_))
  {
    std::vector<bool> group_vec;
    for(int button_idx = 0;
        auto button : static_cast<const QList<QAbstractButton*>>(group->buttons()))
    {
      if(!(button_idx == 0 && none_groups_.contains(group_idx)))
        group_vec.push_back(button->isChecked());
      button_idx++;
    }
    selection.push_back(group_vec);
    group_idx++;
  }
  return selection;
}

void FomodDialog::updateNextButton()
{
  next_button_->setEnabled(selectionIsValid());
  auto s = getSelection();
  if(installer_->hasNextStep(getSelection()))
    next_button_->setText("Next");
  else
    next_button_->setText("Finish");
}

void FomodDialog::onNextButtonPressed()
{
  /*
   * For an unknown reason this dialog can be accepted multiple times when
   * clicking fast enough. This guards against that.
   */
  if(dialog_completed_)
    return;

  ui->description_label->setText("");
  ui->image_label->setPixmap({});
  if(next_button_->text() == "Next")
    updateInstallStep();
  else
  {
    dialog_completed_ = true;
    result_ = installer_->getInstallationFiles(getSelection());
    if(result_.empty())
      reject();
    else
    {
      add_mod_info_.files = result_;
      emit addModAccepted(app_id_, add_mod_info_);
    }
    accept();
  }
  if(installer_->hasPreviousStep())
    back_button_->setVisible(true);
}

void FomodDialog::onPluginSelected(bool checked)
{
  updateNextButton();
}

void FomodDialog::onBackButtonPressed()
{
  updateInstallStep(installer_->stepBack());
  if(!installer_->hasPreviousStep())
    back_button_->setVisible(false);
}

void FomodDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit addModAborted();
}

