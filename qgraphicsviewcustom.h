#ifndef QGRAPHICSVIEWCUSTOM_H
#define QGRAPHICSVIEWCUSTOM_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QCursor>
#include <QDebug>

class QGraphicsViewCustom : public QGraphicsView
{
public:
    QGraphicsViewCustom(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event);
};

#endif // QGRAPHICSVIEWCUSTOM_H

