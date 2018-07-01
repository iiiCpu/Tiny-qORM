
QT       += core sql

win32-msvc* {
    QMAKE_CXXFLAGS += /bigobj
}

g++ {
    CONFIG += c++14
}
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    $$PWD/orm.cpp

HEADERS +=     \
           $$PWD/orm_def.h \
    $$PWD/orm.h
