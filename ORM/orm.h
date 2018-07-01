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


template <typename T>  int  registerTypeORM     (const char * c = nullptr);
template <typename T>  int  registerQObjectORM  (const char * c = nullptr);

namespace orm_pointers { struct ORMPointerStub; }

class ORM
{
public:
    ORM(QString const& dbname = QString());
    virtual ~ORM();

    static void addPrimitiveType(int metaclassid);
    template <typename T> static void addPrimitiveType(){ addPrimitiveType(qMetaTypeId<T>()); }
    static void removePrimitiveType(int metaclassid);
    template <typename T> static void removePrimitiveType(){ removePrimitiveType(qMetaTypeId<T>()); }
    static void addPointerStub(orm_pointers::ORMPointerStub const&);

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

public:
    void       meta_create   (QMetaObject const& meta,               QString const& parent_name = "t"                                                                      );
    QVariant   meta_select   (QMetaObject const& meta,               QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_insert   (QMetaObject const& meta, QVariant & v, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_update   (QMetaObject const& meta, QVariant & v, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_delete   (QMetaObject const& meta, QVariant & v, QString const& parent_name = "t", QString const& property_name = QString(), long long parent_rowid = 0);
    void       meta_drop     (QMetaObject const& meta,               QString const& parent_name = "t"                                                                      );
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
    QVariant v = QVariant::fromValue((void*)&t);
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
        QVariant v = QVariant::fromValue((void*)&t);
        meta_insert(t.staticMetaObject, v);
    }
}
template <typename T>
void ORM::delet(T & t) {
    QVariant v = QVariant::fromValue((void*)&t);
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
        QVariant v = QVariant::fromValue((void*)&t);
        meta_delete(t.staticMetaObject, v);
    }
}
template <typename T>
void ORM::update(T & t) {
    QVariant v = QVariant::fromValue((void*)&t);
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
        QVariant v = QVariant::fromValue((void*)&t);
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

    template <typename T> void* toVPointer (                T  const& t){ return (void*)const_cast<T*>(&t       ); }
    template <typename T> void* toVPointerP(                T *       t){ return (void*)                t        ; }
    template <typename T> void* toVPointerS(QSharedPointer <T> const& t){ return (void*)const_cast<T*>( t.data()); }
    template <typename T> void* toVPointers(std::shared_ptr<T> const& t){ return (void*)const_cast<T*>( t.get ()); }

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

        QMetaType::registerConverter<T ,                T*>(&toPointer <T>);

        QMetaType::registerConverter<                T , void*>(&toVPointer <T>);
        QMetaType::registerConverter<                T*, void*>(&toVPointerP<T>);

        QMetaType::registerConverter<void*,                 T*>(&fromVoidP<T>);
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

        QMetaType::registerConverter<T ,                T*>(&toPointer <T>);
        QMetaType::registerConverter<T, QSharedPointer <T>>(&toPointerS<T>);
        QMetaType::registerConverter<T, std::shared_ptr<T>>(&toPointers<T>);

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

namespace orm_containers {

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
    void registerTypeContainers()
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
        type = qRegisterMetaType<T >(c);
    }
    else {
        type = qRegisterMetaType<T>();
    }

    ORM::addPointerStub(orm_pointers::registerTypePointersEx<T>());
    orm_containers::registerTypeContainers<T>();
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
