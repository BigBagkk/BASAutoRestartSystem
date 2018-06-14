#-------------------------------------------------
#
# Project created by QtCreator 2017-12-08T21:18:17
#
#-------------------------------------------------

QT       += core gui
#串口程序一定要添加以下
QT       += serialport
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = BASAutoRestartSystem
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    comthread.cpp \
    netthread.cpp \
    switchcontrol.cpp

HEADERS  += mainwindow.h \
    comthread.h \
    netthread.h \
    switchcontrol.h

FORMS    += mainwindow.ui

RESOURCES += \
    images.qrc
