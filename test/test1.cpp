#include "test1.h"

#include "orm.h"

QDebug& operator<<(QDebug& dbg, Test1::U const& ur)
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
