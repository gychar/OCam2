#ifndef ZOOMER_H
#define ZOOMER_H

#include <QFrame>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QCursor>
#include <iostream>
#include "qgraphicsviewcustom.h"

using namespace std;

namespace Ui {
class Zoomer;
}

class Zoomer : public QFrame
{
    Q_OBJECT

public:
    explicit Zoomer(QWidget *parent = nullptr);
    ~Zoomer();
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:

    void on_checkBox_toggled(bool checked);

private:
    Ui::Zoomer *ui;
    void display();
//    void mousePressEvent(QMouseEvent *event);
};

#endif // ZOOMER_H
