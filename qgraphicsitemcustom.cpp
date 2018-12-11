#include "qgraphicsitemcustom.h"
#include "g_param.h"

QGraphicsItemCustom::QGraphicsItemCustom(qreal a, qreal b, qreal c, qreal d)
{
    x1 = (a < c) ? a : c;
    y1 = (b < d) ? b : d;
    x2 = (x1 == a) ? c : a;
    y2 = (y1 == b) ? d : b;
//    cout << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
}

QRectF QGraphicsItemCustom::boundingRect() const
{
    if((!g_zoom_full && x1 < 240 && y1 < 480 && x2 < 240 && y2 < 480) || (g_zoom_full && x1 < 960 && y1 < 960 && x2 < 960 && y2 < 960)){
        return QRectF(x1,y1,(x2-x1),(y2-y1));
    }
    return QRectF(0,0,0,0);
}

void QGraphicsItemCustom::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(QPen(Qt::red, 2));
//    painter->setPen(QColor::fromRgb(QRandomGenerator::global()->generate()));
    if(!g_zoom_full && x1 < 240 && y1 < 480 && x2 < 240 && y2 < 480){
        painter->drawRect(QRectF(x1,y1,(x2-x1),(y2-y1)));
    }else if(g_zoom_full && x1 < 960 && y1 < 960 && x2 < 960 && y2 < 960){
        painter->drawRect(QRectF(x1,y1,(x2-x1),(y2-y1)));
    }
}
