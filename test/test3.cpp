#include "test3.h"

#include <QDebug>
#include "orm.h"

#include <QDateTime>

#define ArraySize 6000

void test3() {
    ormRegisterType<Test3::U1>();
    ormRegisterType<Test3::U2>();
    ormRegisterType<Test3::U3>();
    QList<Test3::U1> list, ret1, ret2;
    for (int i = 0; i < ArraySize; ++i) {
        Test3::U1 u1;
        u1.m_i = i;
        u1.m_u2.m_i = i;
        u1.m_u2.m_u3 = i;
        list << u1;
    }


    QSqlQuery query;

    query.exec("DROP TABLE test3_u1;");
    query.exec("CREATE TABLE test3_u1 (indexu1 int, indexu2 int, indexu3 int);");
    quint64 beginSelf = QDateTime::currentMSecsSinceEpoch();
    query.prepare("INSERT into test3_u1(indexu1, indexu2, indexu3) values (:i1, :i2, :i3);");
    for (int i = 0; i < ArraySize; ++i) {
        query.bindValue(":i1", list[i].m_i);
        query.bindValue(":i2", list[i].m_u2.m_i);
        query.bindValue(":i3", list[i].m_u2.m_u3.m_i);
        query.exec();
    }
    quint64 midSelf = QDateTime::currentMSecsSinceEpoch();
    query.exec("SELECT indexu1, indexu2, indexu3 FROM test3_u1;");
    while (query.next()) {
        int i1 = query.value("indexu1").toInt();
        int i2 = query.value("indexu2").toInt();
        int i3 = query.value("indexu3").toInt();
        Test3::U1 u1;
        u1.m_i = i1;
        u1.m_u2.m_i = i2;
        u1.m_u2.m_u3.m_i = i3;
        ret2 << u1;
    }
    quint64 endSelf = QDateTime::currentMSecsSinceEpoch();

    ORM orm;
    orm.drop<Test3::U1>();
    orm.create<Test3::U1>();
    quint64 beginOrm = QDateTime::currentMSecsSinceEpoch();
    orm.insert(list);
    quint64 midOrm = QDateTime::currentMSecsSinceEpoch();
    ret1 = orm.select<Test3::U1>();
    quint64 endOrm = QDateTime::currentMSecsSinceEpoch();


    qDebug() << " ORM: insert=" << midOrm-beginOrm   << " select=" << endOrm-midOrm  ;
    qDebug() << "self: insert=" << midSelf-beginSelf << " select=" << endSelf-midSelf;
}
