#ifndef DIALOG_STATS_H
#define DIALOG_STATS_H

#include <QDialog>
#include "g_param.h"
#include <QVector>
#include <QtAlgorithms>
#include "maths.h"

namespace Ui {
class Dialog_stats;
}

class Dialog_stats : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_stats(QWidget *parent = nullptr);
    ~Dialog_stats();

private slots:

    void on_Image_PB_clicked();

    void on_Image_Dark_PB_clicked();

    void on_Global_checkBox_toggled(bool checked);

    void on_PTC_PB_clicked();

    void on_PTC2_PB_clicked();

    void on_LnNdT_PB_clicked();

    void on_Linear_PB_clicked();

private:
    Ui::Dialog_stats *ui;
    vector<float> Linear_Regression(const vector<float>x, const vector<float>y, int length);
    void Add_Linear_Regression_Plot(vector<float>abr, double x_min, double x_max);
};

#endif // DIALOG_STATS_H
