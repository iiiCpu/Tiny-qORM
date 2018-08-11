#include "test2.h"

#include <QDebug>
#include "orm.h"

#define debug_output false

Test2::Mom::Mom()
{
    m_is = Nice;
    if (debug_output) qDebug() << "Mom()" << this;
}
Test2::Mom::Mom(const Test2::Mom &no)
{
    if (debug_output) qDebug() << "Mom(Mom)" << this << &no;
    m_name = no.m_name;
    m_is = no.m_is;
}
Test2::Mom::~Mom()
{
    if (debug_output)
        qDebug() << "~Mom()" << this;
}
bool Test2::Mom::operator !=(const Test2::Mom &no) const
{
    bool ref = false;
    if (m_name != no.m_name) { if (debug_output) qDebug() << m_name << no.m_name; ref = true; }
    if (m_is   != no.m_is  ) { if (debug_output) qDebug() << m_is   << no.m_is  ; ref = true; }
    return ref;
}
bool Test2::Mom::operator ==(const Test2::Mom &no) const
{
    bool ref = true;
    if (m_name != no.m_name) { if (debug_output) qDebug() << m_name << no.m_name; ref = false; }
    if (m_is   != no.m_is  ) { if (debug_output) qDebug() << m_is   << no.m_is  ; ref = false; }
    return ref;
}
bool Test2::Mom::operator <(const Test2::Mom &no) const
{
    return m_name < no.m_name;
}
Test2::Mom &Test2::Mom::operator =(const Test2::Mom &no)
{
    m_name = no.m_name;
    m_is = no.m_is;
    if (debug_output) qDebug() << "Mom=" << this << &no;
    return *this;
}


Test2::Car::Car()
{
    if (debug_output) qDebug() << "Car()" << this;
}
Test2::Car::Car(const Test2::Car &no)
{
    if (debug_output) qDebug() << "Car(Car)" << this << &no;
}
Test2::Car::~Car()
{
    if (debug_output) qDebug() << "~Car()" << this;
}
Test2::Car &Test2::Car::operator =(const Test2::Car &no)
{
    m_gas = no.m_gas;
    if (debug_output) qDebug() << "Car=" << this << &no;
    return *this;
}
bool Test2::Car::operator !=(const Test2::Car &no) const
{
    bool ref = false;
    if (m_gas != no.m_gas) { if (debug_output) qDebug() << m_gas << no.m_gas; ref = true; }
    return ref;
}
bool Test2::Car::operator ==(const Test2::Car &no) const
{
    bool ref = true;
    if (m_gas != no.m_gas) { if (debug_output) qDebug() << m_gas << no.m_gas; ref = false; }
    return ref;
}
bool Test2::Car::operator <(const Test2::Car &no) const
{
    return m_gas < no.m_gas;
}


Test2::Dad::Dad()
{
    if (debug_output) qDebug() << "Dad()" << this;
    m_car  = nullptr;
}
Test2::Dad::Dad(const Test2::Dad &no)
{
    m_car = nullptr;
    m_name = no.m_name;
    if (no.m_car) {
        m_car = new Test2::Car();
        *m_car = *no.m_car;
    }
    if (debug_output) qDebug() << "Dad(Dad)" << this << &no << m_car << no.m_car;
}
Test2::Dad::~Dad()
{
    if (debug_output) qDebug() << "~Dad()" << this << m_car;
    if (m_car) {
        delete m_car;
        m_car = nullptr;
    }
}
bool Test2::Dad::operator !=(const Test2::Dad &no) const
{
    bool ref = false;
    if (m_name != no.m_name) { if (debug_output) qDebug() << m_name << no.m_name; ref = true; }
    if (m_car && no.m_car) {
        if (*m_car  != *no.m_car ) { if (debug_output) qDebug() << *m_car  << *no.m_car ; ref = true; }
    }
    else {
        if (m_car  != no.m_car ) { if (debug_output) qDebug() << m_car  << no.m_car ; ref = true; }
    }
    return ref;
}
bool Test2::Dad::operator ==(const Test2::Dad &no) const
{
    bool ref = true;
    if (m_name != no.m_name) { if (debug_output) qDebug() << m_name << no.m_name; ref = false; }
    if (m_car && no.m_car) {
        if (*m_car  != *no.m_car ) { if (debug_output) qDebug() << *m_car  << *no.m_car ; ref = false; }
    }
    else {
        if (m_car  != no.m_car ) { if (debug_output) qDebug() << m_car  << no.m_car ; ref = false; }
    }
    return ref;
}
Test2::Dad &Test2::Dad::operator =(const Test2::Dad &no)
{
    if (debug_output) qDebug() << "Dad=" << this << &no << m_car << no.m_car;
    m_name = no.m_name;
    if (m_car) {
        if (no.m_car) {
            *m_car = *no.m_car;
        }
        else {
            delete m_car;
            m_car = nullptr;
        }
    }
    else {
        if (no.m_car) {
            m_car = new Test2::Car();
            *m_car = *no.m_car;
        }
    }
    if (debug_output) qDebug() << "=Dad" << this << &no << m_car << no.m_car;
    return *this;
}


Test2::Brother::Brother()
{
    if (debug_output) qDebug() << "Brother()" << this;
}
Test2::Brother::Brother(const Test2::Brother &no)
{
    m_name         = no.m_name        ;
    m_lastCombo    = no.m_lastCombo   ;
    m_totalPunches = no.m_totalPunches;
    if (debug_output) qDebug() << "Brother(Brother)" << this << &no;
}
Test2::Brother::~Brother()
{
    if (debug_output) qDebug() << "~Brother()" << this;
}
bool Test2::Brother::operator !=(const Test2::Brother &no) const
{
    bool ref = false;
    if (m_name         != no.m_name        ) { if (debug_output) qDebug() << m_name         << no.m_name        ; ref = true; }
    if (m_lastCombo    != no.m_lastCombo   ) { if (debug_output) qDebug() << m_lastCombo    << no.m_lastCombo   ; ref = true; }
    if (m_totalPunches != no.m_totalPunches) { if (debug_output) qDebug() << m_totalPunches << no.m_totalPunches; ref = true; }
    return ref;
}
bool Test2::Brother::operator ==(const Test2::Brother &no) const
{
    bool ref = true;
    if (m_name         != no.m_name        ) { if (debug_output) qDebug() << m_name         << no.m_name        ; ref = false; }
    if (m_lastCombo    != no.m_lastCombo   ) { if (debug_output) qDebug() << m_lastCombo    << no.m_lastCombo   ; ref = false; }
    if (m_totalPunches != no.m_totalPunches) { if (debug_output) qDebug() << m_totalPunches << no.m_totalPunches; ref = false; }
    return ref;
}
bool Test2::Brother::operator <(const Test2::Brother &no) const
{
    return m_name < no.m_name;
}
Test2::Brother &Test2::Brother::operator =(const Test2::Brother &no)
{
    m_name         = no.m_name        ;
    m_lastCombo    = no.m_lastCombo   ;
    m_totalPunches = no.m_totalPunches;
    if (debug_output) qDebug() << "Brother=" << this << &no;
    return *this;
}


Test2::Ur::Ur()
{
    if (debug_output) qDebug() << "Ur()" << this;
}
Test2::Ur::Ur(const Test2::Ur &no)
{
    m_name  = no.m_name ;
    m_mama  = no.m_mama ;
    m_papa  = no.m_papa ;
    m_bros  = no.m_bros ;
    if (debug_output) qDebug() << "Ur(Ur)" << this << &no;
}
Test2::Ur::~Ur()
{
    if (debug_output) qDebug() << "~Ur()" << this;
}
bool Test2::Ur::operator !=(const Test2::Ur &no) const
{
    bool ref = false;
    if (m_name != no.m_name) { if (debug_output) qDebug() << m_name << no.m_name; ref = true; }
    if (m_mama != no.m_mama) { if (debug_output) qDebug() << m_mama << no.m_mama; ref = true; }
    if (m_papa != no.m_papa) { if (debug_output) qDebug() << m_papa << no.m_papa; ref = true; }
    if (m_bros != no.m_bros) { if (debug_output) qDebug() << m_bros << no.m_bros; ref = true; }
    return ref;
}

bool Test2::Ur::operator ==(const Test2::Ur &no) const
{
    bool ref = true;
    if (m_name != no.m_name) { if (debug_output) qDebug() << m_name << no.m_name; ref = false; }
    if (m_mama != no.m_mama) { if (debug_output) qDebug() << m_mama << no.m_mama; ref = false; }
    if (m_papa != no.m_papa) { if (debug_output) qDebug() << m_papa << no.m_papa; ref = false; }
    if (m_bros != no.m_bros) { if (debug_output) qDebug() << m_bros << no.m_bros; ref = false; }
    return ref;
}
Test2::Ur &Test2::Ur::operator =(const Test2::Ur &no)
{
    m_name  = no.m_name ;
    m_mama  = no.m_mama ;
    m_papa  = no.m_papa ;
    m_bros  = no.m_bros ;
    if (debug_output) qDebug() << "Ur=" << this << &no;
    return *this;
}



QDebug &operator<<(QDebug &dbg, const Test2::Dad &ur)
{
    if (ur.m_car) {
        return dbg << "Dad(name=" << ur.m_name << ", car=" << *ur.m_car << ")";
    }
    return dbg << "Dad(name=" << ur.m_name << ", car=nullptr)";
}

QDebug &operator<<(QDebug &dbg, const Test2::Mom &ur)
{
    return dbg << "Mom(name=" << ur.m_name << ", is=" << QVariant::fromValue(ur.m_is).value<QString>() << ")";
}

QDebug &operator<<(QDebug &dbg, const Test2::Car &ur)
{
    return dbg << "Car(gas=" << ur.m_gas << ")";
}

QDebug &operator<<(QDebug &dbg, const Test2::Brother &ur)
{
    return dbg << "Brother(name=" << ur.m_name << ", last_combo=" << ur.m_lastCombo << ", total_punches=" << ur.m_totalPunches << ")";
}

QDebug &operator<<(QDebug &dbg, const Test2::Ur &no)
{
    return dbg << "Ur("
               <<   "name=" << no.m_name
               << ", mama=" << no.m_mama
               << ", papa=" << no.m_papa
               << ", bros=" << no.m_bros
               << ")";
}

void test2()
{
    ormRegisterType<Test2::Ur     >();
    ormRegisterType<Test2::Dad    >();
    ormRegisterType<Test2::Mom    >();
    ormRegisterType<Test2::Car    >();
    ormRegisterType<Test2::Brother>();

    Test2::Ur ur1;
    ur1.m_name = "Phil";
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


    Test2::Ur ur2;
    ur2.m_name = "Bob";
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
        //qDebug() << mine[0];
        //qDebug() << ur1;
        return;
    }
    if (mine[1] != ur2) {
        qDebug() << "Test2: value 2 error";
        return;
    }
    qDebug() << "Test2: passed";
}
