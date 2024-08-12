#include "fomodcheckbox.h"


FomodCheckBox::FomodCheckBox(const QString& text,
                             const QString& description,
                             const QString& image_path,
                             QLabel* description_label,
                             QLabel* image_label) :
  description_(description), image_path_(image_path), description_label_(description_label),
  image_label_(image_label)
{
  setText(text);
}

void FomodCheckBox::enterEvent(QEvent* event)
{
  description_label_->setText(description_);
  QPixmap pixmap(image_path_);
  if(!pixmap.isNull())
    image_label_->setPixmap(
      pixmap.scaled({ std::min(512, pixmap.width()), pixmap.height() }, Qt::KeepAspectRatio));
  else
    image_label_->setPixmap(pixmap);
  QCheckBox::enterEvent(event);
}
