#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "orm.h"

#include "test1.h"
#include "test2.h"

#include <QMetaProperty>
#include <QDataStream>


QDebug operator<<(QDebug dbg, Test1::U const& ur)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "U(m_i=" << ur.m_i << ")";
    return dbg;
}

bool operator!=(Test1::U const& u1, Test1::U const& u2) {
    return u1.m_i != u2.m_i;
}

void test1()
{
    qRegisterMetaType<Test1::U>("Test1::U");

    QList<Test1::U> urs { Test1::U(1), Test1::U(3), Test1::U(7), Test1::U(5) };

    ORM orm;
    orm.drop<Test1::U>();
    orm.create<Test1::U>();
    orm.insert(urs);
    QList<Test1::U> mine = orm.select<Test1::U>();


    if (mine.size() != urs.size()) {
        qDebug() << "Test1: Size error";
        return;
    }
    if (urs[0] != mine[0]) {
        qDebug() << "Test1: value 1 error" << urs[0] << mine[0];
        return;
    }
    if (urs[1] != mine[1]) {
        qDebug() << "Test1: value 2 error";
        return;
    }
    if (urs[2] != mine[2]) {
        qDebug() << "Test1: value 3 error";
        return;
    }
    if (urs[3] != mine[3]) {
        qDebug() << "Test1: value 4 error";
        return;
    }
    qDebug() << "Test1: passed";
}


QDebug operator<<(QDebug dbg, const Test2::Mom &mom)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Mom(" << &mom << ", name=" << mom.m_name << ", is=" << mom.m_is << ")";
    return dbg;
}
QDebug operator<<(QDebug dbg, const Test2::Car &car)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Car(" << &car << ", gas=" << car.m_gas << ")";
    return dbg;
}
QDebug operator<<(QDebug dbg, const Test2::Dad &dad)
{
    QDebugStateSaver saver(dbg);
    if (dad.m_car)
        dbg.nospace() << "Dad(" << &dad << ", rowid=" << dad.m_orm_rowid
                      << ", name=" << dad.m_name << ", car=" << *dad.m_car << ")";
    else
        dbg.nospace() << "Dad(" << &dad << ", rowid=" << dad.m_orm_rowid
                      << ", name=" << dad.m_name << ", car={null})";
    return dbg;
}
QDebug operator<<(QDebug dbg, const Test2::Brother &bro)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Brother(" << &bro << ", name=" << bro.m_name << ", lastCombo=" << bro.m_lastCombo
                  << ", totalPunches=" << bro.m_totalPunches << ")";
    return dbg;
}
QDebug operator<<(QDebug dbg, const Test2::Ur &ur)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Ur(" << &ur << ", name=" << ur.m_name << ", mom=" << ur.m_mama
                  << ", dad=" << ur.m_papa << ", bros=" << ur.m_bros << ", draws=" << ur.m_draws << ur.m_drows << ur.m_drops << ur.m_drags << ")";
    return dbg;
}

Q_DECLARE_METATYPE(QFile*)

void test2()
{
    registerTypeORM<Test2::Ur>();
    registerTypeORM<Test2::Dad>();
    registerTypeORM<Test2::Mom>();
    registerTypeORM<Test2::Car>();
    registerTypeORM<Test2::Brother>();
    orm_containers::registerTypeSequentialContainers<int>();
    orm_containers::registerHashAssociativeContainers<int,int>();
    orm_containers::registerOrderedAssociativeContainers<int,Test2::Brother>();
    orm_containers::registerOrderedAssociativeContainers<Test2::Brother,Test2::Brother>();

    Test2::Ur ur1;
    ur1.m_name = "Phil";
    ur1.m_draws << 11 << 7 << 35 << 6;
    Test2::Brother b11;
    b11.m_name = "Bob";
    b11.m_lastCombo = 3;
    b11.m_totalPunches = 3;
    Test2::Brother b12;
    b12.m_name = "Mark";
    b12.m_lastCombo = 17;
    b12.m_totalPunches = 196;
    ur1.m_bros << b11 << b12;
    ur1.m_mama.m_name = "Margaret";
    ur1.m_mama.m_is = Test2::Mom::Beautiful;
    ur1.m_papa.m_name = "Phil";
    ur1.m_papa.m_car = new Test2::Car;
    ur1.m_papa.m_car->m_gas = 3.14;
    ur1.m_drows[3]=14;
    ur1.m_drops[3]=b12;
    ur1.m_drags[b12]=b12;

    Test2::Ur ur2;
    ur2.m_name = "Bob";
    ur2.m_draws << 1 << 2 << 3 << 99999;
    Test2::Brother b21;
    b21.m_name = "Sam";
    b21.m_lastCombo = 19992543;
    b21.m_totalPunches = 19992543;
    Test2::Brother b22;
    b22.m_name = "Max";
    b22.m_lastCombo = 0;
    b22.m_totalPunches = -6;
    ur2.m_bros << b21 << b22;
    ur2.m_mama.m_name = "Sophie";
    ur2.m_mama.m_is = Test2::Mom::Cozy;
    ur2.m_papa.m_name = "Abdul";

    ORM orm;
    orm.drop<Test2::Ur>();
    orm.create<Test2::Ur>();
    orm.insert(ur1);
    orm.insert(ur2);
    orm.update(ur2);
    QList<Test2::Ur> mine = orm.select<Test2::Ur>();
    orm.update(mine[1]);
    mine = orm.select<Test2::Ur>();
    qDebug() << mine;

    if (mine.size() != 2) {
        qDebug() << "Test2: Size error";
        return;
    }
    if (mine[0] != ur1) {
        qDebug() << "Test2: value 1 error";
        return;
    }
    if (mine[1] != ur2) {
        qDebug() << "Test2: value 2 error";
        return;
    }
    qDebug() << "Test2: passed";

    delete ur1.m_papa.m_car;
    delete mine[0].m_papa.m_car;
}

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



    test1();
    test2();


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

    return 0;
}
