
QT       += core sql

win32-msvc* {
    QMAKE_CXXFLAGS += /bigobj
}

g++ {
    CONFIG += c++11
}

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

DEFINES += ORM_LIBRARY

SOURCES += \
    $$PWD/orm.cpp \
    $$PWD/orm_query_builder.cpp \
    $$PWD/orm_table_model.cpp \
    $$PWD/sqliteorm.cpp

HEADERS +=     \
    $$PWD/orm_def.h \
    $$PWD/orm.h \
    $$PWD/orm_query_builder.h \
    $$PWD/orm_table_model.h \
    $$PWD/orm_templates.h \
    $$PWD/sqliteorm.h
