#ifndef QGRAPHICSITEMCUSTOM_H
#define QGRAPHICSITEMCUSTOM_H

#include <iostream>
#include <QWidget>
#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>
#include <QPainterPath>
#include <QRandomGenerator>

using namespace std;

class QGraphicsItemCustom : public QGraphicsItem
{
public:
    QGraphicsItemCustom(qreal a,qreal b,qreal c, qreal d);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
private:
    qreal x1, y1, x2, y2;
};

#endif // QGRAPHICSITEMCUSTOM_H
