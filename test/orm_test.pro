#-------------------------------------------------
#
# Project created by QtCreator 2018-06-25T12:00:37
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

TARGET = orm_test
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

include(../ORM/ORM.pri)
#include(E:/Downloads/orm_test/orm_test/ORM/ORM.pri)


INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"

LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32/" -lvld
PRE_TARGETDEPS += "C:/Program Files (x86)/Visual Leak Detector/lib/Win32/vld.lib"


SOURCES += \
        main.cpp \
    test0.cpp \
    test1.cpp \
    test3.cpp \
    test2.cpp

HEADERS += \
    test1.h \
    test2.h \
    test0.h \
    test0.h \
    test1.h \
    test3.h \
    test2.h

FORMS +=
