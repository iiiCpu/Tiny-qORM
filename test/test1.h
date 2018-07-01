#ifndef TEST1_H
#define TEST1_H

#include <QMetaType>
#include "orm_def.h"

namespace Test1
{

    struct U
    {
        Q_GADGET
        Q_PROPERTY(int i MEMBER m_i)
    public:
        int m_i;
        U(int i = 38):m_i(i){}
    };
}
Q_DECLARE_METATYPE(Test1::U)

#endif // TEST1_H
