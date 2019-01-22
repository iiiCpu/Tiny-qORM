#include "test1.h"

#include "sqliteorm.h"
#include <QDebug>

QDebug operator<<(QDebug dbg, Test1::U const& ur)
{
    return dbg << "Test1::U(i=" << ur.m_i << ")";
}
bool operator!=(Test1::U const& u1, Test1::U const& u2)
{
    return u1.m_i != u2.m_i;
}
bool operator==(Test1::U const& u1, Test1::U const& u2)
{
    return u1.m_i == u2.m_i;
}
bool operator< (Test1::U const& u1, Test1::U const& u2)
{
    return u1.m_i <  u2.m_i;
}

void test1()
{
    qDebug() << "test1";
    qRegisterMetaType<Test1::U>("Test1::U");

    QList<Test1::U> urs { Test1::U(1), Test1::U(3), Test1::U(7), Test1::U(5) };

    qDebug() << "test1::createORM";
    orm::SQLiteORM orm;
    qDebug() << "test1::drop";
    orm.drop<Test1::U>();
    qDebug() << "test1::create";
    orm.create<Test1::U>();
    qDebug() << "test1::insert";
    orm.insert(urs);
    qDebug() << "test1::select";
    QList<Test1::U> mine = orm.select<Test1::U>();


    if (mine.size() != urs.size()) {
        qDebug() << "Test1: Size error";
        return;
    }
    for (int i = 0; i < urs.size(); ++i) {
        if (urs[i] != mine[i]) {
            qDebug() << "Test1: value " << i << " error: " << urs[i] << mine[i];
            return;
        }
    }
    qDebug() << "Test1: passed";
}
