#include <QPoint>
#include <QRect>

static bool inHotspot(const QRect& rect, const QPoint& pos, float hotspot = 0.25f)
{
    int rowHeight = rect.height();
    return pos.y() > (rect.top() + hotspot*rowHeight) &&
           pos.y() < (rect.bottom() - hotspot*rowHeight);
}
