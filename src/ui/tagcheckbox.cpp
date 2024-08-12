#include "tagcheckbox.h"


TagCheckBox::TagCheckBox(const QString& tag_name, int num_mods)
{
  tag_name_ = tag_name;
  setText(tag_name + " [" + QString::number(num_mods) + " mod" + (num_mods != 1 ? "s]" : "]"));
  setTristate(true);
  connect(this, &QCheckBox::stateChanged, this, &TagCheckBox::onChecked);
  setStyleSheet(style_sheet);
}

void TagCheckBox::onChecked(int state)
{
  emit tagBoxChecked(tag_name_, state);
}
