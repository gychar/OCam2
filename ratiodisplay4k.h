#ifndef RATIODISPLAY4K_H
#define RATIODISPLAY4K_H

#include <QWidget>
#include <QFrame>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QCursor>
#include <iostream>
#include <QResizeEvent>
#include <unistd.h>
#include "qgraphicsviewcustom.h"

namespace Ui {
class RatioDisplay4k;
}

class RatioDisplay4k : public QWidget
{
    Q_OBJECT

public:
    explicit RatioDisplay4k(QWidget *parent = nullptr);
    ~RatioDisplay4k();


private:
    Ui::RatioDisplay4k *ui;
    void display();

protected:
    void closeEvent(QCloseEvent*event);
};

#endif // RATIODISPLAY4K_H
