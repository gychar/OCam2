#include "display4k.h"
#include "ui_display4k.h"
#include "g_param.h"

Display4K::Display4K(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Display4K)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    display();
}

Display4K::~Display4K()
{
    delete ui;
}

void Display4K::display()
{
    QCoreApplication::processEvents();
    ui->graphicsView->setScene(g_scene_4k);
    ui->graphicsView->show();
    cout << "4k displayed" << endl;
}

void Display4K::closeEvent(QCloseEvent *event){
    g_disp4k_show = false;
}

void Display4K::resizeEvent(QResizeEvent *event)
{
    int width = this->width();
    int height = this->height();
    ui->graphicsView->setMaximumWidth(width);
    ui->graphicsView->setMaximumHeight(height);
    ui->graphicsView->setMinimumWidth(width);
    ui->graphicsView->setMinimumHeight(height);
}
