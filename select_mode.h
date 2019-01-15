#ifndef SELECT_MODE_H
#define SELECT_MODE_H

#include <QDialog>
#include "g_param.h"

namespace Ui {
class Select_Mode;
}

class Select_Mode : public QDialog
{
    Q_OBJECT

public:
    explicit Select_Mode(QWidget *parent = nullptr);
    ~Select_Mode();

signals:
    void Selected();

private slots:
    void on_OCam2_PB_clicked();

    void on_Simu4K_PB_clicked();

private:
    Ui::Select_Mode *ui;
};

#endif // SELECT_MODE_H
