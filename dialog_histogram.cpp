#include "dialog_histogram.h"
#include "ui_dialog_histogram.h"

QPainter p;

Dialog_Histogram::Dialog_Histogram(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_Histogram)
{
    ui->setupUi(this);
    paintEvent(event);
}

Dialog_Histogram::~Dialog_Histogram()
{
    delete ui;
}

void Dialog_Histogram::paintEvent(QPaintEvent *event){
    QPainter p(this);
    QImage *histo = new QImage;
    histo = new QImage(this->width()-30,this->height(),QImage::Format_RGB888);
    histo->fill(qRgb(255,255,255));
    p.translate(0,histo->height());
    p.drawLine(0,0,100,100);
    int wid=histo->width();
    int hei=histo->height();
    float xstep = float(wid-40) / 256;
    float ystep = float(hei-40) / max;

    for (int i=0;i<256;i++)
    {
            p.setPen(Qt::blue);
            p.setBrush(Qt::blue);
            p.drawRect(10+i*xstep,-10,xstep,-ystep*hist[i]);
        if(i % 20 == 0||i==255)
        {
            p.drawText(QPointF(10+(i-0.5)*xstep,0),QString::number(i));
        }
    }
    int step = max/20;
    for(int i = 0; i < max; i++){
        if(i % step == 0)
        {
            p.drawText(QPointF(0,-10-(i-0.5)*ystep),QString::number(i));
        }
    }
    ui->Max_Label->setText("Max Value = " + QString::number(max));
    ui->MaxPos_Label->setText("Max Position = " + QString::number(maxpos));
    ui->MaxPercent_Label->setText("Max percent = " + QString::number(max*100/OCAM2_PIXELS_IMAGE_NORMAL) + "%");
}
