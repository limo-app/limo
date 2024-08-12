#include "validatinglineedit.h"
#include "colors.h"
#include <QApplication>
#include <filesystem>


ValidatingLineEdit::ValidatingLineEdit(QWidget* parent, ValidationMode mode) :
  QLineEdit(parent), validation_mode_(mode)
{
  updateValidation();
  connect(this, &ValidatingLineEdit::textChanged, this, &ValidatingLineEdit::onTextChanged);
}

ValidatingLineEdit::ValidatingLineEdit(const QString& contents,
                                       QWidget* parent,
                                       ValidationMode mode) :
  QLineEdit(contents, parent), validation_mode_(mode)
{
  onTextChanged(contents);
  connect(this, &ValidatingLineEdit::textChanged, this, &ValidatingLineEdit::onTextChanged);
}

bool ValidatingLineEdit::hasValidText()
{
  if(!isEnabled() || isHidden())
    return true;
  if(validation_mode_ == VALID_NONE)
    return true;
  if(validation_mode_ == VALID_NOT_EMPTY)
    return !text().isEmpty();
  if(validation_mode_ == VALID_CUSTOM)
    return validator_(text());
  QString path = text();
  if(path.isEmpty())
    return false;
  return std::filesystem::exists(path.toStdString());
}

void ValidatingLineEdit::setValidationMode(ValidationMode mode)
{
  validation_mode_ = mode;
  updateValidation();
}

void ValidatingLineEdit::setCustomValidator(std::function<bool(QString)> validator)
{
  validator_ = validator;
}

void ValidatingLineEdit::updateValidation()
{
  onTextChanged(text());
}

void ValidatingLineEdit::onTextChanged(const QString& new_text)
{
  auto palette = QApplication::palette();
  if(!hasValidText())
  {
    QColor base = palette.color(QPalette::Base);
    QColor invalid = colors::LIGHT_RED;
    const float ratio = 0.5;
    QColor mix_color(ratio * base.red() + (1 - ratio) * invalid.red(),
                     ratio * base.green() + (1 - ratio) * invalid.green(),
                     ratio * base.blue() + (1 - ratio) * invalid.blue());
    palette.setColor(QPalette::Base, mix_color);
  }
  setPalette(palette);
}
