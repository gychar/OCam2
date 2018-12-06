#-------------------------------------------------
#
# Project created by QtCreator 2018-06-20T16:00:24
#
#-------------------------------------------------

QT       += core gui
QT       += testlib
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Ocam
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        ocam2_sdk.c \
        mainwindow.cpp \
    maths.cpp \
    dialog_histogram.cpp \
    g_param.cpp \
    qcustomplot.cpp \
    dialog_stats.cpp \
    zoomer.cpp \
    qgraphicsviewcustom.cpp

HEADERS += \
    ocam2_sdk.h \
    ocam2_pvt.h \
    mainwindow.h \
    global_parameter.h \
    terminal.h \
    maths.h \
    dialog_histogram.h \
    g_param.h \
    qcustomplot.h \
    dialog_stats.h \
    zoomer.h \
    qgraphicsviewcustom.h

FORMS += \
    mainwindow.ui \
    dialog_histogram.ui \
    dialog_stats.ui \
    zoomer.ui

PKGCONFIG += git
new_moc.
QMAKE_EXTRA_COMPILERS +=


INCLUDEPATH = /opt/euresys/multicam/include \
                /usr/local/include \
                /usr/local/include/opencv \
                /usr/local/include/opencv2

LIBS += -L/usr/include -lSDL
LIBS += -L/opt/euresys/multicam/include -lMultiCam
LIBS += -L/usr/X11R6/lib -L. -lclseremc -pthread
LIBS += /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_core.so    \
        /usr/local/lib/libopencv_imgproc.so \
        /usr/local/lib/libopencv_imgcodecs.so

include = -I/usr/X11R6/Include

DISTFILES += \
    MCL.cam \
    ../../../ocam-simu/srcdir/libocam2sdk/ocam2_descrambling.txt





