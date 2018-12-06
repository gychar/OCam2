#include "dialog_stats.h"
#include "ui_dialog_stats.h"

Dialog_stats::Dialog_stats(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_stats)
{
    ui->setupUi(this);
}

Dialog_stats::~Dialog_stats()
{
    delete ui;
}

void Dialog_stats::on_Image_PB_clicked()
{
    if(!g_statistics_global_state){
        int register_num = ui->register_spinBox->value()-1;
        QVector<double> x(g_statistics_try),y(g_statistics_try);
        for (int j = 0; j < g_statistics_try; j++){
            x[j] = g_plot_I_D_Mean[register_num][j];
            y[j] = g_plot_I_Var[register_num][j];
            cout << j << "   y= " << y[j] << "    x= " << x[j] << endl;
        }
        qSort(x);
        qSort(y);
        ui->plot->addGraph();
        ui->plot->graph(0)->setPen(QPen(Qt::blue,3));
        ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
        ui->plot->graph(0)->setData(x,y);
        ui->plot->xAxis->setLabel("Average Signal - Offset [DN]");
        ui->plot->yAxis->setLabel("Temporal Noise Variance [DN²]");
        QString str = QString::number(ui->register_spinBox->value()) + "th register  Mean(Image-Dark) vs Variance(Image)";
        ui->Title_Label->setText(str);
        ui->plot->xAxis->setRange(0.9*x[0],1.1*x[g_statistics_try-1]);
        ui->plot->yAxis->setRange(0.8*y[0],1.2*y[g_statistics_try-1]);
        ui->plot->replot();
        vector<float> linear_regression = Linear_Regression(g_plot_I_D_Mean[register_num],g_plot_I_Var[register_num],g_statistics_try);
        Add_Linear_Regression_Plot(linear_regression,0.9*x[0],1.1*x[g_statistics_try-1]);
    }
    else{
        QVector<double> x(g_statistics_try),y(g_statistics_try);
        for (int j = 0; j < g_statistics_try; j++){
            x[j] = g_plot_g_I_D_Mean[j];
            cout << x[j] << endl;
            y[j] = g_plot_g_I_Var[j];
            cout << y[j] << endl;
        }
        qSort(x);
        qSort(y);
        ui->plot->addGraph();
        ui->plot->graph(0)->setPen(QPen(Qt::blue,3));
        ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
        ui->plot->graph(0)->setData(x,y);
        ui->plot->xAxis->setLabel("Average Signal - Offset [DN]");
        ui->plot->yAxis->setLabel("Temporal Noise Variance [DN²]");
        ui->Title_Label->setText("Global Mean(Image-Dark) vs Variance(Image)");
        ui->plot->xAxis->setRange(0.9*x[0],1.1*x[g_statistics_try-1]);
        ui->plot->yAxis->setRange(0.8*y[0],1.2*y[g_statistics_try-1]);
        vector<float> linear_regression = Linear_Regression(g_plot_g_I_D_Mean,g_plot_g_I_Var,g_statistics_try);
        Add_Linear_Regression_Plot(linear_regression,0.9*x[0],1.1*x[g_statistics_try-1]);
        ui->plot->replot();
    }
}

void Dialog_stats::on_Image_Dark_PB_clicked()
{
    if(!g_statistics_global_state){
        int register_num = ui->register_spinBox->value()-1;
        QVector<double> x(g_statistics_try),y(g_statistics_try);
        for (int j = 0; j < g_statistics_try; j++){
            x[j] = g_plot_I_D_Mean[register_num][j];
            y[j] = g_plot_I_D_Var[register_num][j];
        }
        qSort(x);
        qSort(y);
        ui->plot->addGraph();
        ui->plot->graph(0)->setPen(QPen(Qt::blue,3));
        ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
        ui->plot->graph(0)->setData(x,y);
        ui->plot->xAxis->setLabel("Average Signal - Offset [DN]");
        ui->plot->yAxis->setLabel("Temporal Noise Variance [DN²]");
        QString str = QString::number(ui->register_spinBox->value()) + "th register  Mean(Image-Dark) vs Variance(Image-Dark)";
        ui->Title_Label->setText(str);
        ui->plot->xAxis->setRange(0.9*x[0],1.1*x[g_statistics_try-1]);
        ui->plot->yAxis->setRange(0.8*y[0],1.2*y[g_statistics_try-1]);
        vector<float> linear_regression = Linear_Regression(g_plot_I_D_Mean[register_num],g_plot_I_D_Var[register_num],g_statistics_try);
        Add_Linear_Regression_Plot(linear_regression,0.9*x[0],1.1*x[g_statistics_try-1]);
        ui->plot->replot();
    }
    else{
        QVector<double> x(g_statistics_try),y(g_statistics_try);
        for (int j = 0; j < g_statistics_try; j++){
            x[j] = g_plot_g_I_D_Mean[j];
            cout << x[j] << endl;
            y[j] = g_plot_g_I_D_Var[j];
            cout << y[j] << endl;
        }
        qSort(x);
        qSort(y);
        ui->plot->addGraph();
        ui->plot->graph(0)->setPen(QPen(Qt::blue,3));
        ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
        ui->plot->graph(0)->setData(x,y);
        ui->plot->xAxis->setLabel("Average Signal - Offset [DN]");
        ui->plot->yAxis->setLabel("Temporal Noise Variance [DN²]");
        ui->Title_Label->setText("Global Mean(Image-Dark) vs Variance(Image-Dark)");
        ui->plot->xAxis->setRange(0.9*x[0],1.1*x[g_statistics_try-1]);
        ui->plot->yAxis->setRange(0.8*y[0],1.2*y[g_statistics_try-1]);
        vector<float> linear_regression = Linear_Regression(g_plot_g_I_D_Mean,g_plot_g_I_D_Var,g_statistics_try);
        Add_Linear_Regression_Plot(linear_regression,0.9*x[0],1.1*x[g_statistics_try-1]);
        ui->plot->replot();
    }
}

void Dialog_stats::on_Global_checkBox_toggled(bool checked)
{
    g_statistics_global_state = checked;
}

vector<float> Dialog_stats::Linear_Regression(const vector<float>x, const vector<float>y, int length){
    // output a, b and r. y = ax+b
    vector<float> out(3);
    float a = 0, b = 0, r = 0;
    float mean_x = 0 ,mean_y = 0;
    float a_numerator = 0, a_denominator = 0;
    float r_numerator = 0, r_denominator1 = 0, r_denominator2 = 0;
    // Calculate a
    for(int i = 0; i < length; i++){
        mean_x += x[i];
        mean_y += y[i];
    }
    mean_x /= length;
    mean_y /= length;
    for(int i = 0; i < length; i++){
        a_numerator += (x[i]-mean_x)*(y[i]-mean_y);
        a_denominator += pow((x[i]-mean_x),2);
    }
    a = a_numerator / a_denominator;
    // Calculate b
    b = mean_y - a * mean_x;
    // Calculate r
    for(int i = 0; i < length; i++){
        r_numerator += (x[i]-mean_x)*(y[i]-mean_y);
        r_denominator1 += pow((x[i]-mean_x),2);
        r_denominator2 += pow((y[i]-mean_y),2);
    }
    r = r_numerator / sqrt(r_denominator1*r_denominator2);
    out[0] = a;
    out[1] = b;
    out[2] = r;
    return out;
}

void Dialog_stats::Add_Linear_Regression_Plot(vector<float> abr, double x_min, double x_max){
    float a = abr[0];
    float b = abr[1];
    float r = abr[2];
    // Create curve
    int pts = 1000;
    QVector<double> x(pts),y(pts);
    double x_step = (x_max - x_min) / (pts - 1);
    for (int j = 0; j < pts; j++){
        x[j] = x_min + j * x_step;
        y[j] = a * x[j] + b;
    }
    qSort(x);
    qSort(y);
    ui->plot->addGraph();
    ui->plot->graph(1)->setPen(QPen(Qt::red,3));
    ui->plot->graph(1)->setData(x,y);
    ui->plot->replot();
    cout << "Coefficient of correlation r = " << r << endl;
    cout << "Slope is " << a << endl;
    ui->Slope_label->setText("Slope = " + QString::number(a));
    ui->CorrelationCoeff_label->setText("Correlation coefficient = " + QString::number(r));
}

void Dialog_stats::on_PTC_PB_clicked()
{
    QVector<double> x(g_statistics_try),y(g_statistics_try);
    for (int j = 0; j < g_statistics_try; j++){
        x[j] = g_PTC_Signal[j];
        y[j] = g_PTC_Sigma2Shot[j];
    }
    qSort(x);
    qSort(y);
    for(int j = 0; j < g_statistics_try; j++)
        cout << dec << "x" << j+1 << " = " << x[j] << " y" << j+1 << " = " << y[j] << endl;
    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(Qt::blue,3));
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
    ui->plot->graph(0)->setData(x,y);
    ui->plot->xAxis->setLabel("Signal DN");
    ui->plot->yAxis->setLabel("σ²shot DN²");
    ui->plot->xAxis->setRange(0.95*x[0],1.05*x[g_statistics_try-1]);
    ui->plot->yAxis->setRange(0.95*y[0],1.05*y[g_statistics_try-1]);
    vector<float> linear_regression = Linear_Regression(g_PTC_Signal,g_PTC_Sigma2Shot,g_statistics_try);
    Add_Linear_Regression_Plot(linear_regression,0.9*x[0],1.1*x[g_statistics_try-1]);
    ui->Title_Label->setText("Photon Transfer Curve: y = " + QString::number(linear_regression[0]) + "x + " + QString::number(linear_regression[1]));
    ui->plot->replot();
}

void Dialog_stats::on_PTC2_PB_clicked()
{
    QVector<double> x(g_statistics_try),y(g_statistics_try);
    for (int j = 0; j < g_statistics_try; j++){
        x[j] = g_PTC_Signal[j];
        y[j] = g_PTC_Sigma2Read[j];
    }
    qSort(x);
    qSort(y);
    for(int j = 0; j < g_statistics_try; j++)
        cout << dec << "x" << j+1 << " = " << x[j] << " y" << j+1 << " = " << y[j] << endl;ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(Qt::blue,3));
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
    ui->plot->graph(0)->setData(x,y);
    ui->plot->xAxis->setLabel("Signal DN");
    ui->plot->yAxis->setLabel("σ²Read DN²");
    ui->plot->xAxis->setRange(0.95*x[0],1.05*x[g_statistics_try-1]);
    ui->plot->yAxis->setRange(0.95*y[0],1.05*y[g_statistics_try-1]);
    vector<float> linear_regression = Linear_Regression(g_PTC_Signal,g_PTC_Sigma2Read,g_statistics_try);
    Add_Linear_Regression_Plot(linear_regression,0.9*x[0],1.1*x[g_statistics_try-1]);
    ui->Title_Label->setText("Photon Transfer Curve: y = " + QString::number(linear_regression[0]) + "x + " + QString::number(linear_regression[1]));
    ui->plot->replot();
}

void Dialog_stats::on_LnNdT_PB_clicked()
{
    QVector<double> x(g_n_t), y(g_n_t);
    for (int j = 0; j < g_n_t; j++){
        x[j] = j*g_t_length;
        y[j] = g_mean_histo_register[ui->register_spinBox->value()-1][j];
    }
//    qSort(x);
//    qSort(y);
    for(int j = 0; j < g_n_t; j++)
        cout << dec << "x" << j+1 << " = " << x[j] << " y" << j+1 << " = " << y[j] << endl;
    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(Qt::blue,3));
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
    ui->plot->graph(0)->setData(x,y);
//    ui->plot->xAxis->setLabel("Signal DN");
//    ui->plot->yAxis->setLabel("σ²Read DN²");
    ui->plot->xAxis->setRange(0.95*x[0],1.05*x[g_n_t-1]);
    double y_max = 0;
    for(int i = 0; i < g_n_t; i++){
        if(y[i] < y_max)
            y_max = y[i];
    }
    ui->plot->yAxis->setRange(0.95*y[0],1.05*y_max);
//    vector<float> linear_regression = Linear_Regression(g_PTC_Signal,g_PTC_Sigma2Read,g_statistics_try);
//    Add_Linear_Regression_Plot(linear_regression,0.9*x[0],1.1*x[g_statistics_try-1]);
//    ui->Title_Label->setText("Photon Transfer Curve: y = " + QString::number(linear_regression[0]) + "x + " + QString::number(linear_regression[1]));
    ui->plot->replot();
}

void Dialog_stats::on_Linear_PB_clicked()
{
    int left = ui->Left_LineEdit->text().toInt();
    int right = ui->Right_LineEdit->text().toInt();
    int length = (right-left)/g_t_length;
    vector<float> x(length,0), y(length,0);
    for(int i = 0; i < length; i++){
        x[i] = left + i * g_t_length;
        y[i] = g_mean_histo_register[ui->register_spinBox->value()-1][i+left/g_t_length];
    }
    vector<float> linear_regression = Linear_Regression(x,y,length);
    float a = linear_regression[0];
    float b = linear_regression[1];
    float r = linear_regression[2];
    // Create curve
    int pts = 1000;
    QVector<double> linear_x(pts),linear_y(pts);
    double x_step = double((right - left)) / double((pts - 1));
    for (int j = 0; j < pts; j++){
        linear_x[j] = left + j * x_step;
        linear_y[j] = a * linear_x[j] + b;
    }
    ui->plot->addGraph();
    ui->plot->graph(1)->setPen(QPen(Qt::red,3));
    ui->plot->graph(1)->setData(linear_x, linear_y);
    ui->plot->replot();
    cout << "Coefficient of correlation r = " << r << endl;
    cout << "Slope is " << abs(a) << endl;
    ui->Slope_label->setText("Slope = " + QString::number(abs(a)));
    ui->CorrelationCoeff_label->setText("Correlation coefficient = " + QString::number(r));
    cout << "Gain = " << float(1/abs(a)) << endl;
}
