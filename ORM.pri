
QT       += core sql

win32-msvc* {
    QMAKE_CXXFLAGS += /bigobj
}

g++ {
    CONFIG += c++14
}


SOURCES += ORM.cpp
HEADERS += ORM.h orm_def.h
