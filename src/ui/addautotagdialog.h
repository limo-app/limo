/*!
 * \file addautotagdialog.h
 * \brief Header for the AddAutoTagDialog class.
 */

#pragma once

#include <QDialog>


namespace Ui
{
class AddAutoTagDialog;
}

/*!
 * \brief Dialog for adding a new auto tag or renaming an existing one.
 */
class AddAutoTagDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the ui for adding a new auto tag.
   * \param existing_tags Contains names of all currently existing auto tags.
   * \param parent Parent of this widget.
   */
  explicit AddAutoTagDialog(const QStringList& existing_tags, QWidget* parent = nullptr);
  /*!
   * \brief Initializes the ui for renaming an existing auto tag.
   * \param existing_tags Contains names of all currently existing auto tags.
   * \param tag_name Name of the tag to be renamed.
   * \param parent Parent of this widget.
   */
  AddAutoTagDialog(QStringList existing_tags, const QString& tag_name, QWidget* parent = nullptr);
  /*! \brief Deletes the ui. */
  ~AddAutoTagDialog();

  /*!
   * \brief Resturns the name entered in the ui.
   * \return The name.
   */
  QString getName() const;

private:
  /*! \brief Auto generated ui elements. */
  Ui::AddAutoTagDialog* ui;

private slots:
  /*! \brief Updates the OK button to only be enabled when a unique name has been entered. */
  void onTagNameEdited();
};
