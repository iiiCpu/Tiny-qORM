/*********************************************
 *          made by iCpu with love           *
 * *******************************************/

#ifndef ORM_DEF_H
#define ORM_DEF_H

#include <QMetaType>


#define ORM_DECLARE_METATYPE(Type) \
    Q_DECLARE_METATYPE(    Type   )\
    Q_DECLARE_METATYPE(    Type * )

#define ORM_DECLARE_METATYPE_QT_SMARTPOINTERS(Type) \
    Q_DECLARE_METATYPE(QSharedPointer < Type >     )\
    Q_DECLARE_METATYPE(QWeakPointer   < Type >     )

#define ORM_DECLARE_METATYPE_STD_SMARTPOINTERS(Type) \
    Q_DECLARE_METATYPE(std::shared_ptr< Type >      )\
    Q_DECLARE_METATYPE(std::weak_ptr  < Type >      )

#define ORM_DECLARE_METATYPE_ALL_SMARTPOINTERS(Type) \
    ORM_DECLARE_METATYPE_QT_SMARTPOINTERS (    Type )\
    ORM_DECLARE_METATYPE_STD_SMARTPOINTERS(    Type )

#define ORM_DECLARE_METATYPE_EX(Type)           \
    ORM_DECLARE_METATYPE                  (Type)\
    ORM_DECLARE_METATYPE_ALL_SMARTPOINTERS(Type)

#define ORM_DECLARE_OBJECT(Type)          \
    Q_DECLARE_METATYPE(QPointer < Type > )

#define ORM_DECLARE_OBJECT_ALL_SMARTPOINTERS(Type) \
    ORM_DECLARE_OBJECT                    (  Type )\
    ORM_DECLARE_METATYPE_QT_SMARTPOINTERS (  Type )\
    ORM_DECLARE_METATYPE_STD_SMARTPOINTERS(  Type )

#define ORM_PROPERTY(Type, Property, Member, ...) \
    protected: \
    Type * getP##Property() const { return const_cast<Type*>(&Member); } \
    void setP##Property(Type * a) { Member = *a; } \
    Q_PROPERTY(Type Property MEMBER Member __VA_ARGS__) \
    Q_PROPERTY(Type * orm_##Property READ getP##Property WRITE setP##Property DESIGNABLE false SCRIPTABLE false FINAL)


#define orm_rowid_name                      "orm_rowid"
#define orm_parent_rowid_name               "parent__orm_rowid"

#define orm_classInfo_rowid                 "orm_ci_rowid"
#define orm_classInfo_key                   "orm_ci_key"
#define orm_classInfo_unique                "orm_ci_unique"
#define orm_classInfo_not_null              "orm_ci_not_null"
#define orm_classInfo_autoincrement         "orm_ci_autoincrement"
#define orm_classInfo_skip                  "orm_ci_skip"
#define orm_classInfo_tableName             "orm_ci_table_name"
#define orm_classInfo_tableNameNoParent     "orm_ci_table_name_no_parent"
#define orm_classInfo_fieldName             "orm_ci_field_name"

#define ORM_CLASSINFO_ROWID(param)                   Q_CLASSINFO(orm_classInfo_rowid            , #param)
#define ORM_CLASSINFO_KEY(param)                     Q_CLASSINFO(orm_classInfo_key              , #param)
#define ORM_CLASSINFO_UNIQUE(param)                  Q_CLASSINFO(orm_classInfo_unique           , #param)
#define ORM_CLASSINFO_AUTOINCREMENT(param)           Q_CLASSINFO(orm_classInfo_autoincrement    , #param)
#define ORM_CLASSINFO_NOT_NULL(param)                Q_CLASSINFO(orm_classInfo_not_null         , #param)
#define ORM_CLASSINFO_SKIP(param)                    Q_CLASSINFO(orm_classInfo_skip             , #param)
#define ORM_CLASSINFO_TABLE_NAME(param)              Q_CLASSINFO(orm_classInfo_tableName        , #param)
#define ORM_CLASSINFO_TABLE_NAME_NO_PARENT           Q_CLASSINFO(orm_classInfo_tableNameNoParent, "true")
#define ORM_CLASSINFO_FIELD_NAME(property, field)    Q_CLASSINFO(orm_classInfo_fieldName        , #property "=" #field)


struct ORMValue
{
    ORM_CLASSINFO_ROWID(orm_rowid)
    Q_GADGET
    Q_PROPERTY(long long orm_rowid MEMBER m_orm_rowid)
public:
    long long m_orm_rowid = 0;
    virtual ~ORMValue() = default;
};
ORM_DECLARE_METATYPE(ORMValue)




#endif // ORM_DEF_H
