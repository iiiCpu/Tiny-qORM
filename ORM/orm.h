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
#include <QPointer>
#include <QVector>
#include <QSqlQuery>


template <typename T>  int  ormRegisterType     (const char * c = nullptr);
template <typename T>  int  ormRegisterTypeEx   (const char * c = nullptr);
template <typename T>  int  ormRegisterQObject  (const char * c = nullptr);

namespace orm_pointers { struct ORMPointerStub; }


struct ORM_Config
{
    static void addIgnoredType(int meta_type_id);
    static bool isIgnoredType(int meta_type_id);
    static void removeIgnoredType(int meta_type_id);
    template <typename T> static void addIgnoredType(){ addIgnoredType(qMetaTypeId<T>()); }
    template <typename T> static bool isIgnoredType(){ return isIgnoredType(qMetaTypeId<T>()); }
    template <typename T> static void removeIgnoredType(){ removeIgnoredType(qMetaTypeId<T>()); }

    static void addPrimitiveType(int meta_type_id);
    static bool isPrimitiveType(int meta_type_id);
    static void removePrimitiveType(int meta_type_id);
    template <typename T> static void addPrimitiveType(){ addPrimitiveType(qMetaTypeId<T>()); }
    template <typename T> static bool isPrimitiveType(){ return isPrimitiveType(qMetaTypeId<T>()); }
    template <typename T> static void removePrimitiveType(){ removePrimitiveType(qMetaTypeId<T>()); }

    static void addPrimitiveStringType(int meta_type_id);
    static bool isPrimitiveStringType(int meta_type_id);
    static void removePrimitiveStringType(int meta_type_id);
    template <typename T> static void addPrimitiveStringType(){ addPrimitiveStringType(qMetaTypeId<T>()); }
    template <typename T> static bool isPrimitiveStringType(){ return isPrimitiveStringType(qMetaTypeId<T>()); }
    template <typename T> static void removePrimitiveStringType(){ removePrimitiveStringType(qMetaTypeId<T>()); }

    static void addPrimitiveRawType(int meta_type_id);
    static bool isPrimitiveRawType(int meta_type_id);
    static void removePrimitiveRawType(int meta_type_id);
    template <typename T> static void addPrimitiveRawType(){ addPrimitiveRawType(qMetaTypeId<T>()); }
    template <typename T> static bool isPrimitiveRawType(){ return isPrimitiveRawType(qMetaTypeId<T>()); }
    template <typename T> static void removePrimitiveRawType(){ removePrimitiveRawType(qMetaTypeId<T>()); }

    static void addPointerStub(orm_pointers::ORMPointerStub const&);
    static orm_pointers::ORMPointerStub getPointerStub(int metaTypeID);

    static void addPairType         (int firstTypeID, int secondTypeID, int pairTypeID);
    static void addContainerPairType(int firstTypeID, int secondTypeID, int pairTypeID);

};

class QDebug;
QDebug operator<< (QDebug dbg, ORM_Config const& config);

class ORM
{
    enum class QueryType {
        Create, Insert, Select, Update, Delete, Drop
    };

public:
    ORM(QString const& dbname = QString());
    virtual ~ORM();

    QString databaseName() const;
    void setDatabaseName(const QString & databaseName);

    template <typename T>    void     create           (                     );
    template <typename T>    QList<T> select           (                     );
    template <typename T>    QList<T*>selectObject     (                     );
    template <typename T>    QList<T> selectWhere      (QVariantMap const&  w);
    template <typename T>    QList<T*>selectObjectWhere(QVariantMap const&  w);
    template <typename T>    T        get              (long long          id);
    template <typename T>    T*       getObject        (long long          id);
    template <typename T>    void     insert           (      T          & t );
    template <typename T>    void     insert           (      T          * t );
    template <typename T>    void     insert           (QList<T>         & tl);
    template <typename T>    void     delet            (      T          & t );
    template <typename T>    void     delet            (      T          * t );
    template <typename T>    void     delet            (QList<T>         & tl);
    template <typename T>    void     update           (      T          & t );
    template <typename T>    void     update           (      T          * t );
    template <typename T>    void     update           (QList<T>         & tl);
    template <typename T>    void     drop             (                     );


    void     create      (int meta_type_id                           );
    QVariant select      (int meta_type_id                           );
    QVariant selectWhere (int meta_type_id, QVariantMap const& wheres);
    QVariant get         (int meta_type_id, long long              id);
    void     insert      (int meta_type_id, QVariant         &  value);
    void     delet       (int meta_type_id, QVariant         &  value);
    void     update      (int meta_type_id, QVariant         &  value);
    void     drop        (int meta_type_id                           );
public:
    void       meta_create       (QMetaObject const& meta,                            QString const& parent_name = "t", QString const& property_name = QString()                            );
    QVariant   meta_select       (QMetaObject const& meta,                            QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    QVariant   meta_select_where (QMetaObject const& meta, QVariantMap const& wheres, QString const& parent_name = "t", QString const& property_name = QString()                            );
    QVariant   meta_get          (QMetaObject const& meta, long long              id, QString const& parent_name = "t", QString const& property_name = QString()                            );
    void       meta_insert       (QMetaObject const& meta, QVariant         &  value, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_update       (QMetaObject const& meta, QVariant         &  value, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_delete       (QMetaObject const& meta, QVariant         &  value, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_drop         (QMetaObject const& meta,                            QString const& parent_name = "t", QString const& property_name = QString()                            );
    void     meta_create_pair(int meta_type_id,                  const QString &parent_name, QString const& property_name);
    QVariant meta_select_pair(int meta_type_id,                  const QString &parent_name, QString const& property_name, long long parent_orm_rowid);
    void     meta_insert_pair(int meta_type_id, QVariant &value, const QString &parent_name, QString const& property_name, long long parent_orm_rowid);
    void     meta_update_pair(int meta_type_id, QVariant &value, const QString &parent_name, QString const& property_name, long long parent_orm_rowid);
    void     meta_delete_pair(int meta_type_id, QVariant &value, const QString &parent_name, QString const& property_name, long long parent_orm_rowid);
    void     meta_drop_pair  (int meta_type_id,                  const QString &parent_name, QString const& property_name);

protected:
    virtual QString normalize(QString const& str, QueryType queryType) const;
    virtual QString normalizeVar(QString const& str, int meta_type_id, QueryType queryType) const;
    virtual QString sqlType(int type_id) const;

    virtual QString generate_table_name        (QString const& parent_name, QString const& property_name, QString const& class_name, QueryType queryType) const;
    virtual long long get_last_inserted_rowid(QSqlQuery & query) const;
    virtual long long get_changes (QSqlQuery & query) const;
    virtual long long get_last_rowid (QString table_name) const;
    virtual QString generate_create_query      (QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types                           ) const;
    virtual QString generate_select_query      (QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types, bool parent_orm_rowid    ) const;
    virtual QString generate_select_where_query(QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types, QVariantMap const& wheres) const;
    virtual QString generate_get_query         (QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types                           ) const;
    virtual QString generate_insert_query      (QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types, bool parent_orm_rowid    ) const;
    virtual QString generate_update_query      (QString const& parent_name, QString const& property_name, QString const& class_name, QStringList const& names, QList<int> const& types, bool parent_orm_rowid    ) const;
    virtual QString generate_delete_query      (QString const& parent_name, QString const& property_name, QString const& class_name,                                                    bool parent_orm_rowid    ) const;
    virtual QString generate_drop_query        (QString const& parent_name, QString const& property_name, QString const& class_name                                                                              ) const;

protected:
    QString m_databaseName;
    QMap<QString, QSqlQuery> insertQueries;
    QMap<QString, QSqlQuery> updateQueries;
    QMap<QString, QSqlQuery> selectQueries;
    QMap<QString, QSqlQuery> getQueries;
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
QList<T*> ORM::selectObject() {
    QList<T*> ret;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariantList si = meta_select(*o).toList();
        for (QVariant v : si) {
            T* t =  v.value<T*>();
            if (t) {
                ret << t;
            }
        }
    }
    return ret;
}

template <typename T>
QList<T> ORM::selectWhere(QVariantMap const&  w) {
    QList<T> ret;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariantList si = meta_select_where(*o, w).toList();
        for (QVariant v : si) {
            ret << v.value<T>();
        }
    }
    return ret;
}
template <typename T>
QList<T*> ORM::selectObjectWhere(QVariantMap const& w) {
    QList<T*> ret;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariantList si = meta_select_where(*o, w).toList();
        for (QVariant v : si) {
            T* t =  v.value<T*>();
            if (t) {
                ret << t;
            }
        }
    }
    return ret;
}

template <typename T> T ORM::get(long long id ) {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariantList si = meta_get(*o, id).toList();
        if (si.size()) {
            return si.first().value<T>();
        }
    }
    return T();
}
template <typename T> T* ORM::getObject(long long id ) {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariantList si = meta_get(*o, id).toList();
        if (si.size()) {
            return si.first().value<T*>();
        }
    }
    return nullptr;
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



namespace orm_pointers
{
    template <typename T> ORMPointerStub registerTypePointers();
    template <typename T> ORMPointerStub registerTypeSmartPointers();


    template <typename T>                 T* toPointer (T const& t){                 T* p = new T ; *p = t; return p; }
    template <typename T>        QPointer<T> toPointerP(T const& t){                 T* p = new T ; *p = t; return p; }
    template <typename T> QSharedPointer <T> toPointerS(T const& t){ QSharedPointer <T> p  (new T); *p = t; return p; }
    template <typename T> std::shared_ptr<T> toPointers(T const& t){ std::shared_ptr<T> p  (new T); *p = t; return p; }

    template <typename T> void* toVPointerT(                T  const& t){ return reinterpret_cast<void*>(const_cast<T*>(&t       )); }
    template <typename T> void* toVPointer (                T *       t){ return reinterpret_cast<void*>(                t        ); }
    template <typename T> void* toVPointerP(      QPointer <T>        t){ return reinterpret_cast<void*>(                t.data() ); }
    template <typename T> void* toVPointerS(QSharedPointer <T> const& t){ return reinterpret_cast<void*>(const_cast<T*>( t.data())); }
    template <typename T> void* toVPointers(std::shared_ptr<T> const& t){ return reinterpret_cast<void*>(const_cast<T*>( t.get ())); }

    template <typename T>                 T* fromVoid (void* t){ return                    reinterpret_cast<T*>(t) ; }
    template <typename T>       QPointer <T> fromVoidP(void* t){ return       QPointer <T>(reinterpret_cast<T*>(t)); }
    template <typename T> QSharedPointer <T> fromVoidS(void* t){ return QSharedPointer <T>(reinterpret_cast<T*>(t)); }
    template <typename T> std::shared_ptr<T> fromVoids(void* t){ return std::shared_ptr<T>(reinterpret_cast<T*>(t)); }

    struct ORMPointerStub {
        int T =0;   // T
        int pT=0;   // T*
        int PT=0;   // QPointer<T>
        int ST=0;   // QSharedPointer<T>
        int WT=0;   // QWeakPointer<T>
        int sT=0;   // std::shared_ptr<T>
        int wT=0;   // std::weak_ptr<T>
    };

    template <typename T> ORMPointerStub registerTypePointers()
    {
        ORMPointerStub stub = ORM_Config::getPointerStub(qMetaTypeId<T>());
        if (!stub.T) {
            stub.T = qMetaTypeId<T>();
        }
        if (!stub.pT) {
            stub.pT = qMetaTypeId<T*>() ? qMetaTypeId<T*>() : qRegisterMetaType<T*>();
            QMetaType::registerConverter< T , T*    >(&toPointer <T>);
            QMetaType::registerConverter< T , void* >(&toVPointerT<T>);
            QMetaType::registerConverter< T*, void* >(&toVPointer <T>);
            QMetaType::registerConverter< void*, T* >(&fromVoid <T>);
        }
        return stub;
    }
    template <typename T> ORMPointerStub registerTypeSmartPointers()
    {
        ORMPointerStub stub = ORM_Config::getPointerStub(qMetaTypeId<T>());
        if (!stub.ST) {
            stub.ST = qMetaTypeId< QSharedPointer<T> >() ? qMetaTypeId< QSharedPointer<T> >() : qRegisterMetaType< QSharedPointer<T> >();
            QMetaType::registerConverter< QSharedPointer<T>, void* >(&toVPointerS<T>);
            QMetaType::registerConverter< void*, QSharedPointer<T> >(&fromVoidS<T>);
        }
        if (!stub.WT) {
            stub.WT = qMetaTypeId< QWeakPointer<T> >() ? qMetaTypeId< QWeakPointer<T> >() : qRegisterMetaType< QWeakPointer<T> >();
        }
        if (!stub.sT) {
            stub.sT = qMetaTypeId< std::shared_ptr<T> >() ? qMetaTypeId< std::shared_ptr<T> >() : qRegisterMetaType< std::shared_ptr<T> >();
            QMetaType::registerConverter< void*, std::shared_ptr<T> >(&fromVoids<T>);
            QMetaType::registerConverter< std::shared_ptr<T>, void* >(&toVPointers<T>);
        }
        if (!stub.wT) {
            stub.wT = qMetaTypeId< std::weak_ptr<T> >() ? qMetaTypeId< std::weak_ptr<T> >() : qRegisterMetaType< std::weak_ptr<T> >();
        }

        return stub;
    }
    template <typename T> ORMPointerStub registerTypeObjectPointer()
    {
        ORMPointerStub stub = ORM_Config::getPointerStub(qMetaTypeId<T>());
        if (!stub.T) {
            stub.T = qMetaTypeId<T>();
        }
        if (!stub.pT) {
            stub.pT = qMetaTypeId<T*>();
        }
        if (!stub.PT) {
            stub.PT = qMetaTypeId< QPointer<T> >() ? qMetaTypeId<QPointer<T>>() : qRegisterMetaType< QPointer<T> >();
            QMetaType::registerConverter< QPointer<T>, void* >(&toVPointerP<T>);
            QMetaType::registerConverter< void*, QPointer<T> >(&fromVoidP<T>);
        }
        return stub;
    }
}

namespace orm_containers
{
    template <typename T> void registerSequentialContainers(); // QList/QVector/std::list/std::vector
    template <typename K, typename T> void registerQPair();    // QPair
    template <typename K, typename T> void registerQMap();     // QMap
    template <typename K, typename T> void registerQHash();    // QHash
    template<template <typename> class Container> void registerPrimitiveTypeContainer();

    struct ORM_QVariantPair //: public ORMValue
    {
        Q_GADGET
        Q_PROPERTY(QVariant key MEMBER key)
        Q_PROPERTY(QVariant value MEMBER value)
    public:
        QVariant key, value;
        QVariant& operator[](int index){ return index == 0 ? key : value; }
    };


    template <typename T> QList      <T>     qListFromQVariantList(QVariant const& v)
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

    template <typename T> void registerSequentialContainers()
    {
        qMetaTypeId<QList      <T>>() ? qMetaTypeId<QList      <T>>() : qRegisterMetaType<QList      <T>>();
        qMetaTypeId<QVector    <T>>() ? qMetaTypeId<QVector    <T>>() : qRegisterMetaType<QVector    <T>>();
        qMetaTypeId<std::list  <T>>() ? qMetaTypeId<std::list  <T>>() : qRegisterMetaType<std::list  <T>>();
        qMetaTypeId<std::vector<T>>() ? qMetaTypeId<std::vector<T>>() : qRegisterMetaType<std::vector<T>>();
        QMetaType::registerConverter<QVariantList, QList      <T>>(&(    qListFromQVariantList<T>));
        QMetaType::registerConverter<QVariantList, QVector    <T>>(&(  qVectorFromQVariantList<T>));
        QMetaType::registerConverter<QVariantList, std::list  <T>>(&(  stdListFromQVariantList<T>));
        QMetaType::registerConverter<QVariantList, std::vector<T>>(&(stdVectorFromQVariantList<T>));
    }


    template <typename K, typename T> QPair<K,T> qPairFromPairStub(ORM_QVariantPair const& ps)
    {
        QPair<K,T> pair;
        if (ps.key.canConvert<K>() && ps.value.canConvert<T>()) {
            pair.first = ps.key.value<K>();
            pair.second = ps.value.value<T>();
        }
        return pair;
    }
    template <typename K, typename T> QPair<K,T> qPairFromQVariant(QVariant const& v)
    {
        QPair<K,T> pair;
        if (v.canConvert<ORM_QVariantPair>()) {
            ORM_QVariantPair ps = v.value<ORM_QVariantPair>();
            if (ps.key.canConvert<K>() && ps.value.canConvert<T>()) {
                pair.first = ps.key.value<K>();
                pair.second = ps.value.value<T>();
            }
        }
        return pair;
    }
    template <typename K, typename T> QPair<K,T> qPairFromQVariantList(QVariantList const& vl)
    {
        QPair<K,T> pair;
        QVariant v;
        if (vl.size()) {
            v = vl.first();
        }
        if (v.canConvert<ORM_QVariantPair>()) {
            ORM_QVariantPair ps = v.value<ORM_QVariantPair>();
            if (ps.key.canConvert<K>() && ps.value.canConvert<T>()) {
                pair.first = ps.key.value<K>();
                pair.second = ps.value.value<T>();
            }
        }
        return pair;
    }
    template <typename K, typename T> QMap<K,T> qMapFromQVariantMap(QVariant const& v)
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
    template <typename K, typename T> QMap<K,T> qMapFromQVariantList(QVariantList const& v)
    {
        QMap<K,T> list;
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (it->canConvert<ORM_QVariantPair>()) {
                ORM_QVariantPair p = it->value<ORM_QVariantPair>();
                if (p.key.canConvert<K>() && p.value.canConvert<T>()) {
                    list.insertMulti(p.key.value<K>(), p.value.value<T>());
                }
            }
        }
        return list;
    }
    template <typename K, typename T> QHash<K,T> qHashFromQVariantList(QVariantList const& v)
    {
        QHash<K,T> list;
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (it->canConvert<ORM_QVariantPair>()) {
                ORM_QVariantPair p = it->value<ORM_QVariantPair>();
                if (p.key.canConvert<K>() && p.value.canConvert<T>()) {
                    list.insertMulti(p.key.value<K>(), p.value.value<T>());
                }
            }
        }
        return list;
    }
    template <typename K, typename T> QMultiMap<K,T> qMultiMapFromQVariantMap(QVariant const& v)
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
    template <typename K, typename T> QHash<K,T> qHashFromQVariantHash(QVariant const& v)
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
    template <typename K, typename T> QMultiHash<K,T> qMultiHashFromQVariantMultiHash(QVariant const& v)
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

    template <typename K, typename T> ORM_QVariantPair toQPairStub(QPair<K,T> const& v)
    {
        ORM_QVariantPair ps;
        ps.key = QVariant::fromValue(v.first);
        ps.value = QVariant::fromValue(v.second);
        return ps;
    }

    template <typename K, typename T> QList<ORM_QVariantPair> qMapToPairListStub(QMap<K,T> const& v)
    {
        QList<ORM_QVariantPair> psl;
        for (auto i = v.begin(); i != v.end(); ++i) {
            ORM_QVariantPair ps;
            ps.key = QVariant::fromValue(i.key());
            ps.value = QVariant::fromValue(i.value());
            psl << ps;
        }
        return psl;
    }
    template <typename K, typename T> QList<ORM_QVariantPair> qHashToPairListStub(QHash<K,T> const& v)
    {
        QList<ORM_QVariantPair> psl;
        for (auto i = v.begin(); i != v.end(); ++i) {
            ORM_QVariantPair ps;
            ps.key = QVariant::fromValue(i.key());
            ps.value = QVariant::fromValue(i.value());
            psl << ps;
        }
        return psl;
    }

    template <typename K, typename T> QMap<K,T> qMapFromPairListStub(QList<ORM_QVariantPair> const& v)
    {
        QMap<K,T> map;
        for (auto i = v.begin(); i != v.end(); ++i) {
            if (i->key.canConvert<K>() && i->value.canConvert<T>()) {
                map.insertMulti(i->key.value<K>(), i->value.value<T>());
            }
        }
        return map;
    }
    template <typename K, typename T> QHash<K,T> qHashFromPairListStub(QList<ORM_QVariantPair> const& v)
    {
        QHash<K,T> map;
        for (auto i = v.begin(); i != v.end(); ++i) {
            if (i->key.canConvert<K>() && i->value.canConvert<T>()) {
                map.insertMulti(i->key.value<K>(), i->value.value<T>());
            }
        }
        return map;
    }


    template <typename K, typename T> void registerQPair()
    {
        ORM_Config::addPairType(qMetaTypeId<K>(), qMetaTypeId<T>(),
                                qMetaTypeId<QPair <K,T>>() ? qMetaTypeId<QPair <K,T>>() : qRegisterMetaType<QPair <K,T>>());
        QMetaType::registerConverter<QVariant, QPair<K,T>>(&(qPairFromQVariant<K,T>));
        QMetaType::registerConverter<QVariantList, QPair<K,T>>(&(qPairFromQVariantList<K,T>));
        QMetaType::registerConverter<ORM_QVariantPair, QPair<K,T>>(&(qPairFromPairStub<K,T>));
        QMetaType::registerConverter<QPair<K,T>, ORM_QVariantPair>(&(toQPairStub<K,T>));
    }
    template <typename K, typename T> void registerQMap()
    {
        registerQPair<K,T>();

        ORM_Config::addContainerPairType(qMetaTypeId<K>(), qMetaTypeId<T>(),
                                         qMetaTypeId<QMap <K,T>>() ? qMetaTypeId<QMap <K,T>>() : qRegisterMetaType<QMap <K,T>>());
        QMetaType::registerConverter<QMap<K,T>, QList<ORM_QVariantPair>>(&(qMapToPairListStub<K,T>));
        QMetaType::registerConverter<QVariantMap            , QMap<K,T>>(&(qMapFromQVariantMap<K,T>));
        QMetaType::registerConverter<QVariantList           , QMap<K,T>>(&(qMapFromQVariantList<K,T>));
        QMetaType::registerConverter<QList <ORM_QVariantPair>, QMap<K,T>>(&(qMapFromPairListStub<K,T>));
    }
    template <typename K, typename T> void registerQHash()
    {
        registerQPair<K,T>();

        ORM_Config::addContainerPairType(qMetaTypeId<K>(), qMetaTypeId<T>(),
                                         qMetaTypeId<QHash <K,T>>() ? qMetaTypeId<QHash <K,T>>() : qRegisterMetaType<QHash <K,T>>());
        QMetaType::registerConverter<QHash<K,T>, QList<ORM_QVariantPair>>(&(qHashToPairListStub<K,T>));
        QMetaType::registerConverter<QVariantMap           , QHash<K,T>>(&(qHashFromQVariantHash<K,T>));
        QMetaType::registerConverter<QVariantList          , QHash<K,T>>(&(qHashFromQVariantList<K,T>));
        QMetaType::registerConverter<QList<ORM_QVariantPair>, QHash<K,T>>(&(qHashFromPairListStub<K,T>));
    }

    template <typename T, template <typename> class Container> QString primitiveToString(Container<T> const& c)
    {
        QStringList list;
        for (T const& t : c) {
            list << QString::number(t);
        }
        return list.join(";");
    }
    template <typename T, template <typename> class Container> Container<T> stringToPrimitive(QString const& c)
    {
        Container<T> result;
        QStringList l = c.split(";");
        for (QString s : l) {
            result.append(QVariant(s).value<T>());
        }
        return result;
    }


    template<template <typename> class Container> void registerPrimitiveTypeContainer() {
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<         bool       >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<unsigned char       >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<  signed char       >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<         char       >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<unsigned short      >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<  signed short      >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<         short      >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<unsigned int        >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<  signed int        >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<         int        >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<unsigned long long  >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<  signed long long  >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<         long long  >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<         float      >>());
        ORM_Config::addPrimitiveType(qRegisterMetaType<Container<         double     >>());

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
uint qHash(orm_containers::ORM_QVariantPair const& variantPair) noexcept;
Q_DECLARE_METATYPE(orm_containers::ORM_QVariantPair)

namespace orm_qobjects {
    typedef QObject* (*creators)();
    template <typename T> QObject* creator() { return new T(); }
    void addQObjectStub(int type, orm_qobjects::creators stub);
    bool hasQObjectStub(int meta_type_id);
}


template <typename T> int ormRegisterType(const char * c)
{
    int type = qMetaTypeId<T>();
    if (!type) {
        if (c) {
            type = qRegisterMetaType<T>(c);
        }
        else {
            type = qRegisterMetaType<T>();
        }
    }

    ORM_Config::addPointerStub(orm_pointers::registerTypePointers<T>());
    orm_containers::registerSequentialContainers<T>();
    return type;
}

template <typename T> int ormRegisterTypeEx(const char * c)
{
    int type = ormRegisterType<T>(c);
    ORM_Config::addPointerStub(orm_pointers::registerTypeSmartPointers<T>());
    return type;
}

template <typename T> int ormRegisterQObject(const char * c)
{
    if (orm_qobjects::hasQObjectStub(qMetaTypeId<T*>())) {
        return qMetaTypeId<T*>();
    }

    int type = qMetaTypeId<T*>() ? qMetaTypeId<T*>() : qRegisterMetaType<T*>(c);
    orm_qobjects::addQObjectStub(type, &orm_qobjects::creator<T>);
    return type;
}

#endif // ORM_H
