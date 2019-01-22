#ifndef TEST4_H
#define TEST4_H

#include "orm_def.h"
class QDebug;

void test4();

namespace Test4
{
    struct U1 : public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(int index MEMBER m_i)
    public:
        int m_i = 0;
        U1():m_i(0){}
        U1& operator=(U1 const& o) { m_orm_rowid = o.m_orm_rowid; m_i = o.m_i; return *this; }
    };
}
ORM_DECLARE_METATYPE(Test4::U1)

#endif // TEST4_H
