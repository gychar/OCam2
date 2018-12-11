#ifndef QGRAPHICSVIEWCUSTOM_H
#define QGRAPHICSVIEWCUSTOM_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QCursor>
#include <QColor>
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include "qgraphicsitemcustom.h"
#include <QGraphicsItem>

class QGraphicsViewCustom : public QGraphicsView
{
public:
    QGraphicsViewCustom(QWidget *parent = nullptr);
    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int x1 = 0;
    unsigned int y1 = 0;
    unsigned int x2 = 0;
    unsigned int y2 = 0;
    static QGraphicsItemCustom *item;
    double GetMean(vector<short> img, unsigned int x1, unsigned int y1, unsigned int x2, unsigned y2);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // QGRAPHICSVIEWCUSTOM_H

