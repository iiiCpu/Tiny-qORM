#ifndef TEST2_H
#define TEST2_H

#include "orm_def.h"
#include <QObject>
#include <QString>
#include <QMap>

#include <QSharedPointer>
#include <memory>

class QDebug;

void test2();

namespace Test2
{
    struct Mom : public ORMValue
    {
        Q_GADGET
        ORM_CLASSINFO_TABLE_NAME(Mama)
        Q_PROPERTY(QString name MEMBER m_name)
        Q_PROPERTY(She     is   MEMBER m_is  )
    public:
        enum She {
            Nice,
            Sweet,
            Beautiful,
            Pretty,
            Cozy,
            Fansy,
            Bear
        };
        Q_ENUM(She)
    public:
        QString m_name;
        She     m_is  ;

    public:
        Mom ();
        Mom (Mom const& no);
        ~Mom ();
        bool operator !=(Mom const& no) const;
        bool operator ==(Mom const& no) const;
        bool operator < (Mom const& no) const;
        Mom& operator = (Mom const& no)      ;
    };

    struct Car : public ORMValue
    {
        Q_GADGET
        ORM_CLASSINFO_TABLE_NAME(Car)
        ORM_CLASSINFO_TABLE_NAME_NO_PARENT
        Q_PROPERTY(double gas MEMBER m_gas)
    public:
        double m_gas;

    public:
        Car ();
        Car (Car const& no);
        ~Car();
        Car& operator = (Car const& no)      ;
        bool operator !=(Car const& no) const;
        bool operator ==(Car const& no) const;
        bool operator < (Car const& no) const;
    };

    struct Dad : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QString      name MEMBER m_name)
        Q_PROPERTY(Test2::Car * car  MEMBER m_car )
    public:
        QString m_name;
        Car *   m_car ; // lost somewhere

    public:
        Dad ();
        Dad (Dad const& no);
        ~Dad();
        bool operator !=(Dad const& no) const;
        bool operator ==(Dad const& no) const;
        Dad& operator = (Dad const& no)      ;
    };

    struct Brother : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QString name          MEMBER m_name        )
        Q_PROPERTY(int     last_combo    MEMBER m_lastCombo   )
        Q_PROPERTY(int     total_punches MEMBER m_totalPunches)
    public:
        QString m_name        ;
        int     m_lastCombo   ;
        int     m_totalPunches;

    public:
        Brother ();
        Brother (Brother const& no);
        ~Brother();
        bool     operator !=(Brother const& no) const;
        bool     operator ==(Brother const& no) const;
        bool     operator < (Brother const& no) const;
        Brother& operator = (Brother const& no)      ;
    };

    struct Ur : public QObject
    {
        Q_OBJECT
        ORM_CLASSINFO_TABLE_NAME(Ur)
        Q_PROPERTY(long long                 orm_rowid MEMBER m_id   )
        Q_PROPERTY(QString                   name      MEMBER m_name )
        Q_PROPERTY(Test2::Mom                mom       MEMBER m_mama )
        Q_PROPERTY(Test2::Dad                dad       MEMBER m_papa )
        Q_PROPERTY(QList<Test2::Brother>     bros      MEMBER m_bros )
        Q_PROPERTY(QMap<int, Test2::Brother> boros     MEMBER m_boros)
    public:
        long long                m_id   ;
        QString                  m_name ;
        Mom                      m_mama ;
        Dad                      m_papa ;
        QList<Brother>           m_bros ;
        QMap<int,Test2::Brother> m_boros;

    public:
        Q_INVOKABLE Ur (QObject * parent = nullptr);
        ~Ur();
        bool operator !=(Ur const& no) const;
        bool operator ==(Ur const& no) const;
        Ur&  operator = (Ur const& no)      ;
    };
}

ORM_DECLARE_METATYPE(Test2::Mom    )
ORM_DECLARE_METATYPE_QT_SMARTPOINTERS(Test2::Mom    )
ORM_DECLARE_METATYPE_STD_SMARTPOINTERS(Test2::Mom    )
ORM_DECLARE_METATYPE(Test2::Car    )
ORM_DECLARE_METATYPE(Test2::Dad    )
ORM_DECLARE_METATYPE(Test2::Brother)
//ORM_DECLARE_METATYPE(Test2::Ur     )

QDebug operator<<(QDebug dbg, Test2::Dad     const& ur);
QDebug operator<<(QDebug dbg, Test2::Mom     const& ur);
QDebug operator<<(QDebug dbg, Test2::Car     const& ur);
QDebug operator<<(QDebug dbg, Test2::Brother const& ur);
QDebug operator<<(QDebug dbg, QMap<int, Test2::Brother> const& ur);
QDebug operator<<(QDebug dbg, Test2::Ur      const& no);
QDebug operator<<(QDebug dbg, Test2::Car     const* ur);
QDebug operator<<(QDebug dbg, Test2::Ur      const* no);

#endif // TEST2_H
