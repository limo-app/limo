#include "listaction.h"


ListAction::ListAction(int index, QObject* parent) : QAction{ parent }, index_(index)
{
  connect(this, &QAction::triggered, this, &ListAction::onTriggeredAt);
}

void ListAction::onTriggeredAt()
{
  emit triggeredAt(index_);
}
