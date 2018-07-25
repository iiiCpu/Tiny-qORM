#ifndef TEST2_H
#define TEST2_H


#include "orm_def.h"
#include <QDebug>

namespace Test2
{
    struct Mom : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QString name MEMBER m_name)
        Q_PROPERTY(She is MEMBER m_is)
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
        She m_is;

        bool operator !=(Mom const& no) const { return m_name != no.m_name; }
        //Mom& operator =(Mom const& no) {  m_name = no.m_name; m_is = no.m_is; return *this; }
    };

    struct Car : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(double gas MEMBER m_gas)
    public:
        double m_gas;
    Car& operator =(Car const& no) {  m_gas = no.m_gas; return *this; }
    };

    struct Dad : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QString name MEMBER m_name)
        Q_PROPERTY(Test2::Car * car MEMBER m_car)
    public:
        QString m_name;
        Car * m_car = nullptr; // lost somewhere
        bool operator !=(Dad const& no) const { return m_name != no.m_name; }
        //Dad& operator =(Dad const& no) {  m_name = no.m_name; if (m_car && no.m_car) { m_car = no.m_car; } ; return *this; }
    };

    struct Brother : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QString name MEMBER m_name)
        Q_PROPERTY(int last_combo MEMBER m_lastCombo)
        Q_PROPERTY(int total_punches MEMBER m_totalPunches)
    public:
        QString m_name;
        int m_lastCombo;
        int m_totalPunches;
        bool operator !=(Brother const& no) const { return m_name != no.m_name; }
        bool operator ==(Brother const& no) const { return m_name == no.m_name; }
        bool operator <(Brother const& no) const { return m_name < no.m_name; }
        Brother& operator =(Brother const& no) {  m_name = no.m_name;  m_lastCombo = no.m_lastCombo; m_totalPunches = no.m_totalPunches; return *this; }
    };

    struct Ur : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QString name MEMBER m_name)
        Q_PROPERTY(Test2::Mom mom MEMBER m_mama)
        Q_PROPERTY(Test2::Dad dad MEMBER m_papa)
        Q_PROPERTY(QList<Test2::Brother> bros  MEMBER m_bros )
        Q_PROPERTY(QList<int           > draws MEMBER m_draws)
        Q_PROPERTY(QHash<int           , int           > drows MEMBER m_drows)
        Q_PROPERTY(QMap <int           , Test2::Brother> drops MEMBER m_drops)
        Q_PROPERTY(QMap <Test2::Brother, Test2::Brother> drags MEMBER m_drags)
    public:
        QString m_name;
        Mom m_mama;
        Dad m_papa;
        QList<Brother> m_bros;
        QList<int> m_draws;
        QHash<int,int> m_drows;
        QMap<int,Brother> m_drops;
        QMap<Brother,Brother> m_drags;
        bool operator !=(Ur const& no) const { return m_name != no.m_name || m_mama != no.m_mama ||
                    m_papa != no.m_papa || m_bros != no.m_bros || m_draws != no.m_draws ||
                    m_drows != no.m_drows || m_drops != no.m_drops || m_drags != no.m_drags; }
        Ur& operator =(Ur const& no) {  m_name = no.m_name; m_mama = no.m_mama;
                                        m_papa = no.m_papa; m_bros = no.m_bros;
                                        m_draws = no.m_draws; m_drows = no.m_drows;
                                        m_drops = no.m_drops; m_drags = no.m_drags;
                                        return *this; }
    };
}

ORM_DECLARE_METATYPE_EX(Test2::Mom)
ORM_DECLARE_METATYPE_EX(Test2::Car)
ORM_DECLARE_METATYPE_EX(Test2::Dad)
ORM_DECLARE_METATYPE_EX(Test2::Brother)
ORM_DECLARE_METATYPE_EX(Test2::Ur)

#endif // TEST2_H
