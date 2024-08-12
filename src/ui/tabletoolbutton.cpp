#include "tabletoolbutton.h"

TableToolButton::TableToolButton(int row) : row_(row) {}

void TableToolButton::onRunClicked()
{
  emit clickedRunAt(row_);
}

void TableToolButton::onRemoveClicked()
{
  emit clickedRemoveAt(row_);
}
