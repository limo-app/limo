/*!
 * \file editautotagsdialog.h
 * \brief Header for the EditAutoTagsDialog class.
 */

#pragma once

#include "core/editautotagaction.h"
#include "core/tagcondition.h"
#include <QAction>
#include <QDialog>
#include <map>
#include <set>


namespace Ui
{
class EditAutoTagsDialog;
}

/*!
 * \brief Dialog used to add, remove and edit auto tags.
 */
class EditAutoTagsDialog : public QDialog
{
  Q_OBJECT

public:
  /*!
   * \brief Initializes the ui and generates actions and combo box entries.
   * \param parent Parent of this widget. Passed to the constructor of QDialog.
   */
  explicit EditAutoTagsDialog(QWidget* parent = nullptr);
  /*! \brief Deletes the ui. */
  ~EditAutoTagsDialog();

  /*!
   * \brief Initializes the dialog with the given tags.
   * \param auto_tags Maps tags to a pair of expression and conditions.
   */
  void setupDialog(
    int app_id,
    const std::map<std::string, std::pair<std::string, std::vector<TagCondition>>>& auto_tags);
  /*!
   * \brief Emits dialogClosed.
   * \param event The close event sent upon closing the dialog.
   */
  void closeEvent(QCloseEvent* event) override;

private:
  /*! \brief Contains auto generated ui elements */
  Ui::EditAutoTagsDialog* ui;
  /*! \brief Any index for the connection type check box. */
  static constexpr int connection_cb_any_index = 0;
  /*! \brief All index for the connection type check box. */
  static constexpr int connection_cb_all_index = 1;
  /*! \brief Advanced index for the connection type check box. */
  static constexpr int connection_cb_advanced_index = 2;

  /*! \brief Action column in the condition table. */
  static constexpr int action_col = 0;
  /*! \brief Id column in the condition table. */
  static constexpr int id_col = 1;
  /*! \brief Target column in the condition table. */
  static constexpr int target_col = 2;
  /*! \brief Invert column in the condition table. */
  static constexpr int invert_col = 3;
  /*! \brief Matcher column in the condition table. */
  static constexpr int matcher_col = 4;
  /*! \brief String column in the condition table. */
  static constexpr int string_col = 5;

  /*! \brief File name index of the checkbox for target selection. */
  static constexpr int target_file_index = 0;
  /*! \brief Full path index of the checkbox for target selection. */
  static constexpr int target_path_index = 1;

  /*! \brief False index of the checkbox for inverting a condition. */
  static constexpr int invert_false_index = 0;
  /*! \brief True index of the checkbox for inverting a condition. */
  static constexpr int invert_true_index = 1;

  /*! \brief String index of the checkbox for matcher selection. */
  static constexpr int matcher_string_index = 0;
  /*! \brief Regex index of the checkbox for matcher selection. */
  static constexpr int matcher_regex_index = 1;

  /*! \brief Maps tags to a pair of expression and conditions. */
  std::map<std::string, std::pair<std::string, std::vector<TagCondition>>> auto_tags_;
  /*! \brief Contains all tags for which the conditions have been modified. */
  std::set<std::string> tags_with_updated_conditions_;
  /*! \brief Action for adding a new tag. */
  std::unique_ptr<QAction> add_tag_action_;
  /*! \brief Action for removing a tag. */
  std::unique_ptr<QAction> remove_tag_action_;
  /*! \brief Action for renaming a tag. */
  std::unique_ptr<QAction> rename_tag_action_;
  /*! \brief Maps tags to their selected connection type. */
  std::map<QString, int> connection_index_map_;
  /*! \brief If true: Connection box index changes will not count as condition updates. */
  bool ignore_index_changes_ = false;
  /*! \brief Contains all editing actions performed in this dialog. */
  std::vector<EditAutoTagAction> edit_actions_;
  /*! \brief Target ModdedApplication. */
  int app_id_;
  /*! \brief Indicates whether the dialog has been completed. */
  bool dialog_completed_ = false;

  /*!
   * \brief Enables/ disables interactive elements in the ui.
   * \param enable The new enabled status.
   */
  void enableInteraction(bool enable);
  /*!
   * \brief Catches mouse wheel events if the target is a combo box.
   * \param object Event object.
   * \param event The source event.
   * \return True if event was handled, else false.
   */
  bool eventFilter(QObject* object, QEvent* event) override;
  /*!
   * \brief Creates a boolean expression where variables are every integer in [0, length) and
   * every operator is os.
   * \param op Operator to use.
   * \param length Number of variables.
   * \return The expression.
   */
  QString createExpression(QString op, int length);
  /*! \brief Updates the expresssion line edit and combo box for the currently selected tag. */
  void updateExpressionBox();
  /*! \brief Enables/ disables the Ok button, depending on the current ui state. */
  void updateOkButton();

private slots:
  /*! \brief Updates the currently displayed expression and conditions to match the selected tag. */
  void updateConditionTable();
  /*!
   * \brief Called when a remove button inside the condition table has been pressed.
   * Removes the respective condition.
   * \param row Condition index.
   * \param col Not used.
   */
  void onConditionRemoved(int row, int col);
  /*! \brief Adds a new condition to the condition table. */
  void onConditionAdded();
  /*!
   *  \brief Updates the conditions for the current tag in the auto_tags_ map
   *  according to the settings in the condition table.
   */
  void onConditionEdited();
  /*!
   * \brief Updates the help label and hides/ shows the expression line edit if needed.
   * \param index The new index.
   */
  void on_connection_cb_currentIndexChanged(int index);
  /*! \brief Removes the currently active tag. */
  void onTagRemoved();
  /*! \brief Updates the ui to show data for the new tag. */
  void on_tag_cb_currentIndexChanged(int index);
  /*! \brief Shows a dialog for adding a new tag. Creates the new tag if the dialog was accepted. */
  void onTagAdded();
  /*! \brief Show a dialog for renaming a tag. Renames the tag if the dialog was accepted.*/
  void onTagRenamed();
  /*!
   * \brief Checks if the given expression is valid and disables/ enables the Ok button.
   * \param expression New expression.
   */
  void on_expression_line_edit_textEdited(const QString& expression);
  /*! \brief Constructs all missing editing actions and emits tagsEdited. */
  void on_buttonBox_accepted();
  /*! \brief Emits dialogClosed. */
  void on_buttonBox_rejected();

signals:
  /*!
   * \brief Signals dialog completion.
   * \param actions Editing actions performed.
   */
  void tagsEdited(int app_id, std::vector<EditAutoTagAction> actions);
  /*! \brief Signales dialog has been closed without performing any actions. */
  void dialogClosed();
};
