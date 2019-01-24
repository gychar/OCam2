#include "select_mode.h"
#include "ui_select_mode.h"

Select_Mode::Select_Mode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Select_Mode)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    ui->Simu4K_PB->setFocus();
}

Select_Mode::~Select_Mode()
{
    delete ui;
}

void Select_Mode::on_OCam2_PB_clicked()
{
    g_test4K = false;
    emit Selected();
    this->close();
}

void Select_Mode::on_Simu4K_PB_clicked()
{
    g_test4K = true;
    emit Selected();
    this->close();
}
