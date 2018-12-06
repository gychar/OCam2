#include "qgraphicsviewcustom.h"
#include "g_param.h"

QGraphicsViewCustom::QGraphicsViewCustom(QWidget *parent) : QGraphicsView (parent)
{
    setMouseTracking(true);
}

void QGraphicsViewCustom::mouseMoveEvent(QMouseEvent *event){
    this->setCursor(Qt::CrossCursor);
    int x = event->x();
    int y = event->y();
    g_zoom_x = x/4;
    g_zoom_y = y/4;
    QGraphicsView::mouseMoveEvent(event);
}
