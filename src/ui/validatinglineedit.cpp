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
    return accept_empty_paths_;
  if(validation_mode_ == VALID_PATH_EXISTS)
    return std::filesystem::exists(path.toStdString());
  if(validation_mode_ == VALID_IS_EXISTING_FILE)
    return std::filesystem::is_regular_file(path.toStdString());
  // validation_mode_ == VALID_IS_EXISTING_DIRECTORY
  return std::filesystem::is_directory(path.toStdString());
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

bool ValidatingLineEdit::acceptsEmptyPaths() const
{
  return accept_empty_paths_;
}

void ValidatingLineEdit::setAcceptsEmptyPaths(bool accept)
{
  accept_empty_paths_ = accept;
  updateValidation();
}

bool ValidatingLineEdit::showsStatusTooltip() const
{
  return show_status_tooltip_;
}

void ValidatingLineEdit::setShowStatusTooltip(bool show)
{
  show_status_tooltip_ = show;
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
    if(show_status_tooltip_)
    {
      const QString empty_info = accept_empty_paths_ ? " or field must be empty" : "";
      if(validation_mode_ == VALID_NOT_EMPTY)
        setToolTip("Field must not be empty");
      else if(validation_mode_ == VALID_CUSTOM)
        setToolTip("Field contains invalid input");
      else if(validation_mode_ == VALID_PATH_EXISTS)
        setToolTip("Path must exist" + empty_info);
      else if(validation_mode_ == VALID_IS_EXISTING_FILE)
        setToolTip("Path must be existing file" + empty_info);
      else if(validation_mode_ == VALID_IS_EXISTING_DIRECTORY)
        setToolTip("Path must be existing directory" + empty_info);
    }
  }
  else if(show_status_tooltip_)
    setToolTip("");
  setPalette(palette);
}
