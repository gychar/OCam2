#include "zoomer.h"
#include "ui_zoomer.h"
#include "g_param.h"

Zoomer::Zoomer(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Zoomer)
{
    ui->setupUi(this);

    qApp->installEventFilter(this);
    this->setFixedSize(242,522);
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    display();
    ui->graphicsView->setMouseTracking(true);
}

Zoomer::~Zoomer()
{
    delete ui;
}

void Zoomer::display(){
    g_scene_zoom->clear();
    g_scene_zoom->addPixmap(QPixmap::fromImage(*g_zoom_image));
    ui->graphicsView->setScene(g_scene_zoom);
    ui->graphicsView->show();
}

void Zoomer::on_checkBox_toggled(bool checked)
{
    if(checked){
        this->setFixedSize(962,1002);
        ui->graphicsView->setMinimumHeight(962);
        ui->graphicsView->setMinimumWidth(962);
        g_zoom_full = true;
    }else{
        this->setFixedSize(242,522);
        ui->graphicsView->setMaximumHeight(482);
        ui->graphicsView->setMaximumWidth(242);
        g_zoom_full = false;
    }
}

bool Zoomer::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::MouseMove)
    {
//        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        ui->x_label->setText(QString::number(g_zoom_x));
        ui->y_label->setText(QString::number(g_zoom_y));
        int v = 0;
        // Zoom = full
        if(g_zoom_full){
            v = g_imageNormal[g_zoom_x+g_zoom_y*g_imgsize];
        }
        // Zoom = register
        else{
            int offsetx = g_zoom_id_register % 4;
            int offsety = g_zoom_id_register / 4;
            v = g_imageNormal[g_zoom_x+offsetx*g_mask_x+(g_zoom_y+offsety*g_mask_y)*g_imgsize];
        }
        ui->pixel_label->setText(QString::number(v));
    }
    if(g_zoom_mean_done){
        ui->mean_label->setText("mean: "+QString::number(g_zoom_mean));
    }

    return false;
}








