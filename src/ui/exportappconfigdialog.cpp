#include "exportappconfigdialog.h"
#include "ui_exportappconfigdialog.h"


ExportAppConfigDialog::ExportAppConfigDialog(QWidget* parent) :
  QDialog(parent), ui(new Ui::ExportAppConfigDialog)
{
  ui->setupUi(this);
}

ExportAppConfigDialog::~ExportAppConfigDialog()
{
  delete ui;
}

void ExportAppConfigDialog::init(int app_id,
                                 const QString& app_name,
                                 const QStringList& deployers,
                                 const QStringList& auto_tags)
{
  dialog_completed_ = false;
  app_id_ = app_id;
  setWindowTitle("Export config for " + app_name);

  ui->deployer_list->clear();
  for(const auto& deployer : deployers)
  {
    auto item = new QListWidgetItem(deployer);
    item->setCheckState(Qt::Checked);
    ui->deployer_list->addItem(item);
  }

  ui->auto_tag_list->clear();
  for(const auto& tag : auto_tags)
  {
    auto item = new QListWidgetItem(tag);
    item->setCheckState(Qt::Checked);
    ui->auto_tag_list->addItem(item);
  }
}

void ExportAppConfigDialog::closeEvent(QCloseEvent* event)
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit dialogClosed();
  QDialog::reject();
}

void ExportAppConfigDialog::reject()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit dialogClosed();
  QDialog::reject();
}

void ExportAppConfigDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  std::vector<int> deployers;
  for(int i = 0; i < ui->deployer_list->count(); i++)
  {
    if(ui->deployer_list->item(i)->checkState() == Qt::Checked)
      deployers.push_back(i);
  }

  QStringList auto_tags;
  for(int i = 0; i < ui->auto_tag_list->count(); i++)
  {
    if(ui->auto_tag_list->item(i)->checkState() == Qt::Checked)
      auto_tags << ui->auto_tag_list->item(i)->text();
  }

  emit appConfigExported(app_id_, deployers, auto_tags);
}

void ExportAppConfigDialog::on_buttonBox_rejected()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;
  emit dialogClosed();
}
