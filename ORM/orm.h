/*********************************************
 *          made by iCpu with love           *
 * *******************************************/

#ifndef ORM_H
#define ORM_H

#include "orm_def.h"

#include <QMetaObject>
#include <QVariant>
#include <list>
#include <vector>
#include <QList>
#include <QVector>
#include <QSqlQuery>


template <typename T>  int  registerTypeORM     (const char * c = nullptr);
template <typename T>  int  registerQObjectORM  (const char * c = nullptr);

namespace orm_pointers { struct ORMPointerStub; }

class ORM
{
public:
    ORM(QString const& dbname = QString());
    virtual ~ORM();

    static void addPrimitiveType(int metaclassid);
    static void removePrimitiveType(int metaclassid);
    template <typename T> static void addPrimitiveType(){ addPrimitiveType(qMetaTypeId<T>()); }
    template <typename T> static void removePrimitiveType(){ removePrimitiveType(qMetaTypeId<T>()); }

    static void addPrimitiveStringType(int metaclassid);
    static void removePrimitiveStringType(int metaclassid);
    template <typename T> static void addPrimitiveStringType(){ addPrimitiveStringType(qMetaTypeId<T>()); }
    template <typename T> static void removePrimitiveStringType(){ removePrimitiveStringType(qMetaTypeId<T>()); }

    static void addPrimitiveRawType(int metaclassid);
    static void removePrimitiveRawType(int metaclassid);
    template <typename T> static void addPrimitiveRawType(){ addPrimitiveRawType(qMetaTypeId<T>()); }
    template <typename T> static void removePrimitiveRawType(){ removePrimitiveRawType(qMetaTypeId<T>()); }

    static void addPointerStub(orm_pointers::ORMPointerStub const&);

    static void addPairType         (int firstType, int secondType, int pairType);
    static void addContainerPairType(int firstType, int secondType, int pairType);


    virtual QString normalize(QString const& str) const;
    virtual QString TYPE(int type_id) const;

    QString databaseName() const;
    void setDatabaseName(const QString & databaseName);

    template <typename T>    void     create      (             );
    template <typename T>    QList<T> select      (             );
    template <typename T>    QList<T> selectObject(             );
    template <typename T>    void     insert      (      T  & t );
    template <typename T>    void     insert      (      T  * t );
    template <typename T>    void     insert      (QList<T> & tl);
    template <typename T>    void     delet       (      T  & t );
    template <typename T>    void     delet       (      T  * t );
    template <typename T>    void     delet       (QList<T> & tl);
    template <typename T>    void     update      (      T  & t );
    template <typename T>    void     update      (      T  * t );
    template <typename T>    void     update      (QList<T> & tl);
    template <typename T>    void     drop        (             );


    void     create      (int metatypeid);
    QVariant select      (int metatypeid);
    void     insert      (int metatypeid, QVariant & v);
    void     delet       (int metatypeid, QVariant & v);
    void     update      (int metatypeid, QVariant & v);
    void     drop        (int metatypeid);

protected:
    QString m_databaseName;
    QMap<QString, QSqlQuery> insertQueries;
    QMap<QString, QSqlQuery> updateQueries;
    QMap<QString, QSqlQuery> selectQueries;

    virtual QString generate_table_name  (QString const& parent_name, QString const& property_name, QString const& class_name) const;
    virtual long long get_last_inserted_rowid(QSqlQuery & query) const;
    virtual long long get_last_updated_rowid (QSqlQuery & query) const;
    virtual QString generate_create_query(QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types                       ) const;
    virtual QString generate_select_query(QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types, bool parent_orm_rowid) const;
    virtual QString generate_insert_query(QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types, bool parent_orm_rowid) const;
    virtual QString generate_update_query(QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types, bool parent_orm_rowid) const;
    virtual QString generate_delete_query(QString const& parent_name, QString const& property_name, QString const& class_name,                                                    bool parent_orm_rowid) const;
    virtual QString generate_drop_query  (QString const& parent_name, QString const& property_name, QString const& class_name                                                                          ) const;
public:
    void       meta_create   (QMetaObject const& meta,               QString const& parent_name = "t", QString const& property_name = QString()                            );
    QVariant   meta_select   (QMetaObject const& meta,               QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_insert   (QMetaObject const& meta, QVariant & v, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_update   (QMetaObject const& meta, QVariant & v, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_delete   (QMetaObject const& meta, QVariant & v, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_drop     (QMetaObject const& meta,               QString const& parent_name = "t", QString const& property_name = QString()                            );
    void     meta_create_pair(int usermetatype,              const QString &parent_name, QString const& property_name);
    QVariant meta_select_pair(int usermetatype,              const QString &parent_name, QString const& property_name, long long parent_orm_rowid);
    void     meta_insert_pair(int usermetatype, QVariant &v, const QString &parent_name, QString const& property_name, long long parent_orm_rowid);
    void     meta_update_pair(int usermetatype, QVariant &v, const QString &parent_name, QString const& property_name, long long parent_orm_rowid);
    void     meta_delete_pair(int usermetatype, QVariant &v, const QString &parent_name, QString const& property_name, long long parent_orm_rowid);
    void     meta_drop_pair  (int usermetatype,              const QString &parent_name, QString const& property_name);
};




template <typename T>
void ORM::create() {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        meta_create(*o);
    }
}
template <typename T>
QList<T> ORM::select() {
    QList<T> ret;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariantList si = meta_select(*o).toList();
        for (QVariant v : si) {
            ret << v.value<T>();
        }
    }
    return ret;
}
template <typename T>
void ORM::insert(T & t) {
    QVariant v = QVariant::fromValue(reinterpret_cast<void*>(&t));
    meta_insert(t.staticMetaObject, v);
}
template <typename T>
void ORM::insert(T * t) {
    if (t) {
        QVariant v = QVariant::fromValue(t);
        meta_insert(t->staticMetaObject, v);
    }
}
template <typename T>
void ORM::insert(QList<T> & tl) {
    for (T & t : tl) {
        QVariant v = QVariant::fromValue(reinterpret_cast<void*>(&t));
        meta_insert(t.staticMetaObject, v);
    }
}
template <typename T>
void ORM::delet(T & t) {
    QVariant v = QVariant::fromValue(reinterpret_cast<void*>(&t));
    meta_delete(t.staticMetaObject, v);
}
template <typename T>
void ORM::delet(T * t) {
    if (t) {
        QVariant v = QVariant::fromValue(t);
        meta_delete(t->staticMetaObject, v);
    }
}
template <typename T>
void ORM::delet(QList<T> & tl) {
    for (T & t : tl) {
        QVariant v = QVariant::fromValue(reinterpret_cast<void*>(&t));
        meta_delete(t.staticMetaObject, v);
    }
}
template <typename T>
void ORM::update(T & t) {
    QVariant v = QVariant::fromValue(reinterpret_cast<void*>(&t));
    meta_update(t.staticMetaObject, v);
}
template <typename T>
void ORM::update(T * t) {
    if (t) {
        QVariant v = QVariant::fromValue(t);
        meta_update(t->staticMetaObject, v);
    }
}
template <typename T>
void ORM::update(QList<T> & tl) {
    for (T & t : tl) {
        QVariant v = QVariant::fromValue(reinterpret_cast<void*>(&t));
        meta_update(t.staticMetaObject, v);
    }
}
template <typename T>
void ORM::drop() {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        meta_drop(*o);
    }
}



namespace orm_pointers {


    template <typename T>                 T* toPointer (T const& t){                 T* p = new T ; *p = t; return p; }
    template <typename T> QSharedPointer <T> toPointerS(T const& t){ QSharedPointer <T> p  (new T); *p = t; return p; }
    template <typename T> std::shared_ptr<T> toPointers(T const& t){ std::shared_ptr<T> p  (new T); *p = t; return p; }

    template <typename T> void* toVPointer (                T  const& t){ return reinterpret_cast<void*>(const_cast<T*>(&t       )); }
    template <typename T> void* toVPointerP(                T *       t){ return reinterpret_cast<void*>(                t        ); }
    template <typename T> void* toVPointerS(QSharedPointer <T> const& t){ return reinterpret_cast<void*>(const_cast<T*>( t.data())); }
    template <typename T> void* toVPointers(std::shared_ptr<T> const& t){ return reinterpret_cast<void*>(const_cast<T*>( t.get ())); }

    template <typename T>                 T* fromVoidP(void* t){ return                    reinterpret_cast<T*>(t) ; }
    template <typename T> QSharedPointer <T> fromVoidS(void* t){ return QSharedPointer <T>(reinterpret_cast<T*>(t)); }
    template <typename T> std::shared_ptr<T> fromVoids(void* t){ return std::shared_ptr<T>(reinterpret_cast<T*>(t)); }

    struct ORMPointerStub {
        int T =0;
        int pT=0;
        int ST=0;
        int WT=0;
        int sT=0;
        int wT=0;
    };

    template <typename T>
    ORMPointerStub registerTypePointers()
    {
        ORMPointerStub stub;
        stub.T = qMetaTypeId<T>();
        stub.pT = qRegisterMetaType<T*>();

        QMetaType::registerConverter<T , T*   >(&toPointer <T>);

        QMetaType::registerConverter<T , void*>(&toVPointer <T>);
        QMetaType::registerConverter<T*, void*>(&toVPointerP<T>);

        QMetaType::registerConverter<void*, T*>(&fromVoidP<T>);
        return stub;
    }
    template <typename T>
    ORMPointerStub registerTypePointersEx()
    {
        ORMPointerStub stub;
        stub.T = qMetaTypeId<T>();
        stub.pT = qRegisterMetaType<T*>();
        stub.ST = qRegisterMetaType<QSharedPointer <T>>();
        stub.WT = qRegisterMetaType<QWeakPointer   <T>>();
        stub.sT = qRegisterMetaType<std::shared_ptr<T>>();
        stub.wT = qRegisterMetaType<std::weak_ptr  <T>>();

        QMetaType::registerConverter<                T , void*>(&toVPointer <T>);
        QMetaType::registerConverter<                T*, void*>(&toVPointerP<T>);
        QMetaType::registerConverter<QSharedPointer <T>, void*>(&toVPointerS<T>);
        QMetaType::registerConverter<std::shared_ptr<T>, void*>(&toVPointers<T>);

        QMetaType::registerConverter<void*,                 T*>(&fromVoidP<T>);
        QMetaType::registerConverter<void*, QSharedPointer <T>>(&fromVoidS<T>);
        QMetaType::registerConverter<void*, std::shared_ptr<T>>(&fromVoids<T>);
        return stub;
    }
}

struct ORMQVariantPair : public ORMValue
{
    Q_GADGET
    Q_PROPERTY(QVariant key MEMBER key)
    Q_PROPERTY(QVariant value MEMBER value)
public:
    QVariant key, value;
    QVariant& operator[](int index){ return index == 0 ? key : value; }
};
Q_DECLARE_METATYPE(ORMQVariantPair)
uint qHash(ORMQVariantPair const& v) noexcept;

struct ORMPairStub
{
    Q_GADGET
};
Q_DECLARE_METATYPE(ORMPairStub)
struct ORMHashMapStub
{
    Q_GADGET
};
Q_DECLARE_METATYPE(ORMHashMapStub)


namespace orm_containers
{

    template <typename T>
    QList<T> qListFromQVariantList(QVariant const& v)
    {
        QList<T> list;
        QSequentialIterable si = v.value<QSequentialIterable>();
        for (QVariant const& v : si) {
            if(v.canConvert<T>()) {
                list << v.value<T>();
            }
        }
        return list;
    }
    template <typename T> QVector    <T>   qVectorFromQVariantList(QVariant const& v) { return qListFromQVariantList<T>(v).toVector              (); }
    template <typename T> std::list  <T>   stdListFromQVariantList(QVariant const& v) { return qListFromQVariantList<T>(v).toStdList             (); }
    template <typename T> std::vector<T> stdVectorFromQVariantList(QVariant const& v) { return qListFromQVariantList<T>(v).toVector().toStdVector(); }

    template <typename T>
    void registerTypeSequentialContainers()
    {
        qRegisterMetaType<QList      <T>>();
        qRegisterMetaType<QVector    <T>>();
        qRegisterMetaType<std::list  <T>>();
        qRegisterMetaType<std::vector<T>>();
        QMetaType::registerConverter<QVariantList, QList      <T>>(&(    qListFromQVariantList<T>));
        QMetaType::registerConverter<QVariantList, QVector    <T>>(&(  qVectorFromQVariantList<T>));
        QMetaType::registerConverter<QVariantList, std::list  <T>>(&(  stdListFromQVariantList<T>));
        QMetaType::registerConverter<QVariantList, std::vector<T>>(&(stdVectorFromQVariantList<T>));
    }


    template <typename K, typename T>
    QPair<K,T> pairFromPairStub(ORMQVariantPair const& ps)
    {
        QPair<K,T> pair;
        if (ps.key.canConvert<K>() && ps.value.canConvert<T>()) {
            pair.first = ps.key.value<K>();
            pair.second = ps.value.value<T>();
        }
        return pair;
    }
    template <typename K, typename T>
    QPair<K,T> pairFromQVariant(QVariant const& v)
    {
        QPair<K,T> pair;
        if (v.canConvert<ORMQVariantPair>()) {
            ORMQVariantPair ps = v.value<ORMQVariantPair>();
            if (ps.key.canConvert<K>() && ps.value.canConvert<T>()) {
                pair.first = ps.key.value<K>();
                pair.second = ps.value.value<T>();
            }
        }
        return pair;
    }
    template <typename K, typename T>
    QMap<K,T> qMapFromQVariantMap(QVariant const& v)
    {
        QMap<K,T> list;
        QAssociativeIterable ai = v.value<QAssociativeIterable>();
        QAssociativeIterable::const_iterator it = ai.begin();
        const QAssociativeIterable::const_iterator end = ai.end();
        for ( ; it != end; ++it) {
            if(it.key().canConvert<K>() && it.value().canConvert<T>()) {
                list.insert(it.key().value<K>(), it.value().value<T>());
            }
        }
        return list;
    }
    template <typename K, typename T>
    QMap<K,T> qMapFromQVariantList(QVariantList const& v)
    {
        QMap<K,T> list;
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (it->canConvert<ORMQVariantPair>()) {
                ORMQVariantPair p = it->value<ORMQVariantPair>();
                if (p.key.canConvert<K>() && p.value.canConvert<T>()) {
                    list.insertMulti(p.key.value<K>(), p.value.value<T>());
                }
            }
        }
        return list;
    }
    template <typename K, typename T>
    QHash<K,T> qHashFromQVariantList(QVariantList const& v)
    {
        QHash<K,T> list;
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (it->canConvert<ORMQVariantPair>()) {
                ORMQVariantPair p = it->value<ORMQVariantPair>();
                if (p.key.canConvert<K>() && p.value.canConvert<T>()) {
                    list.insertMulti(p.key.value<K>(), p.value.value<T>());
                }
            }
        }
        return list;
    }
    template <typename K, typename T>
    QMultiMap<K,T> qMultiMapFromQVariantMap(QVariant const& v)
    {
        QMap<K,T> list;
        QAssociativeIterable ai = v.value<QAssociativeIterable>();
        QAssociativeIterable::const_iterator it = ai.begin();
        const QAssociativeIterable::const_iterator end = ai.end();
        for ( ; it != end; ++it) {
            if(it.key().canConvert<K>() && it.value().canConvert<T>()) {
                list.insertMulti(it.key().value<K>(), it.value().value<T>());
            }
        }
        return list;
    }
    template <typename K, typename T>
    QHash<K,T> qHashFromQVariantHash(QVariant const& v)
    {
        QHash<K,T> list;
        QAssociativeIterable ai = v.value<QAssociativeIterable>();
        QAssociativeIterable::const_iterator it = ai.begin();
        const QAssociativeIterable::const_iterator end = ai.end();
        for ( ; it != end; ++it) {
            if(it.key().canConvert<K>() && it.value().canConvert<T>()) {
                list.insert(it.key().value<K>(), it.value().value<T>());
            }
        }
        return list;
    }
    template <typename K, typename T>
    QMultiHash<K,T> qMultiHashFromQVariantMultiHash(QVariant const& v)
    {
        QHash<K,T> list;
        QAssociativeIterable ai = v.value<QAssociativeIterable>();
        QAssociativeIterable::const_iterator it = ai.begin();
        const QAssociativeIterable::const_iterator end = ai.end();
        for ( ; it != end; ++it) {
            if(it.key().canConvert<K>() && it.value().canConvert<T>()) {
                list.insertMulti(it.key().value<K>(), it.value().value<T>());
            }
        }
        return list;
    }

    template <typename K, typename T>
    ORMQVariantPair toPairStub(QPair<K,T> const& v)
    {
        ORMQVariantPair ps;
        ps.key = QVariant::fromValue(v.first);
        ps.value = QVariant::fromValue(v.second);
        return ps;
    }

    template <typename K, typename T>
    QList<ORMQVariantPair> mapToPairListStub(QMap<K,T> const& v)
    {
        QList<ORMQVariantPair> psl;
        for (auto i = v.begin(); i != v.end(); ++i) {
            ORMQVariantPair ps;
            ps.key = QVariant::fromValue(i.key());
            ps.value = QVariant::fromValue(i.value());
            psl << ps;
        }
        return psl;
    }
    template <typename K, typename T>
    QList<ORMQVariantPair> hashToPairListStub(QHash<K,T> const& v)
    {
        QList<ORMQVariantPair> psl;
        for (auto i = v.begin(); i != v.end(); ++i) {
            ORMQVariantPair ps;
            ps.key = QVariant::fromValue(i.key());
            ps.value = QVariant::fromValue(i.value());
            psl << ps;
        }
        return psl;
    }

    template <typename K, typename T>
    QMap<K,T> mapFromPairListStub(QList<ORMQVariantPair> const& v)
    {
        QMap<K,T> map;
        for (auto i = v.begin(); i != v.end(); ++i) {
            if (i->key.canConvert<K>() && i->value.canConvert<T>()) {
                map.insertMulti(i->key.value<K>(), i->value.value<T>());
            }
        }
        return map;
    }
    template <typename K, typename T>
    QHash<K,T> hashFromPairListStub(QList<ORMQVariantPair> const& v)
    {
        QHash<K,T> map;
        for (auto i = v.begin(); i != v.end(); ++i) {
            if (i->key.canConvert<K>() && i->value.canConvert<T>()) {
                map.insertMulti(i->key.value<K>(), i->value.value<T>());
            }
        }
        return map;
    }


    template <typename K, typename T>
    void registerOrderedAssociativeContainers()
    {
        ORM::addPairType(qMetaTypeId<K>(), qMetaTypeId<T>(), qRegisterMetaType<QPair <K,T>>());
        QMetaType::registerConverter<QVariant, QPair<K,T>>(&(    pairFromQVariant<K,T>));
        QMetaType::registerConverter<ORMQVariantPair, QPair<K,T>>(&(    pairFromPairStub<K,T>));
        QMetaType::registerConverter<QPair<K,T>, ORMQVariantPair>(&(          toPairStub<K,T>));

        ORM::addContainerPairType(qMetaTypeId<K>(), qMetaTypeId<T>(), qRegisterMetaType<QMap <K,T>>());
      //qRegisterMetaType<QMultiMap  <K,T>>();
        QMetaType::registerConverter<QMap       <K,T>, QList <ORMQVariantPair>>(&(       mapToPairListStub<K,T>));
      //QMetaType::registerConverter<QMultiMap  <K,T>, QList <PairStub>>(&(       mapToPairListStub<K,T>));
        QMetaType::registerConverter<QVariantMap     , QMap       <K,T>>(&(     qMapFromQVariantMap<K,T>));
        QMetaType::registerConverter<QVariantList    , QMap       <K,T>>(&(     qMapFromQVariantList<K,T>));
      //QMetaType::registerConverter<QVariantMap     , QMultiMap  <K,T>>(&(qMultiMapFromQVariantMap<K,T>));
        QMetaType::registerConverter<QList <ORMQVariantPair>, QMap       <K,T>>(&(     mapFromPairListStub<K,T>));
    }
    template <typename K, typename T>
    void registerHashAssociativeContainers()
    {
        ORM::addPairType(qMetaTypeId<K>(), qMetaTypeId<T>(), qRegisterMetaType<QPair <K,T>>());
        QMetaType::registerConverter<QVariant, QPair<K,T>>(&(    pairFromQVariant<K,T>));
        QMetaType::registerConverter<ORMQVariantPair, QPair<K,T>>(&(    pairFromPairStub<K,T>));
        QMetaType::registerConverter<QPair<K,T>, ORMQVariantPair>(&(          toPairStub<K,T>));

        ORM::addContainerPairType(qMetaTypeId<K>(), qMetaTypeId<T>(), qRegisterMetaType<QHash <K,T>>());
      //qRegisterMetaType<QMultiHash  <K,T>>();
        QMetaType::registerConverter<QHash      <K,T>, QList <ORMQVariantPair>>(&(      hashToPairListStub<K,T>));
      //QMetaType::registerConverter<QMultiHash <K,T>, QList <PairStub>>(&(             hashToPairListStub<K,T>));
        QMetaType::registerConverter<QVariantMap     , QHash      <K,T>>(&(          qHashFromQVariantHash<K,T>));
        QMetaType::registerConverter<QVariantList    , QHash      <K,T>>(&(          qHashFromQVariantList<K,T>));
      //QMetaType::registerConverter<QVariantMap     , QMultiHash <K,T>>(&(qMultiHashFromQVariantMultiHash<K,T>));
        QMetaType::registerConverter<QList<ORMQVariantPair> , QHash      <K,T>>(&(           hashFromPairListStub<K,T>));
    }

    template <typename T, template <typename> class Container>
    QString primitiveToString(Container<T> const& c)
    {
        QStringList list;
        for (T const& t : c) {
            list << QString::number(t);
        }
        return list.join(";");
    }
    template <typename T, template <typename> class Container>
    Container<T> stringToPrimitive(QString const& c)
    {
        Container<T> result;
        QStringList l = c.split(";");
        for (QString s : l) {
            result.append(QVariant(s).value<T>());
        }
        return result;
    }


    template<template <typename> class Container>
    void registerPrimitiveTypeContainer() {
        ORM::addPrimitiveType(qRegisterMetaType<Container<         bool       >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<unsigned char       >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<  signed char       >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<         char       >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<unsigned short      >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<  signed short      >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<         short      >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<unsigned int        >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<  signed int        >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<         int        >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<unsigned long long  >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<  signed long long  >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<         long long  >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<         float      >>());
        ORM::addPrimitiveType(qRegisterMetaType<Container<         double     >>());

        QMetaType::registerConverter< Container<         bool       >, QString >(&(primitiveToString<          bool       , Container>));
        QMetaType::registerConverter< Container<unsigned char       >, QString >(&(primitiveToString< unsigned char       , Container>));
        QMetaType::registerConverter< Container<  signed char       >, QString >(&(primitiveToString<   signed char       , Container>));
        QMetaType::registerConverter< Container<unsigned short      >, QString >(&(primitiveToString< unsigned short      , Container>));
        QMetaType::registerConverter< Container<  signed short      >, QString >(&(primitiveToString<   signed short      , Container>));
        QMetaType::registerConverter< Container<unsigned int        >, QString >(&(primitiveToString< unsigned int        , Container>));
        QMetaType::registerConverter< Container<  signed int        >, QString >(&(primitiveToString<   signed int        , Container>));
        QMetaType::registerConverter< Container<unsigned long long  >, QString >(&(primitiveToString< unsigned long long  , Container>));
        QMetaType::registerConverter< Container<  signed long long  >, QString >(&(primitiveToString<   signed long long  , Container>));
        QMetaType::registerConverter< Container<         float      >, QString >(&(primitiveToString<          float      , Container>));
        QMetaType::registerConverter< Container<         double     >, QString >(&(primitiveToString<          double     , Container>));

        QMetaType::registerConverter<QString, Container<         bool       >>(&(stringToPrimitive<          bool       , Container>));
        QMetaType::registerConverter<QString, Container<unsigned char       >>(&(stringToPrimitive< unsigned char       , Container>));
        QMetaType::registerConverter<QString, Container<  signed char       >>(&(stringToPrimitive<   signed char       , Container>));
        QMetaType::registerConverter<QString, Container<unsigned short      >>(&(stringToPrimitive< unsigned short      , Container>));
        QMetaType::registerConverter<QString, Container<  signed short      >>(&(stringToPrimitive<   signed short      , Container>));
        QMetaType::registerConverter<QString, Container<unsigned int        >>(&(stringToPrimitive< unsigned int        , Container>));
        QMetaType::registerConverter<QString, Container<  signed int        >>(&(stringToPrimitive<   signed int        , Container>));
        QMetaType::registerConverter<QString, Container<unsigned long long  >>(&(stringToPrimitive< unsigned long long  , Container>));
        QMetaType::registerConverter<QString, Container<  signed long long  >>(&(stringToPrimitive<   signed long long  , Container>));
        QMetaType::registerConverter<QString, Container<         float      >>(&(stringToPrimitive<          float      , Container>));
        QMetaType::registerConverter<QString, Container<         double     >>(&(stringToPrimitive<          double     , Container>));
    }
}

template <typename T>
int registerTypeORM(const char * c)
{
    int type = 0;
    if (c) {
        type = qRegisterMetaType<T>(c);
    }
    else {
        type = qRegisterMetaType<T>();
    }

    ORM::addPointerStub(orm_pointers::registerTypePointers<T>());
    orm_containers::registerTypeSequentialContainers<T>();
    return type;
}

namespace orm_qobjects {
    typedef QObject* (*creators)();
    template <typename T> QObject* creator() { return new T(); }
    void addQObjectStub(int type, orm_qobjects::creators stub);
}

template <typename T>
int registerQObjectORM(const char * c)
{
    int type = qRegisterMetaType<T*>(c);
    orm_qobjects::addQObjectStub(type, &orm_qobjects::creator<T>);
    return type;
}

#endif // ORM_H
