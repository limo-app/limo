#include "addtooldialog.h"
#include "qdebug.h"
#include "ui_addtooldialog.h"
#include <QPushButton>

AddToolDialog::AddToolDialog(QWidget* parent) : QDialog(parent), ui(new Ui::AddToolDialog)
{
  ui->setupUi(this);
  connect(ui->tool_widget,
          &EditToolWidget::inputValidityChanged,
          this,
          &AddToolDialog::toolWidgetInputValidityChanged);
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddToolDialog::onButtonBoxAccepted);
}

AddToolDialog::~AddToolDialog()
{
  delete ui;
}

void AddToolDialog::toolWidgetInputValidityChanged(bool is_valid)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(is_valid);
}

void AddToolDialog::setAddMode(int app_id)
{
  setWindowTitle("New Tool");
  is_edit_mode_ = false;
  app_id_ = app_id;
  ui->tool_widget->init();
  dialog_completed_ = false;
}

void AddToolDialog::setEditMode(int app_id, int tool_id, Tool tool)
{
  setWindowTitle(("Edit " + tool.getName()).c_str());
  is_edit_mode_ = true;
  app_id_ = app_id;
  tool_id_ = tool_id;
  ui->tool_widget->init(tool);
  dialog_completed_ = false;
}

void AddToolDialog::onButtonBoxAccepted()
{
  if(dialog_completed_)
    return;
  dialog_completed_ = true;

  if(is_edit_mode_)
    emit toolEdited(app_id_, tool_id_, ui->tool_widget->getTool());
  else
    emit toolAdded(app_id_, ui->tool_widget->getTool());
}
