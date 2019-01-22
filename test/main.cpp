#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#ifdef VLD
# include <vld.h>
#endif

#include <functional>

#include "orm.h"
#include "sqliteorm.h"

#include "test0.h"
#include "test1.h"
#include "test2.h"
#include "test3.h"
#include "test4.h"

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
        db.exec("PRAGMA synchronous = OFF;");
        db.exec("PRAGMA temp_store  = MEMORY;");
    }
    QApplication a(argc, argv);

#ifdef QT_DEBUG
    QObject::connect(QCoreApplication::instance(), &QObject::destroyed, [](QObject * d) {
        if (d == QCoreApplication::instance())
            for (auto obj : QCoreApplication::instance()->children())
            qDebug() << "automaticly deleted " << obj
                     << " from thread " << obj->thread()
                     << " Check for memory leaks."; });
#endif

    //test_QtStructures();
    //test1();
    test2();
    test3();
    test4();


    if (!(true)) {
        for (int i = 0; i < 0x00100000; ++i) {
            const QMetaObject * o = QMetaType::metaObjectForType(i);
            if (o) {
                qDebug() << QString("; %1; %2; %3; %4; %5; %6; %7; %8; %9; %10; %11; %12; %13; %14; %15; %16; %17;").arg(i,5).arg(o->className(), 48).arg(QMetaType::typeName(i),48)
                         .arg(orm::Impl::isIgnored(i)              , 1)
                         .arg(orm::Impl::isEnumeration(i)          , 1)
                         .arg(orm::Impl::isPrimitive(i)            , 1)
                         .arg(orm::Impl::isPrimitiveType(i)        , 1)
                         .arg(orm::Impl::isPrimitiveRaw(i)         , 1)
                         .arg(orm::Impl::isPrimitiveString(i)      , 1)
                         .arg(orm::Impl::isTypeForTable(i)         , 1)
                         .arg(orm::Impl::isGadget(i)               , 1)
                         .arg(orm::Impl::isQObject(i)              , 1)
                         .arg(orm::Impl::isPointer(i)              , 1)
                         .arg(orm::Impl::isWeakPointer(i)          , 1)
                         .arg(orm::Impl::isSequentialContainer(i)  , 1)
                         .arg(orm::Impl::isPair(i)                 , 1)
                         .arg(orm::Impl::isAssociativeContainer(i) , 1);
                for (int j = 0; j <  o->propertyCount(); ++j) {
                    qDebug() << "  " << o->property(j).userType() << QMetaType::typeName(o->property(j).userType()) << o->property(j).name();
                }
            }
            else {
                if (QMetaType::typeName(i)) {
                    qDebug() << QString("; %1; %2; %3; %4; %5; %6; %7; %8; %9; %10; %11; %12; %13; %14; %15; %16; %17;").arg(i,5).arg("", 48).arg(QMetaType::typeName(i),48)
                             .arg(orm::Impl::isIgnored(i)              , 1)
                             .arg(orm::Impl::isEnumeration(i)          , 1)
                             .arg(orm::Impl::isPrimitive(i)            , 1)
                             .arg(orm::Impl::isPrimitiveType(i)        , 1)
                             .arg(orm::Impl::isPrimitiveRaw(i)         , 1)
                             .arg(orm::Impl::isPrimitiveString(i)      , 1)
                             .arg(orm::Impl::isTypeForTable(i)         , 1)
                             .arg(orm::Impl::isGadget(i)               , 1)
                             .arg(orm::Impl::isQObject(i)              , 1)
                             .arg(orm::Impl::isPointer(i)              , 1)
                             .arg(orm::Impl::isWeakPointer(i)          , 1)
                             .arg(orm::Impl::isSequentialContainer(i)  , 1)
                             .arg(orm::Impl::isPair(i)                 , 1)
                             .arg(orm::Impl::isAssociativeContainer(i) , 1);
                }
            }
        }
        qDebug() << orm::Config();
    }

    return 0;
}
