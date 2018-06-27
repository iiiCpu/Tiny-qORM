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

/**
 * @brief the registerTypeORM function registers some T subtypes making it accessable for ORM
 */
template <            typename T> void      registerTypeORM        (const char * c = nullptr);

/**
 * @brief
 * The ORM class is the most simple ORM engine you've seen in last year.
 * Or maybe in your entire life.
 *
 * @details
 * It allows you to create and drop tables for class, insert and select data,
 * and update\delete it if class have ROWIDs.
 *
 * No cache, no indexes, no 'where' clause. Just load and save data. No schema, no XML.
 * Versioning is not supported too.
 *
 * That's why it is VERY easy to use. Let's take your sample q_gadget structure:

         struct Ur {
             Q_GADGET
             Q_PROPERTY(int i MEMBER m_i)
         public:
             int m_i;
         };
         Q_DECLARE_METATYPE(Ur)

 * There are 1 simple step to make it work
 *  0)  Create ORM object and use it
 *

         ORM orm;
         orm.create<Ur>();
         QList<Ur> stff = orm.select<Ur>();
         orm.insert(stff);

 *
 * And that's all. You made it! Now you have database with Ur gadgets. Take the cookie, you deserve it!
 * Take note that there are no primary key, so every time you insert data you actually insert data.
 *
 * But what if you want to use it with more advanced structure? Like this!
 *

struct Mom {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(She is MEMBER m_is)
public:
    enum She {
        Nice,
        Sweet,
        Beautiful,
        Pretty,
        Cozy,
        Fansy,
        Bear
    }; Q_ENUM(She)
public:
    QString m_name;
    She m_is;

    bool operator !=(Mom const& no) { return m_name != no.m_name; }
};
Q_DECLARE_METATYPE(Mom)

struct Car {
    Q_GADGET
    Q_PROPERTY(double gas MEMBER m_gas)
public:
    double m_gas;
};
Q_DECLARE_METATYPE(Car)

struct Dad {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(Car * car MEMBER m_car)
public:
    QString m_name;
    Car * m_car = nullptr; // lost somewhere
    bool operator !=(Dad const& no) { return m_name != no.m_name; }
};
Q_DECLARE_METATYPE(Dad)

struct Brother {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(int last_combo MEMBER m_lastCombo)
    Q_PROPERTY(int total_punches MEMBER m_totalPunches)
public:
    QString m_name;
    int m_lastCombo;
    int m_totalPunches;
    bool operator !=(Brother const& no) { return m_name != no.m_name; }
    bool operator ==(Brother const& no) { return m_name == no.m_name; }
};
Q_DECLARE_METATYPE(Brother)

struct Ur
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(Mom mom MEMBER m_mama)
    Q_PROPERTY(Dad dad MEMBER m_papa)
    Q_PROPERTY(QList<Brother> bros MEMBER m_bros)
    Q_PROPERTY(long long fap_counter MEMBER m_counter)
public:
    QString m_name;
    Mom m_mama;
    Dad m_papa;
    QList<Brother> m_bros;
    long long m_counter;
};
Q_DECLARE_METATYPE(Ur)


 *
 * WOW!!! Such a big family of Ur! And now you gonna put them into this tiny SQLite3 database?
 * You sick freak... I'm in, let's do it!
 *
 * 1) If your structure have other structures as fields, (smart) pointers or container of structures,
 *      you have to add ROWID to your structure.
 *    You can either add 'long long orm_rowid' property or inherit the ORMValue structure.
 *

struct Dad
{
    Q_GADGET
    Q_PROPERTY(long long orm_rowid MEMBER m_own_rowid)              // <<<
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(Car * car MEMBER m_car)
public:
    long long m_own_rowid;                                          // <<<
    QString m_name;
    Car * m_car = nullptr; // lost somewhere
    bool operator !=(Dad const& no) { return m_name != no.m_name; }
};

    //        VVVVVVVVVVVVVVVVV
    struct Ur : public ORMValue // ...                              // <<<


 *
 * 2) Replace 'Q_DECLARE_METATYPE' with 'ORM_DECLARE_METATYPE'.
 *    ORM_DECLARE_METATYPE is actually Q_DECLARE_METATYPE inside, but it also generates
 *    metadata for pointers and containers.
 *

ORM_DECLARE_METATYPE(Mom)
ORM_DECLARE_METATYPE(Car)
ORM_DECLARE_METATYPE(Dad)
ORM_DECLARE_METATYPE(Brother)
ORM_DECLARE_METATYPE(Ur)

 *
 * 3) Finally replace 'qRegisterMetaType' with 'registerTypeORM'.
 *

    registerTypeORM<Ur>("Ur");
    registerTypeORM<Dad>("Dad");
    registerTypeORM<Mom>("Mom");
    registerTypeORM<Brother>("Brother");
    registerTypeORM<Car>("Car");

 *
 * 4) Done! Now go back to step 0 and take another coockie. You totally deserve it!
 *
 *   FAQ:
 * Q.: Is it free? What's the license?
 * A.: It's as free as love.
 *     But speaking in terms of law, let's say, MIT. You can also use it with any (l)GPL. Is it OK?
 *
 * Q.: What types are supported by your ORM?
 * A.: +  Any primitive type. At least I hope so.
 *     +  Any registered Q_ENUM/Q_GADGET/Q_OBJECT, QList\QVector of registered types, (smart)pointers to gadgets.
 *     +  Any type with valid T->QVariant->T conversion. Add it to primitive type list with ORM::addPrimitiveType
 *    +/- std containers are not supported but easy to add, goto 'orm_containers' namespace.
 *     -  Static arrays and pointers to arrays will be never supported.
 *     -  Associative containers and pairs are not supported. It is possible to add all necessary checkups, but
 *          I can't figure out how to turn it into table\value.
 *     -  classes without Qt meta.
 *
 * Q.: Your ORM can't do <X>.
 * A.: Well, maybe. So what?
 *
 * Q.: Will you add <X>?
 * A.: Don't think so.
 *
 * Q.: Is it as tiny as you claim?
 * A.: Yes and no.
 *     It is simple and easy to use, so, yes.
 *     It is filled with templates and generates tons of unnecessary data, so, no.
 *
 * Q.: I used it on a big ass file and ran out of entries in object file. What to do?
 * A.: Add this to .pro file and run qmake.
 *
        win32-msvc* {
            QMAKE_CXXFLAGS += /bigobj
        }

 *     GCC/MinGW/Clang? Good luck!
 *
 * Q.: I used it on a big ass file and ran out of heap memory.
 * A.: High five! Me too. Now, how about make your file smaller? By, let's see, hmm, multiplying them?
 *
 * Q.: Your code sucks and you should burn in hell.
 * A.: Q > /dev/null
 *
 */
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

    template <typename T>    void     create(             );
    template <typename T>    QList<T> select(             );
    template <typename T>    void     insert(      T  & t );
    template <typename T>    void     insert(QList<T> & tl);
    template <typename T>    void     delet (      T  & t );
    template <typename T>    void     delet (QList<T> & tl);
    template <typename T>    void     update(      T  & t );
    template <typename T>    void     update(QList<T> & tl);
    template <typename T>    void     drop  (             );

protected:
    QString m_databaseName;

protected:
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
    else {
        qDebug("Error casting");
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
    else {
        qDebug("Error casting");
    }
    return ret;
}
template <typename T>
void ORM::insert(T & t) {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariant v = QVariant::fromValue((void*)&t);
        meta_insert(*o, v);
    }
    else {
        qDebug("Error casting");
    }
}
template <typename T>
void ORM::insert(QList<T> & tl) {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (T & t : tl) {
            QVariant v = QVariant::fromValue((void*)&t);
            meta_insert(*o, v);
        }
    }
    else {
        qDebug("Error casting");
    }
}
template <typename T>
void ORM::delet(T & t) {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariant v = QVariant::fromValue((void*)&t);
        meta_delete(*o, v);
    }
    else {
        qDebug("Error casting");
    }
}
template <typename T>
void ORM::delet(QList<T> & tl) {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (T & t : tl) {
            QVariant v = QVariant::fromValue((void*)&t);
            meta_delete(*o, v);
        }
    }
    else {
        qDebug("Error casting");
    }
}
template <typename T>
void ORM::update(T & t) {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        QVariant v = QVariant::fromValue((void*)&t);
        meta_update(*o, v);
    }
    else {
        qDebug("Error casting");
    }
}
template <typename T>
void ORM::update(QList<T> & tl) {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (T & t : tl) {
            QVariant v = QVariant::fromValue((void*)&t);
            meta_update(*o, v);
        }
    }
    else {
        qDebug("Error casting");
    }
}
template <typename T>
void ORM::drop() {
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        meta_drop(*o);
    }
    else {
        qDebug("Error casting");
    }
}



namespace orm_pointers {

    struct ORMPointerStub {
        int T =0;
        int pT=0;
        int ST=0;
        int WT=0;
        int sT=0;
        int wT=0;
    };

    template <typename T>
    ORMPointerStub ORMPointerFunc()
    {
        ORMPointerStub stub;
        stub.T =  qMetaTypeId<                T >();
        stub.pT = qMetaTypeId<                T*>();
        stub.ST = qMetaTypeId<QSharedPointer <T>>();
        stub.WT = qMetaTypeId<QWeakPointer   <T>>();
        stub.sT = qMetaTypeId<std::shared_ptr<T>>();
        stub.wT = qMetaTypeId<std::weak_ptr  <T>>();
        return stub;
    }

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

    template <typename T>
    void registerTypePointers()
    {
        qRegisterMetaType<T*>();
        qRegisterMetaType<QSharedPointer <T>>();
        qRegisterMetaType<QWeakPointer   <T>>();
        qRegisterMetaType<std::shared_ptr<T>>();
        qRegisterMetaType<std::weak_ptr  <T>>();

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
void registerTypeORM(const char * c)
{
    if (c) {
        qRegisterMetaType<T >(c);
    }
    else {
        qRegisterMetaType<T>();
    }

    orm_pointers::registerTypePointers<T>();
    ORM::addPointerStub(orm_pointers::ORMPointerFunc<T>());
    orm_containers::registerTypeContainers<T>();
}

#endif // ORM_H
