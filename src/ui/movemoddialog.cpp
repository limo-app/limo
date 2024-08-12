#include "movemoddialog.h"
#include "ui_movemoddialog.h"
#include <QDebug>
#include <QPushButton>
#include <QValidator>


MoveModDialog::MoveModDialog(QString name, int source, int num_mods, QWidget* parent) :
  QDialog(parent), ui(new Ui::MoveModDialog), source_(source)
{
  ui->setupUi(this);
  setWindowTitle("Move " + name);
  ui->target_field->setValidator(new QIntValidator(1, num_mods, this));
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

MoveModDialog::~MoveModDialog()
{
  delete ui;
}

void MoveModDialog::on_buttonBox_accepted()
{
  emit modMovedTo(source_, ui->target_field->text().toInt() - 1);
}

void MoveModDialog::on_target_field_textEdited(const QString& new_text)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->target_field->hasAcceptableInput());
}
