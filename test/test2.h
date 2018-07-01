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
    ORM_DECLARE_METATYPE_EX(Mom)

    struct Car : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(double gas MEMBER m_gas)
    public:
        double m_gas;
    Car& operator =(Car const& no) {  m_gas = no.m_gas; return *this; }
    };
    ORM_DECLARE_METATYPE_EX(Car)

    struct Dad : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QString name MEMBER m_name)
        Q_PROPERTY(Car * car MEMBER m_car)
    public:
        QString m_name;
        Car * m_car = nullptr; // lost somewhere
        bool operator !=(Dad const& no) const { return m_name != no.m_name; }
        //Dad& operator =(Dad const& no) {  m_name = no.m_name; if (m_car && no.m_car) { m_car = no.m_car; } ; return *this; }
    };
    ORM_DECLARE_METATYPE_EX(Dad)

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
        Brother& operator =(Brother const& no) {  m_name = no.m_name;  m_lastCombo = no.m_lastCombo; m_totalPunches = no.m_totalPunches; return *this; }
    };
    ORM_DECLARE_METATYPE_EX(Brother)

    struct Ur : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QString name MEMBER m_name)
        Q_PROPERTY(Mom mom MEMBER m_mama)
        Q_PROPERTY(Dad dad MEMBER m_papa)
        Q_PROPERTY(QList<Brother> bros MEMBER m_bros)
        Q_PROPERTY(QList<int> draws MEMBER m_draws)
    public:
        QString m_name;
        Mom m_mama;
        Dad m_papa;
        QList<Brother> m_bros;
        QList<int> m_draws;
        bool operator !=(Ur const& no) const { return m_name != no.m_name || m_mama != no.m_mama ||
                    m_papa != no.m_papa || m_bros != no.m_bros || m_draws != no.m_draws; }
        Ur& operator =(Ur const& no) {  m_name = no.m_name; m_mama = no.m_mama; m_papa = no.m_papa; m_bros = no.m_bros; m_draws = no.m_draws;  ; return *this; }
    };
    ORM_DECLARE_METATYPE_EX(Ur)
}

#endif // TEST2_H
