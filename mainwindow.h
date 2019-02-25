#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Include Headers
#include <QMainWindow>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <SDL/SDL.h>
#include <multicam.h>
#include <ocam2_sdk.h>
#include <ocam2_pvt.h>
#include <algorithm>
#include <functional>
#include <QApplication>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QFileDialog>
#include <QtEndian>
#include <QKeyEvent>
#include <QTime>
#include <QDateTime>
#include <QTest>
#include <QTableWidgetItem>
#include <QMouseEvent>
#include <fstream>
#include <QThread>
#include <thread>
#include "terminal.h"
#include "maths.h"
#include "dialog_histogram.h"
#include "dialog_stats.h"
#include "zoomer.h"
#include "g_param.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "zoomer.h"
#include "display4k.h"
#include "select_mode.h"

using namespace std;
using namespace cv;

#ifndef BYTE
#define BYTE unsigned char
#endif


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Zoomer *Zoomer_win;
    Display4K *Disp4K_win;
    Select_Mode *Mode_win;
    QTimer *timer;
    bool eventFilter(QObject *obj, QEvent *ev);

signals:
    void Acqui4K_done();

public slots:
    void Mode_selected();
    void Process4K();

private slots:
    void on_Snap_shot_PB_clicked();

    void on_SingleSave_PB_clicked();

    void on_SingleLoad_PB_clicked();

    void on_Threshold_Slider_valueChanged(int value);

    void on_Frequency_Slider_valueChanged(int value);

    void on_SequenceSave_PB_clicked();

    void on_Preset_PB_clicked();

    void on_SequenceLoad_PB_clicked();

    void on_Exit_PB_clicked();

    void on_Median_Checkbox_stateChanged(int arg1);

    void on_Dark_PB_clicked();

    void on_BufferSave_PB_clicked();

    void on_BufferAcquire_PB_clicked();

    void on_BufferLoad_PB_clicked();

    void on_BufferDisplay_PB_clicked();

    void on_Max_checkBox_stateChanged(int arg1);

    void on_Run_PB_clicked(bool checked);

    void on_PTC_PB_clicked();

    void on_Darkimage_PB_clicked();

    void on_Dark_Checkbox_stateChanged(int arg1);

    void on_BP_Slider_valueChanged(int value);

    void on_WP_Slider_valueChanged(int value);

    void on_Statistics_PB_clicked();

    void on_PNG_PB_clicked();

    void on_Histo_PB_clicked();

    void on_Temp_PB_clicked();

    void on_Console_Input_LineEdit_returnPressed();

    void on_MaskStatistics_PB_clicked();

    void on_spinBox_valueChanged(int arg1);

    void on_ThreshSave_PB_clicked();

    void on_ThreshLoad_PB_clicked();

    void on_AcquireCycle_PB_clicked();

    void on_PlotStatistics_PB_clicked();

    void on_Save_PB_clicked();

    void on_LoadDark_PB_clicked();

    void on_Load_PB_clicked();

    void on_AddNoise_PB_clicked();

    void on_GaussianNoise_CheckBox_stateChanged(int arg1);

    void on_GetMask_PB_clicked();

    void on_AcquireSet_PB_clicked();

    void on_LoadSet_PB_clicked();

    void on_Test_PB_clicked();

    void on_ClearHistory_PB_clicked();

    void on_BufferAllocate_PB_clicked();

    void on_ReadNoise_Checkbox_stateChanged(int arg1);

    void on_Overillumination_Reset_PB_clicked();

    void on_Gain_Histo_PB_clicked();

    void on_Zoom_PB_clicked();

    void on_Flat_PB_clicked();

    void on_LoadFlat_PB_clicked();

    void on_Flat_Checkbox_stateChanged(int arg1);

    void on_Load_Bias_PB_clicked();

    void onTimeOut();

    void on_Temp_n45_PB_clicked();

    void on_Temp_p30_PB_clicked();

    void on_Temp_off_PB_clicked();

    void on_Test4K_RB_clicked(bool checked);

    void on_Display_4K_PB_clicked();

    void on_Sampling_Button_toggled(bool checked);

    void on_MegaPixel_Button_toggled(bool checked);

    void on_pushButton_clicked();

    void on_MegaPixel_Mean_Button_toggled(bool checked);

    void on_MegaPixel_Median_Button_clicked(bool checked);

    void on_MegaPixel_Median_Button_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    // Initialize functions
    void test_error();
    void ChangeSection();
    void Init_Driver();
    int InitializeMultiCam();
    static void McCallback(PMCCALLBACKINFO CallBackInfo);
    static void *SignalHandler ( void * Param);
    void AcquireImages();
    void AcquireImages4K();
    void display(const short imagebuffer[]);
    void display_4K(const vector<unsigned short> imagebuffer);
    void display_4K_full(const vector<unsigned short> imagebuffer);
    void Ocam_Init();
    void pixel_correction(short img1[], short img2[]);
    void pixel_correction_4K(vector<short> img1, vector<short> img2);
    void threshold_function(short thresh);
    void InitBigImageBuffer();
    void SerialInit();
    void SerialCommand(QString str, int mode);
    vector<float> short2float(short in[], const int length);
    static void *ReadData(void * SerialRefPtr);
    void UpdateTemp();
    void GetTime();
    void ZoomImage();
    void mousePressEvent(QMouseEvent *event);
    vector<unsigned short> Sampling4k(const vector<unsigned short> img);
    vector<unsigned short> MegaPixel4k(const vector<unsigned short> img);
    void SplitReg();
    void Supp_OverScan();
    void Combine_Reg_4K();
    void test();
    void test2();

};

#endif // MAINWINDOW_H
