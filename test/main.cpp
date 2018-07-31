#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "orm.h"

#include "test0.h"
#include "test1.h"
#include "test2.h"
#include "orm_templates.h"

#include <QMetaProperty>
#include <QDataStream>



QDebug operator<<(QDebug dbg, QStructures const& t) { return orm_toDebug(dbg, t); }

void test0()
{
    ormRegisterType<QStructures>();
    ORM_Config::addPrimitiveRawType<QRect>();

    QStructures s;

    s.m_QChar                 = QChar                     ('y');
    s.m_QString               = QString                   ("Yo, nigga!");
    s.m_QStringList           = QStringList               (s.m_QString.split(' '));
    s.m_QByteArray            = QByteArray                (s.m_QString.toLatin1());
    s.m_QBitArray             = QBitArray                 (25);
    s.m_QDate                 = QDate                     (QDate::currentDate().addDays(-8));
    s.m_QTime                 = QTime                     (QTime::currentTime().addSecs(33333));
    s.m_QDateTime             = QDateTime                 (QDateTime::currentDateTimeUtc());
    s.m_QUrl                  = QUrl                      ("https://github.com");
    s.m_QLocale               = QLocale                   (QLocale::Arabic, QLocale::Egypt);
    s.m_QRect                 = QRect                     (0, 0, 640, 480);
    s.m_QRectF                = QRectF                    (-320.5, -319.5, 319.5, 320.5);
    s.m_QSize                 = QSize                     (400, 300);
    s.m_QSizeF                = QSizeF                    (4096.5, 4095.6);
    s.m_QLine                 = QLine                     (100, 100, 200, 200);
    s.m_QLineF                = QLineF                    (1.2, 3.4, 5.6, 7.8);
    s.m_QPoint                = QPoint                    (0, 9);
    s.m_QPointF               = QPointF                   (3.14, 3.15);
    s.m_QRegExp               = QRegExp                   ("(0x[a-fA-F0-9]+)");
    s.m_QEasingCurve          = QEasingCurve              ();
    s.m_QUuid                 = QUuid                     (QUuid::createUuid());
    s.m_QModelIndex           = QModelIndex               ();
    s.m_QRegularExpression    = QRegularExpression        ("(0x[a-fA-F0-9]+)");
    s.m_QJsonValue            = QJsonValue                (23);
    s.m_QJsonObject           = QJsonObject               ();
    s.m_QJsonArray            = QJsonArray                ();
    s.m_QJsonDocument         = QJsonDocument             ();
    s.m_QPersistentModelIndex = QPersistentModelIndex     ();
    s.m_QByteArrayList        = QByteArrayList            ();
    s.m_QFont                 = QFont                     ("Courier", 3, 24);
    s.m_QPixmap               = QPixmap                   (400, 300);
    s.m_QBrush                = QBrush                    (QColor(Qt::yellow));
    s.m_QColor                = QColor                    (33, 48, 188);
    s.m_QPalette              = QPalette                  (QColor(Qt::black), QColor(Qt::lightGray), QColor(Qt::white), QColor(Qt::darkBlue), QColor(Qt::blue), QColor(Qt::red), QColor(Qt::cyan));
    s.m_QIcon                 = QIcon                     (s.m_QPixmap);
    s.m_QImage                = QImage                    (320, 240, QImage::Format_ARGB32);
    s.m_QPolygon              = QPolygon                  (s.m_QRect);
    s.m_QRegion               = QRegion                   (s.m_QRect);
    s.m_QBitmap               = QBitmap                   (s.m_QPixmap);
    s.m_QCursor               = QCursor                   (Qt::ArrowCursor);
    s.m_QKeySequence          = QKeySequence              (QKeySequence::Print);
    s.m_QPen                  = QPen                      (Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    s.m_QTextLength           = QTextLength               ();
    s.m_QTextFormat           = QTextFormat               (QTextFormat::BlockFormat);
    s.m_QMatrix               = QMatrix                   (1, 0, 0, 1, 1, 1);
    s.m_QTransform            = QTransform                (0, 0, 1, 0, 1, 0, 1, 0, 0);
    s.m_QMatrix4x4            = QMatrix4x4                (1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1);
    s.m_QVector2D             = QVector2D                 (6.4f, 6.4f);
    s.m_QVector3D             = QVector3D                 (1.3f, 2.4f, 3.5f);
    s.m_QVector4D             = QVector4D                 (1.4f, 2.5f, 3.6f, 4.7f);
    s.m_QQuaternion           = QQuaternion               (0.5, 7, 7, 7);
    s.m_QPolygonF             = QPolygonF                 (s.m_QPolygon);
    s.m_QSizePolicy           = QSizePolicy               (QSizePolicy::Expanding, QSizePolicy::Expanding);
    qDebug() << s;

    ORM orm;
    orm.drop<QStructures>();
    orm.create<QStructures>();
    orm.insert(s);
    QList<QStructures> mine = orm.select<QStructures>();
    qDebug() << mine;
}


QDebug operator<<(QDebug dbg, Test1::U const& ur) { return orm_toDebug(dbg, ur); }
bool operator!=(Test1::U const& u1, Test1::U const& u2) { return orm_nequal(u1, u2); }
bool operator==(Test1::U const& u1, Test1::U const& u2) { return orm_equal(u1, u2); }
bool operator< (Test1::U const& u1, Test1::U const& u2) { return orm_less(u1, u2); }

void test1()
{
    qRegisterMetaType<Test1::U>("Test1::U");
    QMetaType::registerComparators<Test1::U>();

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

QDebug operator<<(QDebug dbg, Test2::Dad const& ur) { return orm_toDebug(dbg, ur); }
QDebug operator<<(QDebug dbg, Test2::Mom const& ur) { return orm_toDebug(dbg, ur); }
QDebug operator<<(QDebug dbg, Test2::Car const& ur) { return orm_toDebug(dbg, ur); }
QDebug operator<<(QDebug dbg, Test2::Brother const& ur) { return orm_toDebug(dbg, ur); }

QDebug& operator<<(QDebug& dbg, Test2::Ur const& no) {
    QDebugStateSaver s(dbg);
    dbg.nospace() << no.m_name << no.m_mama << no.m_papa
                  << no.m_bros << no.m_draws << no.m_drows
                  << no.m_drops << no.m_drop << no.m_drags;
    return dbg;
}

void test2()
{
    ormRegisterType<Test2::Ur     >();
    ormRegisterType<Test2::Dad    >();
    ormRegisterType<Test2::Mom    >();
    ormRegisterType<Test2::Car    >();
    ormRegisterType<Test2::Brother>();
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
    ur1.m_drop.first = 3;
    ur1.m_drop.second = b12;
    ur1.m_drags[b12]=b12;

    //qDebug() << toString(ur1);

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
    //qDebug() << mine;

    if (mine.size() != 2) {
        qDebug() << "Test2: Size error";
        return;
    }
    if (mine[0] != ur1) {
        qDebug() << "Test2: value 1 error";
        qDebug() << mine[0];
        qDebug() << ur1;
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


    test0();
    test1();
    test2();


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
