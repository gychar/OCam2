#ifndef DIALOG_HISTOGRAM_H
#define DIALOG_HISTOGRAM_H

#include <QDialog>
#include <ocam2_sdk.h>
#include <ocam2_pvt.h>
#include <QPaintEvent>
#include <QPainter>
#include <QtTest>

namespace Ui {
class Dialog_Histogram;
}

class Dialog_Histogram : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_Histogram(QWidget *parent = nullptr);
    ~Dialog_Histogram();
    int hist[256];
    int max;
    int maxpos;
    QPaintEvent *event;
    void paintEvent(QPaintEvent *event);

private:
    Ui::Dialog_Histogram *ui;
};

#endif // DIALOG_HISTOGRAM_H
