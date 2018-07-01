/*********************************************
 *          made by iCpu with love           *
 * *******************************************/

#ifndef ORM_DEF_H
#define ORM_DEF_H

#include <QMetaObject>

#include <memory>
#include <QSharedPointer>
#include <QWeakPointer>


#define ORM_DECLARE_METATYPE(Type)              \
    Q_DECLARE_METATYPE(                 Type   )\
    Q_DECLARE_METATYPE(                 Type * )

#define ORM_DECLARE_METATYPE_EX(Type)           \
    Q_DECLARE_METATYPE(                 Type   )\
    Q_DECLARE_METATYPE(                 Type * )\
    Q_DECLARE_METATYPE(QSharedPointer < Type > )\
    Q_DECLARE_METATYPE(QWeakPointer   < Type > )\
    Q_DECLARE_METATYPE(std::shared_ptr< Type > )\
    Q_DECLARE_METATYPE(std::weak_ptr  < Type > )


struct ORMValue {
    Q_GADGET
    Q_PROPERTY(long long orm_rowid MEMBER m_orm_rowid)
public:
    long long m_orm_rowid = 0;
    virtual ~ORMValue(){}
};
ORM_DECLARE_METATYPE(ORMValue)

#endif // ORM_DEF_H
