/*!
 * \file validatinglineedit.h
 * \brief Header for the ValidatingLineEdit class.
 */

#pragma once

#include <QLineEdit>

/*!
 * \brief A line edit which automatically validates its input and shows a visual
 * indicator for invalid inputs.
 */
class ValidatingLineEdit : public QLineEdit
{
  Q_OBJECT
public:
  /*! \brief Type of validation to be applied. */
  enum ValidationMode
  {
    /*! \brief All text is valid. */
    VALID_NONE = 0,
    /*! \brief Requires text to not be an empty string. */
    VALID_NOT_EMPTY = 1,
    /*! \brief Requires text to be a path to an existing file or directory. */
    VALID_PATH_EXISTS = 2,
    /*! \brief Uses a custom validation function. */
    VALID_CUSTOM = 3
  };

  /*!
   * \brief Calls QLineEdit constructor and sets the validation mode.
   * \param parent Parent widget.
   * \param mode Validation mode to use.
   */
  ValidatingLineEdit(QWidget* parent = nullptr, ValidationMode mode = VALID_NOT_EMPTY);
  /*!
   * \brief Calls QLineEdit constructor and sets the validation mode.
   * \param contents Initial text for this line edit.
   * \param parent Parent widget.
   * \param mode Validation mode to use.
   */
  ValidatingLineEdit(const QString& contents,
                     QWidget* parent = nullptr,
                     ValidationMode mode = VALID_NOT_EMPTY);

  /*!
   * \brief Checks if the current text is valid.
   * \return True if the text is valid.
   */
  bool hasValidText();
  /*!
   * \brief Changes the validation mode to the new mode.
   * \param mode New validation mode.
   */
  void setValidationMode(ValidationMode mode);
  /*!
   * \brief Sets a new custom validator function. Does not affect validation mode.
   * \param validator If the validation mode is set to VALID_CUSTOM, this function will be called
   * and be passed the current input text as argument, whenever the text
   * is changed. The new text is valid, if the function returns true.
   */
  void setCustomValidator(std::function<bool(QString)> validator);
  /*! \brief Updates the visual indicator using the current text. */
  void updateValidation();

private:
  /*! \brief Determines how the input text is validated, see \ref ValidationMode modes. */
  ValidationMode validation_mode_;
  /*! \brief If the validation mode is set to VALID_CUSTOM, this function will be called
   * and be passed the current input text as argument, whenever the text
   * is changed. The new text is valid, if the function returns true. */
  std::function<bool(QString)> validator_ = [](QString s) { return true; };

private slots:
  /*!
   * \brief Connected to this line edits textChanged signal. Updates the visual indicator.
   * \param new_text The new display text
   */
  void onTextChanged(const QString& new_text);
};
