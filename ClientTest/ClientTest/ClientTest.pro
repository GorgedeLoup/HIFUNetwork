#-------------------------------------------------
#
# Project created by QtCreator 2015-11-04T14:01:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ClientTest
TEMPLATE = app

INCLUDEPATH += ../lib\
               ../lib/Client\
               ../lib/config\
               ../lib/common

LIBS += ../bin/Client.dll

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
