/*!
 * \file fomodcheckbox.h
 * \brief Header for the FomodCheckBox class.
 */

#pragma once

#include <QCheckBox>
#include <QLabel>


/*!
 * \brief Used by FomodDialog. Updates the containing dialogs info panel with
 * a description and image representative of the plugin represented by this button.
 */
class FomodCheckBox : public QCheckBox
{
  Q_OBJECT
public:
  /*!
   * \brief Constructor.
   * \param text Text displayed on this button.
   * \param description Description of the plugin represented by this button.
   * \param image_path Image for the buttons plugin.
   * \param description_label Pointer to the QLabel which will be updated with
   * the plugin description.
   * \param image_label Pointer to the QLabel which will be updated with
   * the plugin image.
   */
  FomodCheckBox(const QString& text,
                const QString& description,
                const QString& image_path,
                QLabel* description_label,
                QLabel* image_label);

protected:
  /*!
   * \brief Gets called when the mouse enters this widget. Sets description and image of the
   * info panel.
   * \param event The source event.
   */
  void enterEvent(QEvent* event) override;

private:
  /*! \brief Description of the plugin represented by this button. */
  QString description_;
  /*! \brief Path to the image for the buttons plugin. */
  QString image_path_;
  /*! \brief Pointer to the QLabel which will be updated with the plugin description. */
  QLabel* description_label_;
  /*! \brief Pointer to the QLabel which will be updated with the plugin image. */
  QLabel* image_label_;
};
