#ifndef TEST3_H
#define TEST3_H

#include "orm_def.h"
class QDebug;

void test3();

namespace Test3
{
    struct U3 : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(int index MEMBER m_i)
    public:
        int m_i;
        U3(int i = rand()):m_i(i){}
        bool operator !=(U3 const& o) const { return m_i != o.m_i; }
        U3& operator=(U3 const& o) { m_orm_rowid = o.m_orm_rowid; m_i = o.m_i; return *this; }
    };
    struct U2 : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(Test3::U3 u3    MEMBER m_u3)
        Q_PROPERTY(int       index MEMBER m_i )
    public:
        U3 m_u3;
        int m_i;
        U2(int i = rand()):m_i(i){}
        bool operator !=(U2 const& o) const { return m_i != o.m_i || m_u3 != o.m_u3; }
        U2& operator=(U2 const& o) { m_orm_rowid = o.m_orm_rowid; m_u3 = o.m_u3; m_i = o.m_i; return *this; }
    };
    struct U1 : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(Test3::U3* u3 MEMBER m_u3)
        Q_PROPERTY(Test3::U2 u2 MEMBER m_u2)
        Q_PROPERTY(int index MEMBER m_i)
    public:
        U3* m_u3 = nullptr;
        U2 m_u2;
        int m_i = 0;
        U1():m_i(0){}
        ~U1(){ delete m_u3; }
        U1& operator=(U1 const& o) { m_orm_rowid = o.m_orm_rowid; m_u2 = o.m_u2; m_i = o.m_i; return *this; }
    };
}
ORM_DECLARE_METATYPE(Test3::U3)
ORM_DECLARE_METATYPE(Test3::U2)
ORM_DECLARE_METATYPE(Test3::U1)




#endif // TEST3_H
