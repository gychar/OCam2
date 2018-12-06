#include "mainwindow.h"
#include <QApplication>
#include "g_param.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    Zoomer zoom;
    zoom.show();
    return a.exec();
}
