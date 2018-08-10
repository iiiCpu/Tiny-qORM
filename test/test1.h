#ifndef TEST1_H
#define TEST1_H

#include <QMetaType>
class QDebug;

void test1();

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

QDebug& operator<<(QDebug & dbg, Test1::U const& ur);
bool operator!=(Test1::U const& u1, Test1::U const& u2);
bool operator==(Test1::U const& u1, Test1::U const& u2);
bool operator< (Test1::U const& u1, Test1::U const& u2);


#endif // TEST1_H
