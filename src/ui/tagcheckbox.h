/*!
 * \file tagcheckbox.h
 * \brief Header for the TagCheckBox class.
 */

#pragma once

#include <QCheckBox>


/*!
 * \brief When clicked: Emits a signal containing its own text as well as the new check state.
 */
class TagCheckBox : public QCheckBox
{
  Q_OBJECT
public:
  /*!
   * \brief Constructor. Initializes the style sheet and sets this checkbox to a tristate box.
   * \param text Display text for this checkbox.
   */
  TagCheckBox(const QString& tag_name, int num_mods);

  /*! \brief The style sheet used for this checkbox. */
  // clang-format off
  static constexpr char style_sheet[] =
    "QCheckBox::indicator:indeterminate {"
      "image: url(:/filter_accept.svg);"
      "width:16px;"
      "height:16px;"
      "margin-left: 2;"
      "margin-right: 2;"
      "spacing: 5;"
    "}"
    "QCheckBox::indicator:checked {"
      "image: url(:/filter_reject.svg);"
      "width:16px;"
      "height:16px;"
      "margin-left: 2;"
      "margin-right: 2;"
      "spacing: 5;"
    "}";
  // clang-format on

private:
  /*! \brief Name of the displayed tag. */
  QString tag_name_;

private slots:
  /*!
   * \brief Connected to stateChanged.
   * Emits tagBoxChecked with the display text and the check state as arguments.
   * \param state The new check state.
   */
  void onChecked(int state);

signals:
  /*!
   * \brief Signals check state has been chenged.
   * \param tag_name Display text of this check box.
   * \param state New check state.
   */
  void tagBoxChecked(QString tag_name, int state);
};
