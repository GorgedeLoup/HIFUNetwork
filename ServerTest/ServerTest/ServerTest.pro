#-------------------------------------------------
#
# Project created by QtCreator 2015-11-04T16:21:50
#
#-------------------------------------------------

QT       += core gui network 

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ServerTest
TEMPLATE = app

INCLUDEPATH += ../lib\
               ../lib/Server\
               ../lib/config\
               ../lib/common

LIBS += ../bin/Server.dll

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
