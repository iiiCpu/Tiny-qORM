#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

//#include <vld.h>

#include "orm.h"

#include "test0.h"
#include "test1.h"
#include "test2.h"
#include "test3.h"

#include <QMetaProperty>
#include <QDataStream>

int main(int argc, char *argv[])
{
    qSetMessagePattern("   [%{function}  %{line}] - %{message}");

    QSqlDatabase db;
    if (!db.isOpen()) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("orm.sqlite");
        if (!db.open()) {
            qDebug() << db.lastError().databaseText() << db.lastError().driverText();
        }
        db.exec("PRAGMA journal_mode = MEMORY;");
        db.exec("PRAGMA synchronous = NORMAL;");
    }



    QApplication a(argc, argv);


    //test0();
    test1();
    test2();
    test3();


    if (false) {
        for (int i = 0; i < 0x00100000; ++i) {
            const QMetaObject * o = QMetaType::metaObjectForType(i);
            if (o) {
                qDebug() << i << o->className() << QMetaType::typeName(i);
                for (int j = 0; j <  o->propertyCount(); ++j) {
                    qDebug() << "  " << o->property(j).userType() << QMetaType::typeName(o->property(j).userType()) << o->property(j).name();
                }
            }
            else {
                if (QMetaType::typeName(i)) {
                    qDebug() << i << QMetaType::typeName(i);
                }
            }
        }
        qDebug() << ORM_Config();
    }


    return 0;
}
