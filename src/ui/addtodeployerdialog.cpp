#include "addtodeployerdialog.h"
#include "ui_addtodeployerdialog.h"


AddToDeployerDialog::AddToDeployerDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::AddToDeployerDialog)
{
  ui->setupUi(this);
}

AddToDeployerDialog::~AddToDeployerDialog()
{
  delete ui;
}

void AddToDeployerDialog::setupDialog(const QStringList& deployer_names,
                                      const QString& mod_name,
                                      std::vector<int>& mod_ids,
                                      const std::vector<int>& mod_deployers,
                                      const std::vector<bool>& auto_deployers)
{
  mod_ids_ = mod_ids;
  if(mod_ids.size() == 1)
    ui->text_label->setText("Add \"" + mod_name + "\" to:");
  else
    ui->text_label->setText("Add " + QString::number(mod_ids.size()) + " mods to:");
  ui->deployer_list->clear();
  for(int i = 0; i < deployer_names.size(); i++)
  {
    auto item = new QListWidgetItem(deployer_names[i], ui->deployer_list);
    if(auto_deployers[i])
      item->setHidden(true);
    else
      item->setCheckState(std::find(mod_deployers.begin(), mod_deployers.end(), i) !=
                              mod_deployers.end()
                            ? Qt::Checked
                            : Qt::Unchecked);
  }
  dialog_completed_ = false;
}

void AddToDeployerDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  std::vector<bool> deployers;
  for(int i = 0; i < ui->deployer_list->count(); i++)
    deployers.push_back(ui->deployer_list->item(i)->checkState() == Qt::Checked);
  emit modDeployersUpdated(mod_ids_, deployers);
}
