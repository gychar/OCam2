#include "select_mode.h"
#include "ui_select_mode.h"

Select_Mode::Select_Mode(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Select_Mode)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);
}

Select_Mode::~Select_Mode()
{
    delete ui;
}

void Select_Mode::on_OCam2_PB_clicked()
{
    g_test4K = false;
    cout << "ocam2" << endl;
    emit Selected();
    this->close();
}

void Select_Mode::on_Simu4K_PB_clicked()
{
    g_test4K = true;
    cout << "4K" << endl;
    emit Selected();
    this->close();
}
