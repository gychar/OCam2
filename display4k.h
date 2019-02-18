#ifndef DISPLAY4K_H
#define DISPLAY4K_H

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

namespace Ui {
class Display4K;
}

class Display4K : public QWidget
{
    Q_OBJECT

public:
    explicit Display4K(QWidget *parent = nullptr);
    ~Display4K();

private:
    Ui::Display4K *ui;
    void display();

protected:
    void closeEvent(QCloseEvent*event);
    void resizeEvent(QResizeEvent* event);
};

#endif // DISPLAY4K_H
