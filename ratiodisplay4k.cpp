#include "ratiodisplay4k.h"
#include "ui_ratiodisplay4k.h"
#include "g_param.h"

RatioDisplay4k::RatioDisplay4k(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RatioDisplay4k)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    display();
}

RatioDisplay4k::~RatioDisplay4k()
{
    delete ui;
}

void RatioDisplay4k::display()
{
    QCoreApplication::processEvents();
    ui->graphicsView->setScene(g_scene_inter_4k);
    ui->graphicsView->show();
    cout << "4k intermediate displayed" << endl;
}

void RatioDisplay4k::closeEvent(QCloseEvent *event)
{
    g_disp_ratio_4k_show = false;
}
