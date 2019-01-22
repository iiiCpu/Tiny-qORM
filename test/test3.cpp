#include "test3.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "sqliteorm.h"
#include <QSqlDriver>

#include <QDateTime>

#include <sqlite3.h>

#define ArraySize 50000

void test3() {
    orm::Register<Test3::U1>();
    orm::Register<Test3::U2>();
    orm::Register<Test3::U3>();
    QList<Test3::U1> list, ret1, ret2, ret3;
    for (int i = 0; i < ArraySize; ++i) {
        Test3::U1 u1;
        u1.m_i = i;
        u1.m_u2.m_i = i;
        u1.m_u2.m_u3 = i;
        list << u1;
        if (i && i % (ArraySize/20) == 0) {
            qDebug() << i;
        }
    }


    QSqlQuery query;
    query.exec("DROP TABLE test3_u1;");
    query.exec("CREATE TABLE test3_u1 (indexu1 int, indexu2 int, indexu3 int);");
    qint64 beginSelf = QDateTime::currentMSecsSinceEpoch();
    query.prepare("INSERT into test3_u1(indexu1, indexu2, indexu3) values (:i1, :i2, :i3);");
    for (int i = 0; i < ArraySize; ++i) {
        query.bindValue(":i1", list[i].m_i);
        query.bindValue(":i2", list[i].m_u2.m_i);
        query.bindValue(":i3", list[i].m_u2.m_u3.m_i);
        query.exec();
        if (i && i % (ArraySize/20) == 0) {
            qDebug() << i;
        }
    }
    qint64 midSelf = QDateTime::currentMSecsSinceEpoch();
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
        int i = ret2.size();
        if (i && i % (ArraySize/20) == 0) {
            qDebug() << i;
        }
    }
    qint64 endSelf = QDateTime::currentMSecsSinceEpoch();

    orm::SQLiteORM orm;
    orm.drop<Test3::U1>();
    orm.create<Test3::U1>();
    qint64 beginOrm = QDateTime::currentMSecsSinceEpoch();
    orm.insert(list);
    qint64 midOrm = QDateTime::currentMSecsSinceEpoch();
    ret1 = orm.select<Test3::U1>();
    qint64 endOrm = QDateTime::currentMSecsSinceEpoch();

    qint64 beginRaw = 0;
    qint64 midRaw   = 0;
    qint64 endRaw   = 0;
    sqlite3 *db = nullptr;
    int rc = sqlite3_open( QSqlDatabase::database().databaseName().toUtf8().constData(), &db );
    if (!rc) {
        sqlite3_stmt *stmt1 = nullptr;
        rc = sqlite3_prepare(db, QString("PRAGMA journal_mode = MEMORY;").toUtf8().constData(), -1, &stmt1, nullptr);
        if (!rc) {
            sqlite3_step(stmt1);
            sqlite3_finalize(stmt1);
        }
        else {
            qDebug() << rc;
        }
        rc = sqlite3_prepare(db, QString("PRAGMA temp_store = MEMORY;").toUtf8().constData(), -1, &stmt1, nullptr);
        if (!rc) {
            sqlite3_step(stmt1);
            sqlite3_finalize(stmt1);
        }
        else {
            qDebug() << rc;
        }
        rc = sqlite3_prepare(db, QString("PRAGMA synchronous = OFF;").toUtf8().constData(), -1, &stmt1, nullptr);
        if (!rc) {
            sqlite3_step(stmt1);
            sqlite3_finalize(stmt1);
        }
        else {
            qDebug() << rc;
        }
        rc = sqlite3_prepare(db, QString("DROP TABLE test3_r_u1;").toUtf8().constData(), -1, &stmt1, nullptr);
        if (!rc) {
            sqlite3_step(stmt1);
            sqlite3_finalize(stmt1);
        }
        else {
            qDebug() << rc;
        }
        sqlite3_stmt *stmt2 = nullptr;
        rc = sqlite3_prepare(db, QString("CREATE TABLE test3_r_u1 (indexu1 int, indexu2 int, indexu3 int);").toUtf8().constData(), -1, &stmt2, nullptr);
        if (!rc) {
            sqlite3_step(stmt2);
            sqlite3_finalize(stmt2);
        }
        else {
            qDebug() << rc;
        }
        beginRaw = QDateTime::currentMSecsSinceEpoch();
        sqlite3_stmt *stmt3 = nullptr;
        rc = sqlite3_prepare(db, QString("INSERT into test3_r_u1(indexu1, indexu2, indexu3) values (?, ?, ?);").toUtf8().constData(), -1, &stmt3, nullptr);
        if (!rc) {
            for (int i = 0; i < ArraySize; ++i) {
                sqlite3_bind_int( stmt3, 1, list[i].m_i);
                sqlite3_bind_int( stmt3, 2, list[i].m_u2.m_i);
                sqlite3_bind_int( stmt3, 3, list[i].m_u2.m_u3.m_i);
                sqlite3_step(stmt3);
                if (i && i % (ArraySize/20) == 0) {
                    qDebug() << i;
                }
                sqlite3_reset(stmt3);
            }
            sqlite3_finalize(stmt3);
        }
        else {
            qDebug() << rc;
        }
        midRaw = QDateTime::currentMSecsSinceEpoch();
        sqlite3_stmt *stmt4 = nullptr;
        rc = sqlite3_prepare(db, QString("SELECT indexu1, indexu2, indexu3 FROM test3_r_u1;").toUtf8().constData(), -1, &stmt4, nullptr);
        if (!rc) {
            while (sqlite3_step( stmt4 ) == SQLITE_ROW) {
                int i1 = sqlite3_column_int( stmt4, 0 );
                int i2 = sqlite3_column_int( stmt4, 1 );
                int i3 = sqlite3_column_int( stmt4, 2 );
                Test3::U1 u1;
                u1.m_i = i1;
                u1.m_u2.m_i = i2;
                u1.m_u2.m_u3.m_i = i3;
                ret3 << u1;
                int i = ret3.size();
                if (i && i % (ArraySize/20) == 0) {
                    qDebug() << i;
                }
            }
        }
        else {
            qDebug() << rc;
        }
        sqlite3_finalize(stmt4);
        endRaw = QDateTime::currentMSecsSinceEpoch();
    }
    else {
        qDebug() << rc;
    }
    sqlite3_close( db );

    qDebug() << " ORM: insert=" << midOrm-beginOrm   << " select=" << endOrm-midOrm  ;
    qDebug() << "self: insert=" << midSelf-beginSelf << " select=" << endSelf-midSelf;
    qDebug() << " RAW: insert=" << midRaw-beginRaw   << " select=" << endRaw-midRaw  ;
}
