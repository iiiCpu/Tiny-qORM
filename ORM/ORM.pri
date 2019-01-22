
QT       += core sql

win32-msvc* {
    QMAKE_CXXFLAGS += /bigobj
}

g++ {
    CONFIG += c++11
}
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

SOURCES += \
    $$PWD/orm.cpp \
    $$PWD/sqliteorm.cpp

HEADERS +=     \
    $$PWD/orm_def.h \
    $$PWD/orm.h \
    $$PWD/orm_templates.h \
    $$PWD/sqliteorm.h
