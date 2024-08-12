#include "addprofiledialog.h"
#include "ui_addprofiledialog.h"
#include <QPushButton>


AddProfileDialog::AddProfileDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AddProfileDialog)
{
  ui->setupUi(this);
}

AddProfileDialog::~AddProfileDialog()
{
  delete ui;
}

void AddProfileDialog::setAddMode(int app_id, const QStringList& profiles)
{
  app_id_ = app_id;
  ui->name_field->setText("");
  ui->clone_check_box->setCheckState(Qt::Unchecked);
  setWindowTitle("New Profile");
  ui->clone_check_box->setHidden(false);
  ui->clone_selection_box->setHidden(false);
  edit_mode_ = false;
  ui->clone_selection_box->clear();
  ui->clone_selection_box->addItems(profiles);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  dialog_completed_ = false;
}

void AddProfileDialog::setEditMode(int app_id,
                                   int profile,
                                   const QString& name,
                                   const QString& app_version)
{
  app_id_ = app_id;
  profile_ = profile;
  ui->name_field->setText(name);
  ui->app_version_field->setText(app_version);
  edit_mode_ = true;
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  setWindowTitle("Edit " + name);
  ui->clone_check_box->setHidden(true);
  ui->clone_selection_box->setHidden(true);
  dialog_completed_ = false;
}

void AddProfileDialog::on_clone_check_box_stateChanged(int state)
{
  if(state == Qt::Checked)
    ui->clone_selection_box->setEnabled(true);
  else
    ui->clone_selection_box->setEnabled(false);
}


void AddProfileDialog::on_name_field_textChanged(const QString& text)
{
  if(text.isEmpty())
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  else
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}


void AddProfileDialog::on_buttonBox_accepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  if(!edit_mode_)
  {
    int source = -1;
    if(ui->clone_check_box->checkState() == Qt::Checked)
      source = ui->clone_selection_box->currentIndex();
    emit profileAdded(app_id_,
                      { ui->name_field->text().toStdString(),
                        ui->app_version_field->text().toStdString(),
                        source });
  }
  else
    emit profileEdited(
      app_id_,
      profile_,
      { ui->name_field->text().toStdString(), ui->app_version_field->text().toStdString(), -1 });
}
