#include "tablepushbutton.h"

TablePushButton::TablePushButton(int row, int col) : row_(row), col_(col)
{
  connect(this, &QPushButton::clicked, this, &TablePushButton::onClickedAt);
}

void TablePushButton::onClickedAt()
{
  emit clickedAt(row_, col_);
}
