/*!
 * \file listaction.h
 * \brief Header for the ListAction class.
 */

#pragma once

#include <QAction>
#include <QObject>


/*!
 * \brief QAction derivate that emits a signal containing an index when triggered.
 */
class ListAction : public QAction
{
  Q_OBJECT
public:
  /*!
   * \brief Constructs a new action with the given index.
   * \param index Target index.
   * \param parent Parent of this object.
   */
  explicit ListAction(int index, QObject* parent = nullptr);

private:
  /*! \brief Index belonging to this action. */
  int index_;

private slots:
  /*! \brief Emits \ref triggeredAt. */
  void onTriggeredAt();

signals:
  /*!
   * \brief Emits this action's index.
   * \param index The index.
   */
  void triggeredAt(int index);
};
