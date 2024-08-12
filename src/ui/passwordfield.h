/*!
 * \file passwordfield.h
 * \brief Header for the PasswordField class.
 */
#pragma once

#include "validatinglineedit.h"
#include <QPushButton>
#include <QWidget>


/*!
 * \brief Widget used to enter passwords. Contains a line field for input and a button
 * to show/ hide the password. Can be paired with another PasswordField as repetition
 * check.
 */
class PasswordField : public QWidget
{
  Q_OBJECT
public:
  /*! \brief Role of this field. */
  enum Role
  {
    /*! \brief This is the main PasswordField. */
    main,
    /*! \brief This PasswordField is meant for repetition checking. */
    repeat
  };

  /*!
   * \brief Initializes the UI.
   * \param parent Parent for this widget, this is passed to the constructor of QDialog.
   */
  explicit PasswordField(QWidget* parent = nullptr);

  /*!
   * \brief Returns the password currently in the password field..
   * \return The password.
   */
  QString getPassword() const;
  /*!
   * \brief Returns the ValidatingLineEdit used for input.
   * \return The line edit.
   */
  ValidatingLineEdit* getPasswordLineEdit() const;
  /*!
   * \brief Sets a partner PasswordField for repetition checking.
   * \param partner The partner.
   * \param partner_role Role of the partner.
   */
  void setPartnerField(PasswordField* partner, Role partner_role);

private:
  /*! \brief Used for input. Gives visual feedback is this has a repeat role and the passwords dont match. */
  ValidatingLineEdit* password_line_edit_;
  /*! \brief Button used to show/ hide the password. */
  QPushButton* view_button_;
  /*! \brief Icon used to indicate that the password is to be shown. */
  const QIcon show_icon = QIcon::fromTheme("view-visible");
  /*! \brief Icon used to indicate that the password is to be hidden. */
  const QIcon hide_icon = QIcon::fromTheme("view-hidden");
  /*! \brief Partner PasswordField. */
  PasswordField* partner_ = nullptr;
  /*! \brief Role if this field. */
  Role role_ = main;
  /*! \brief Always true if role_ == main. Else only true if both passwords match. */
  bool is_valid_ = true;

  /*! \brief Updates the is_valid_ flag to be false if role_ == repeat and the passwords mismatch. */
  void updateValidationStatus();

public slots:
  /*!
   * \brief Updates the visual feedback for matching passwords.
   * \param new_password The new partner input.
   */
  void onPartnerFieldChanged(QString new_password);

private slots:
  /*!
   * \brief Updates the visual feedback for matching passwords.
   * \param new_password The new partner password.
   */
  void onPasswordEdited(QString new_password);
  /*! \brief Toggles password visibility. */
  void onViewButtonPressed();

signals:
  /*!
   * \brief Signals that the current password has been edited by the user.
   * \param new_password The new password as entered by the user.
   */
  void passwordEdited(QString new_password);
  /*!
   * \brief Sent when the is_valid_ status of this field changes.
   * \param is_valid The new status.
   */
  void passwordValidityChanged(bool is_valid);
};
