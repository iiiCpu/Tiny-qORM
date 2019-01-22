/*********************************************
 *          made by iCpu with love           *
 * *******************************************/

#ifndef ORM_H
#define ORM_H

#include "orm_def.h"

#include <list>
#include <vector>
#include <QList>
#include <QVector>
#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>
#include <QPointer>
#include <memory>



ORM_DECLARE_OBJECT_ALL_SMARTPOINTERS(QObject)

class QSqlQuery;

namespace orm
{
    template <typename T>  int  Register  (const char * c = nullptr);   // T and T* for QGADGET, T* for QOBJECT
    template <typename T>  int  RegisterEx(const char * c = nullptr);   // Register<T>(c) + smartpointers

    namespace Pointers
    {
        struct PointerStub; // stores different pointer type IDs for same T. Add your field here for new pointer type.
        template <typename T> PointerStub registerTypePointers();        // T* for QGADGET, T* and QPointer<T> and QScopedPointer<T> for QObject
        template <typename T> PointerStub registerTypeSmartPointers();   // std::shared_ptr<T>, std::weak_ptr<T>, QSharedPointer<T>, QWeakPointer<T>
    }

    namespace Containers
    {
        template <typename T>                           void registerSequentialContainers();  // QList<T>, QVector<T>, std::list<T>, std::vector<T>
        template <typename K, typename T>               void registerQPair();                 // QPair<K,T>
        template <template <typename,typename>
                  class C, typename K, typename T>      void registerAssociative();           // C<K,T> usage registerAssociative<QMap,int,double>() for QMap<int, double>
        template <template <typename> class Container>  void registerPrimitiveTypeContainer();
    }

    class ORM
    {
        enum class Type { Type };
        enum class Object { Object };
    public:
        struct TableEntry
        {
            QMap <int, TableEntry> properties                ; // key is property index
            QString                 property_name            ; // = typename for root elements
            QString                 field_name               ; // if is_field = false than contains table name
            int                     orig_meta_type_id = 0    ;
            int                     norm_meta_type_id = 0    ; // T or T*
            int                     property_index    = 0    ;
            bool                    is_container      = false;
            bool                    is_primary_key    = false;
            bool                    is_orm_row_id     = false;
            bool                    is_field          = true ;
            bool                    is_QObject        = false;
            bool                    is_not_null       = false;
            bool                    is_unique         = false;
            bool                    is_autoincrement  = false;
            bool                    has_primary_key   = false;
            bool                    has_orm_row_id    = false;
        };
    public:
        QString m_databaseName;
        bool                                  m_bufferization; // false disables saving TableEntry
        QMap<int, QHash<QString, TableEntry>> m_types; // first key is metatype id, second key is parent name

        QHash<QString, QString>               m_createQueries; // key is table name
        QHash<QString, QString>               m_selectQueries; // key is table name
        QHash<QString, QString>               m_insertQueries; // key is table name
        QHash<QString, QString>               m_updateQueries; // key is table name
        QHash<QString, QString>               m_deleteQueries; // key is table name
        QHash<QString, QString>               m_dropQueries; // key is table name


    public:
        template <class T>                                                             void             create()                         {        ORM_Impl::create       <T>(this);        }
        template <class T> typename std::enable_if<!std::is_base_of<QObject,T>::value, QList<T >>::type select()                         { return ORM_Impl::selectGadget <T>(this);        }
        template <class T> typename std::enable_if< std::is_base_of<QObject,T>::value, QList<T*>>::type select()                         { return ORM_Impl::selectObject <T>(this);        }
        template <class T> typename std::enable_if<!std::is_base_of<QObject,T>::value, QList<T >>::type select(QVariantMap const& where) { return ORM_Impl::selectGadgetW<T>(this, where); }
        template <class T> typename std::enable_if< std::is_base_of<QObject,T>::value, QList<T*>>::type select(QVariantMap const& where) { return ORM_Impl::selectObjectW<T>(this, where); }
        template <class T> typename std::enable_if<!std::is_base_of<QObject,T>::value,       T  >::type get   (long long id)             { return ORM_Impl::getGadget    <T>(this, id);    }
        template <class T> typename std::enable_if< std::is_base_of<QObject,T>::value,       T* >::type get   (long long id)             { return ORM_Impl::getObject    <T>(this, id);    }
        template <class T>                                                             void             insert(T const& t)               {        ORM_Impl::insert       <T>(this, t);     }
        template <class T>                                                             void             insert(T * t)                    {        ORM_Impl::insert       <T>(this, t);     }
        template <class T>                                                             void             insert(QList<T> const& tl)       {        ORM_Impl::insert       <T>(this, tl);    }
        template <class T>                                                             void             delet (T const& t)               {        ORM_Impl::delet        <T>(this, t);     }
        template <class T>                                                             void             delet (T * t)                    {        ORM_Impl::delet        <T>(this, t);     }
        template <class T>                                                             void             delet (QList<T> const& tl)       {        ORM_Impl::delet        <T>(this, tl);    }
        template <class T>                                                             void             update(T const& t)               {        ORM_Impl::update       <T>(this, t);     }
        template <class T>                                                             void             update(T * t)                    {        ORM_Impl::update       <T>(this, t);     }
        template <class T>                                                             void             update(QList<T> const& tl)       {        ORM_Impl::update       <T>(this, tl);    }
        template <class T>                                                             void             drop  ()                         {        ORM_Impl::drop         <T>(this);        }

    public:
        ORM(QString const& dbname = QString());
        virtual ~ORM();

        QString databaseName() const;
        void setDatabaseName(const QString & databaseName);

        void     id_create       (int meta_type_id                           );
        QVariant id_select       (int meta_type_id                           );
        QVariant id_select_where (int meta_type_id, QVariantMap const& wheres);
        QVariant id_get          (int meta_type_id, long long              id);
        void     id_insert       (int meta_type_id, QVariant         &  value);
        void     id_delete       (int meta_type_id, QVariant         &  value);
        void     id_update       (int meta_type_id, QVariant         &  value);
        void     id_drop         (int meta_type_id                           );

//    protected:
        virtual long long get_last_inserted_rowid  (QSqlQuery & query)                                                                                                                                                 const =0;
        virtual long long get_changes              (QSqlQuery & query)                                                                                                                                                 const =0;
        virtual long long get_last_rowid           (QString table_name)                                                                                                                                                const =0;

        virtual QString normalize                  (QString const& str                  )                                                                                                                              const =0;
        virtual QString normalizeVar               (QString const& str, int meta_type_id)                                                                                                                              const =0;
        virtual QString sqlType                    (int type_id)                                                                                                                                                       const =0;

        virtual QString generate_table_name        (QString const& parent_name, QString const& property_name, QString const& class_name)                                                                               const =0;

        virtual QString generate_create_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const =0;
        virtual QString generate_insert_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const =0;
        virtual QString generate_select_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const =0;
        virtual QString generate_update_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const =0;
        virtual QString generate_delete_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const =0;
        virtual QString generate_drop_query        (ORM::TableEntry const& entry, ORM::TableEntry * parent) const =0;

    public: //protected:
        class ORM_Impl
        {
        public:
            static QString rowidName();
            static QString parentRowidName();
        public:
            static ORM::TableEntry queryData(ORM * orm, int meta_type_id);
            static QVariant read(bool isObject, ORM::TableEntry const& data, QVariant const& value);

            static void     meta_create(ORM * orm, TableEntry & entry,                   TableEntry * parent = nullptr                                        );
            static void     meta_insert(ORM * orm, TableEntry & entry, QVariant & value, TableEntry * parent = nullptr, QVariant const& parentVal = QVariant());
            static QVariant meta_select(ORM * orm, TableEntry & entry,                   TableEntry * parent = nullptr, QVariant const& parentVal = QVariant());
            static void     meta_update(ORM * orm, TableEntry & entry, QVariant & value, TableEntry * parent = nullptr, QVariant const& parentVal = QVariant());
            static void     meta_delete(ORM * orm, TableEntry & entry, QVariant & value, TableEntry * parent = nullptr, QVariant const& parentVal = QVariant());
            static void     meta_drop  (ORM * orm, TableEntry & entry,                   TableEntry * parent = nullptr                                        );

        public:
            template<class T>
            static void create           (ORM * orm, typename std::enable_if<!std::is_base_of<QObject,T>::value, Type>::type = Type::Type) {
                orm->id_create(qMetaTypeId<T>());
            }
            template<class T>
            static void create           (ORM * orm, typename std::enable_if<std::is_base_of<QObject,T>::value, Object>::type = Object::Object) {
                orm->id_create(qMetaTypeId<T*>());
            }



            template <typename T> static QList<T> selectGadget(ORM * orm)
            {
                QList<T> ret;
                QVariantList si = orm->id_select(qMetaTypeId<T>()).toList();
                for (QVariant v : si) {
                    ret << v.value<T>();
                }
                return ret;
            }
            template <typename T> static QList<T*> selectObject(ORM * orm)
            {
                QList<T*> ret;
                QVariantList si = orm->id_select(qMetaTypeId<T*>()).toList();
                for (QVariant v : si) {
                    T* t =  v.value<T*>();
                    if (t) {
                        ret << t;
                    }
                }
                return ret;
            }
            template <typename T> static QList<T> selectGadgetW(ORM * orm, QVariantMap const&  where) {
                QList<T> ret;
                QVariantList si = orm->id_select_where(qMetaTypeId<T>(), where).toList();
                for (QVariant v : si) {
                    ret << v.value<T>();
                }
                return ret;
            }
            template <typename T> static QList<T*> selectObjectW(ORM * orm, QVariantMap const&  where) {

                QList<T*> ret;
                QVariantList si = orm->id_select_where(qMetaTypeId<T*>(), where).toList();
                for (QVariant v : si) {
                    T* t =  v.value<T*>();
                    if (t) {
                        ret << t;
                    }
                }
                return ret;
            }

            template <typename T> static T getGadget(ORM * orm, long long id) {
                QVariantList si = orm->id_get(qMetaTypeId<T>(), id).toList();
                if (si.size()) {
                    return si.first().value<T>();
                }
                return T();
            }
            template <typename T> static T* getObject(ORM * orm, long long id)
            {
                QVariantList si = orm->id_get(qMetaTypeId<T*>(), id).toList();
                if (si.size()) {
                    return si.first().value<T*>();
                }
                return nullptr;
            }

            template <typename T> static void insert(ORM * orm, T const& t, typename std::enable_if<!std::is_base_of<QObject,T>::value, Type>::type = Type::Type) {
                QVariant v = QVariant::fromValue(t);
                orm->id_insert(v.userType(), v);
            }
            template <typename T> static void insert(ORM * orm, T const& t, typename std::enable_if<std::is_base_of<QObject,T>::value, Object>::type = Object::Object) {
                QVariant v = QVariant::fromValue(const_cast<T*>(&t));
                orm->id_insert(v.userType(), v);
            }
            template <typename T> static void insert(ORM * orm, T * t) {
                if (t) {
                    QVariant v = QVariant::fromValue(t);
                    orm->id_insert(v.userType(), v);
                }
            }
            template <typename T> static void insert(ORM * orm, QList<T> const& tl, typename std::enable_if<!std::is_base_of<QObject,T>::value, Type>::type = Type::Type) {
                for (T const& t : tl) {
                    QVariant v = QVariant::fromValue(t);
                    orm->id_insert(v.userType(), v);
                }
            }
            template <typename T> static void insert(ORM * orm, QList<T> const& tl, typename std::enable_if<std::is_base_of<QObject,T>::value, Object>::type = Object::Object) {
                for (T const& t : tl) {
                    QVariant v = QVariant::fromValue(const_cast<T*>(&t));
                    orm->id_insert(v.userType(), v);
                }
            }
            template <typename T> static void delet(ORM * orm, T const& t, typename std::enable_if<!std::is_base_of<QObject,T>::value, Type>::type = Type::Type) {
                QVariant v = QVariant::fromValue(t);
                orm->id_delete(v.userType(), v);
            }
            template <typename T> static void delet(ORM * orm, T const& t, typename std::enable_if<std::is_base_of<QObject,T>::value, Object>::type = Object::Object) {
                QVariant v = QVariant::fromValue(const_cast<T*>(&t));
                orm->id_delete(v.userType(), v);
            }
            template <typename T> static void delet(ORM * orm, T * t) {
                if (t) {
                    QVariant v = QVariant::fromValue(t);
                    orm->id_delete(v.userType(), v);
                }
            }
            template <typename T> static void delet(ORM * orm, QList<T> const& tl, typename std::enable_if<!std::is_base_of<QObject,T>::value, Type>::type = Type::Type) {
                for (T const& t : tl) {
                    QVariant v = QVariant::fromValue(t);
                    orm->id_delete(v.userType(), v);
                }
            }
            template <typename T> static void delet(ORM * orm, QList<T> const& tl, typename std::enable_if<std::is_base_of<QObject,T>::value, Object>::type = Object::Object) {
                for (T const& t : tl) {
                    QVariant v = QVariant::fromValue(const_cast<T*>(&t));
                    orm->id_delete(v.userType(), v);
                }
            }
            template <typename T> static void update(ORM * orm, T const& t, typename std::enable_if<!std::is_base_of<QObject,T>::value, Type>::type = Type::Type) {
                QVariant v = QVariant::fromValue(t);
                orm->id_update(v.userType(), v);
            }
            template <typename T> static void update(ORM * orm, T const& t, typename std::enable_if<std::is_base_of<QObject,T>::value, Object>::type = Object::Object) {
                QVariant v = QVariant::fromValue(const_cast<T*>(&t));
                orm->id_update(v.userType(), v);
            }
            template <typename T> static void update(ORM * orm, T * t) {
                if (t) {
                    QVariant v = QVariant::fromValue(t);
                    orm->id_update(v.userType(), v);
                }
            }
            template <typename T> static void update(ORM * orm, QList<T> const& tl, typename std::enable_if<!std::is_base_of<QObject,T>::value, Type>::type = Type::Type) {
                for (T const& t : tl) {
                    QVariant v = QVariant::fromValue(t);
                    orm->id_update(v.userType(), v);
                }
            }
            template <typename T> static void update(ORM * orm, QList<T> const& tl, typename std::enable_if<std::is_base_of<QObject,T>::value, Object>::type = Object::Object) {
                for (T const& t : tl) {
                    QVariant v = QVariant::fromValue(const_cast<T*>(&t));
                    orm->id_update(v.userType(), v);
                }
            }
            template<class T>
            static void drop           (ORM * orm, typename std::enable_if<!std::is_base_of<QObject,T>::value, Type>::type = Type::Type) {
                orm->id_drop(qMetaTypeId<T>());
            }
            template<class T>
            static void drop           (ORM * orm, typename std::enable_if<std::is_base_of<QObject,T>::value, Object>::type = Object::Object) {
                orm->id_drop(qMetaTypeId<T*>());
            }
        protected:
            void meta_insert2(ORM * orm, int meta_type_id, QVariant & value, const QString & parent_name, const QString & property_name, long long parent_orm_rowid);
        };
        friend class ORM_Impl;
    };

    struct Config
    {

        template <typename T> static void addIgnoredType           () {        Config_Impl::addIgnoredType    <T>(); }
        template <typename T> static bool isIgnoredType            () { return Config_Impl::isIgnoredType     <T>(); }
        template <typename T> static void removeIgnoredType        () {        Config_Impl::removeIgnoredType <T>(); }

        template <typename T> static void addPrimitiveType         () {        addPrimitiveType         (qMetaTypeId<T>()); }
        template <typename T> static bool isPrimitiveType          () { return isPrimitiveType          (qMetaTypeId<T>()); }
        template <typename T> static void removePrimitiveType      () {        removePrimitiveType      (qMetaTypeId<T>()); }

        template <typename T> static void addPrimitiveStringType   () {        addPrimitiveStringType   (qMetaTypeId<T>()); }
        template <typename T> static bool isPrimitiveStringType    () { return isPrimitiveStringType    (qMetaTypeId<T>()); }
        template <typename T> static void removePrimitiveStringType() {        removePrimitiveStringType(qMetaTypeId<T>()); }

        template <typename T> static void addPrimitiveRawType      () {        addPrimitiveRawType      (qMetaTypeId<T>()); }
        template <typename T> static bool isPrimitiveRawType       () { return isPrimitiveRawType       (qMetaTypeId<T>()); }
        template <typename T> static void removePrimitiveRawType   () {        removePrimitiveRawType   (qMetaTypeId<T>()); }

        // Type is ignored by ORM
        static void addIgnoredType   (int meta_type_id);
        static bool isIgnoredType    (int meta_type_id);
        static void removeIgnoredType(int meta_type_id);

        // Type is
        static void addPrimitiveType   (int meta_type_id);
        static bool isPrimitiveType    (int meta_type_id);
        static void removePrimitiveType(int meta_type_id);

        static void addPrimitiveStringType   (int meta_type_id);
        static bool isPrimitiveStringType    (int meta_type_id);
        static void removePrimitiveStringType(int meta_type_id);

        static void addPrimitiveRawType   (int meta_type_id);
        static bool isPrimitiveRawType    (int meta_type_id);
        static void removePrimitiveRawType(int meta_type_id);

        static void addPointerStub(orm::Pointers::PointerStub const&);
        static orm::Pointers::PointerStub getPointerStub(int metaTypeID);

        static void addPairType         (int firstTypeID, int secondTypeID, int pairTypeID);
        static void addContainerPairType(int firstTypeID, int secondTypeID, int pairTypeID);
        static void addSeqContainerType (int seqTypeID, int innerTypeID);



    protected:
        struct Config_Impl
        {
            template <typename T> static typename std::enable_if< QMetaTypeId2<T>::Defined, void>::type addIgnoredType   () {        orm::Config::addIgnoredType   (qMetaTypeId<T>()); }
            template <typename T> static typename std::enable_if< QMetaTypeId2<T>::Defined, bool>::type isIgnoredType    () { return orm::Config::isIgnoredType    (qMetaTypeId<T>()); }
            template <typename T> static typename std::enable_if< QMetaTypeId2<T>::Defined, void>::type removeIgnoredType() {        orm::Config::removeIgnoredType(qMetaTypeId<T>()); }
            template <typename T> static typename std::enable_if<!QMetaTypeId2<T>::Defined, void>::type addIgnoredType   () {                                                          }
            template <typename T> static typename std::enable_if<!QMetaTypeId2<T>::Defined, bool>::type isIgnoredType    () { return true;                                             }
            template <typename T> static typename std::enable_if<!QMetaTypeId2<T>::Defined, void>::type removeIgnoredType() {                                                          }

        };

        //    template <typename T>
        //    static QVariant makeVariant(T & t) {
        //        return makeVariant(&t);
        //    }
        //    template <template <typename> class Container, typename T>
        //    static QVariant makeVariant(Container<T> & tl) {
        //        QVariantList vlist;
        //        for (auto const& t : tl) {
        //            vlist << makeVariant(t);
        //        }
        //        return vlist;
        //    }
        //    template <typename T>
        //    static QVariant makeVariant(QPointer<T> & t) {
        //        return makeVariant(t.data());
        //    }
        //    template <typename T>
        //    static QVariant makeVariant(QSharedPointer<T> & t) {
        //        return makeVariant(t.data());
        //    }
        //    template <typename T>
        //    static QVariant makeVariant(QScopedPointer<T> & t) {
        //        return makeVariant(t.data());
        //    }
        //    template <typename T>
        //    static QVariant makeVariant(QWeakPointer<T> & t) {
        //        return makeVariant(t.data());
        //    }
        //    template <typename T>
        //    static QVariant makeVariant(std::unique_ptr<T> & t) {
        //        return makeVariant(t.get());
        //    }
        //    template <typename T>
        //    static QVariant makeVariant(std::shared_ptr<T> & t) {
        //        return makeVariant(t.get());
        //    }
        //    template <typename T>
        //    static QVariant makeVariant(std::weak_ptr<T> & t) {
        //        return makeVariant(t.lock());
        //    }
        //    template <typename T>
        //    static QVariant makeVariant(T * t) {
        //        if (t == nullptr) {
        //            return QVariant();
        //        }
        //        return QVariant::fromValue(t);
        //    }
    };



    namespace Pointers
    {

        struct PointerStub {
            int T =0;   // T
            int pT=0;   // T*
            int PT=0;   // QPointer<T>
            int ST=0;   // QSharedPointer<T>
            int WT=0;   // QWeakPointer<T>
            int sT=0;   // std::shared_ptr<T>
            int wT=0;   // std::weak_ptr<T>
        };
        struct NewStub { };
        struct VoidPointer { void* data = nullptr; VoidPointer():data(nullptr){} VoidPointer(void*p):data(p){} };

        namespace Pointers_Impl
        {

            template <typename T>                 T* toPointer (T const& t){                 T* p = new T ; *p = t; return p; }
            template <typename T>        QPointer<T> toPointerP(T const& t){                 T* p = new T ; *p = t; return p; }
            template <typename T> QSharedPointer <T> toPointerS(T const& t){ QSharedPointer <T> p  (new T); *p = t; return p; }
            template <typename T> std::shared_ptr<T> toPointers(T const& t){ std::shared_ptr<T> p  (new T); *p = t; return p; }
            template <typename T> QSharedPointer <T> fromPointerS(         T* t){ return QSharedPointer <T>                     (t) ; }
            template <typename T> std::shared_ptr<T> fromPointers(         T* t){ return std::shared_ptr<T>                     (t) ; }

            template <typename T>                 T* fromNewStub (NewStub const&){                 T* p = new T ; return p; }
            template <typename T>        QPointer<T> fromNewStubP(NewStub const&){        QPointer<T> p  (new T); return p; }
            template <typename T> QSharedPointer <T> fromNewStubS(NewStub const&){ QSharedPointer <T> p  (new T); return p; }
            template <typename T> std::shared_ptr<T> fromNewStubs(NewStub const&){ std::shared_ptr<T> p  (new T); return p; }


            template <typename T> VoidPointer toVoidPointerT(                T  const& t){ return VoidPointer( reinterpret_cast<void*>(const_cast<T*>(&t       )) ); }
            template <typename T> VoidPointer toVoidPointer (                T *       t){ return VoidPointer( reinterpret_cast<void*>(                t        ) ); }
            template <typename T> VoidPointer toVoidPointerP(      QPointer <T>        t){ return VoidPointer( reinterpret_cast<void*>(                t.data() ) ); }
            template <typename T> VoidPointer toVoidPointerS(QSharedPointer <T> const& t){ return VoidPointer( reinterpret_cast<void*>(const_cast<T*>( t.data())) ); }
            template <typename T> VoidPointer toVoidPointerW(  QWeakPointer <T> const& t){ return VoidPointer( reinterpret_cast<void*>(const_cast<T*>( t.data())) ); }
            template <typename T> VoidPointer toVoidPointers(std::shared_ptr<T> const& t){ return VoidPointer( reinterpret_cast<void*>(const_cast<T*>( t.get ())) ); }
            template <typename T> VoidPointer toVoidPointerw(  std::weak_ptr<T> const& t){ return VoidPointer( reinterpret_cast<void*>(const_cast<T*>( t.lock().get())) ); }

            template <typename T>                 T  fromVoidPointerT   (VoidPointer const& t){ T Tt = *reinterpret_cast<T*>(t.data) ; return Tt; }
            template <typename T>                 T* fromVoidPointer    (VoidPointer const& t){ return                    reinterpret_cast<T*>(t.data) ; }
            template <typename T>       QPointer <T> fromVoidPointerP   (VoidPointer const& t){ return       QPointer <T>(reinterpret_cast<T*>(t.data)); }
            template <typename T> QSharedPointer <T> fromVoidPointerS   (VoidPointer const& t){ return QSharedPointer <T>(reinterpret_cast<T*>(t.data)); }
            template <typename T> std::shared_ptr<T> fromVoidPointers   (VoidPointer const& t){ return std::shared_ptr<T>(reinterpret_cast<T*>(t.data)); }

            template <typename T> typename std::enable_if<!std::is_base_of<QObject,T>::value, PointerStub>::type registerTypePointers()
            {
                PointerStub stub = Config::getPointerStub(qMetaTypeId<T>());
                if (!stub.T) {
                    stub.T = qMetaTypeId<T>();
                }
                if (!stub.pT) {
                    stub.pT = qMetaTypeId<T*>() ? qMetaTypeId<T*>() : qRegisterMetaType<T*>();
                    //QMetaType::registerConverter< T , VoidPointer >(&  toVoidPointerT<T>);
                    QMetaType::registerConverter< T*, VoidPointer >(&  toVoidPointer <T>);
                    //QMetaType::registerConverter< VoidPointer, T  >(&fromVoidPointerT<T>);
                    QMetaType::registerConverter< VoidPointer, T* >(&fromVoidPointer <T>);
                    QMetaType::registerConverter< NewStub, T* >(&fromNewStub <T>);
                }
                return stub;
            }
            template <typename T> typename std::enable_if< std::is_base_of<QObject,T>::value, PointerStub>::type registerTypePointers()
            {
                PointerStub stub = Config::getPointerStub(qMetaTypeId<T*>());
                if (!stub.pT) {
                    stub.pT = qMetaTypeId<T*>();
                    QMetaType::registerConverter< T*, VoidPointer >(&  toVoidPointer <T>);
                    QMetaType::registerConverter< VoidPointer, T* >(&fromVoidPointer <T>);
                    QMetaType::registerConverter< NewStub, T* >(&fromNewStub <T>);
                }
                if (!stub.PT) {
                    stub.PT = qMetaTypeId< QPointer<T> >() ? qMetaTypeId<QPointer<T>>() : qRegisterMetaType< QPointer<T> >();
                    QMetaType::registerConverter< QPointer<T>, VoidPointer >(&  toVoidPointerP<T>);
                    QMetaType::registerConverter< VoidPointer, QPointer<T> >(&fromVoidPointerP<T>);
                    QMetaType::registerConverter< NewStub, QPointer<T> >(&fromNewStubP<T>);
                }
                return stub;
            }
        }

        template <typename T> PointerStub registerTypePointers()
        {
            return Pointers_Impl::registerTypePointers<T>();
        }

        template <typename T> PointerStub registerTypeSmartPointers()
        {
            PointerStub stub = Config::getPointerStub(qMetaTypeId<T*>());
            if (!stub.pT) {
                stub.pT = qMetaTypeId<T*>() ? qMetaTypeId<T*>() : qRegisterMetaType<T*>();
            }
            if (!stub.ST) {
                stub.ST = qMetaTypeId< QSharedPointer<T> >() ? qMetaTypeId< QSharedPointer<T> >() : qRegisterMetaType< QSharedPointer<T> >();
                QMetaType::registerConverter<  VoidPointer, QSharedPointer<T> >(&Pointers_Impl::fromVoidPointerS<T>);
                QMetaType::registerConverter<  QSharedPointer<T>, VoidPointer >(&Pointers_Impl::  toVoidPointerS<T>);
            }
            if (!stub.WT) {
                stub.WT = qMetaTypeId< QWeakPointer<T> >() ? qMetaTypeId< QWeakPointer<T> >() : qRegisterMetaType< QWeakPointer<T> >();
                QMetaType::registerConverter<    QWeakPointer<T>, VoidPointer >(&Pointers_Impl::  toVoidPointerW<T>);
            }
            if (!stub.sT) {
                stub.sT = qMetaTypeId< std::shared_ptr<T> >() ? qMetaTypeId< std::shared_ptr<T> >() : qRegisterMetaType< std::shared_ptr<T> >();
                QMetaType::registerConverter< VoidPointer, std::shared_ptr<T> >(&Pointers_Impl::fromVoidPointers<T>);
                QMetaType::registerConverter< std::shared_ptr<T>, VoidPointer >(&Pointers_Impl::  toVoidPointers<T>);
            }
            if (!stub.wT) {
                stub.wT = qMetaTypeId< std::weak_ptr<T> >() ? qMetaTypeId< std::weak_ptr<T> >() : qRegisterMetaType< std::weak_ptr<T> >();
                QMetaType::registerConverter<   std::weak_ptr<T>, VoidPointer >(&Pointers_Impl::  toVoidPointerw<T>);
            }

            return stub;
        }
    }

    namespace Containers
    {
        template <typename T> void registerSequentialContainers(); // QList<T>/QVector<T>/std::list<T>/std::vector<T>
        template <typename K, typename T> void registerQPair();    // QPair<K,T>
        template <typename K, typename T, template <typename,typename> class C> void registerAssociative();
        template<template <typename> class Container> void registerPrimitiveTypeContainer();


        struct ORM_QVariantPair //: public ORMValue
        {
            Q_GADGET
            Q_PROPERTY(QVariant key MEMBER key)
            Q_PROPERTY(QVariant value MEMBER value)
            Q_PROPERTY(QVariant orm_rowid MEMBER rowid)
        public:
            QVariant key, value, rowid;
            QVariant& operator[](int index){ return index == 0 ? key : index == 1 ? value : rowid; }
        };

        namespace Containers_Impl
        {
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

            template <typename K, typename T, template <typename,typename> class C> C<K,T> qAssociativeFromQVariantList(QVariantList const& v)
            {
                C<K,T> list;
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
            template <typename K, typename T, template <typename,typename> class C> C<K,T> qAssociativeFromQVariantAssociative(QVariant const& v)
            {
                C<K,T> list;
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

            template <typename K, typename T> ORM_QVariantPair toQPairStub(QPair<K,T> const& v)
            {
                ORM_QVariantPair ps;
                ps.key = QVariant::fromValue(v.first);
                ps.value = QVariant::fromValue(v.second);
                return ps;
            }

            template <typename K, typename T, template <typename,typename> class C> QList<ORM_QVariantPair> qAssociativeToPairListStub(C<K,T> const& v)
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

            template <typename K, typename T, template <typename,typename> class C> C<K,T> qAssociativeFromPairListStub(QList<ORM_QVariantPair> const& v)
            {
                C<K,T> map;
                for (auto i = v.begin(); i != v.end(); ++i) {
                    if (i->key.canConvert<K>() && i->value.canConvert<T>()) {
                        map.insertMulti(i->key.value<K>(), i->value.value<T>());
                    }
                }
                return map;
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


        }



        template <typename T> void registerSequentialContainers()
        {
            qMetaTypeId<QList      <T>>() ? qMetaTypeId<QList      <T>>() : qRegisterMetaType<QList      <T>>();
            qMetaTypeId<QVector    <T>>() ? qMetaTypeId<QVector    <T>>() : qRegisterMetaType<QVector    <T>>();
            qMetaTypeId<std::list  <T>>() ? qMetaTypeId<std::list  <T>>() : qRegisterMetaType<std::list  <T>>();
            qMetaTypeId<std::vector<T>>() ? qMetaTypeId<std::vector<T>>() : qRegisterMetaType<std::vector<T>>();
            Config::addSeqContainerType(qMetaTypeId<QList      <T>>(),qMetaTypeId<T>());
            Config::addSeqContainerType(qMetaTypeId<QVector    <T>>(),qMetaTypeId<T>());
            Config::addSeqContainerType(qMetaTypeId<std::list  <T>>(),qMetaTypeId<T>());
            Config::addSeqContainerType(qMetaTypeId<std::vector<T>>(),qMetaTypeId<T>());
            QMetaType::registerConverter<QVariantList, QList      <T>>(&(Containers_Impl::    qListFromQVariantList<T>));
            QMetaType::registerConverter<QVariantList, QVector    <T>>(&(Containers_Impl::  qVectorFromQVariantList<T>));
            QMetaType::registerConverter<QVariantList, std::list  <T>>(&(Containers_Impl::  stdListFromQVariantList<T>));
            QMetaType::registerConverter<QVariantList, std::vector<T>>(&(Containers_Impl::stdVectorFromQVariantList<T>));
        }


        template <typename K, typename T> void registerQPair()
        {
            Config::addPairType(qMetaTypeId<K>(), qMetaTypeId<T>(),
                                qMetaTypeId<QPair <K,T>>() ? qMetaTypeId<QPair <K,T>>() : qRegisterMetaType<QPair <K,T>>());
            QMetaType::registerConverter<QVariant,         QPair<K,T>>(&(Containers_Impl::qPairFromQVariant    <K,T>));
            QMetaType::registerConverter<QVariantList,     QPair<K,T>>(&(Containers_Impl::qPairFromQVariantList<K,T>));
            QMetaType::registerConverter<ORM_QVariantPair, QPair<K,T>>(&(Containers_Impl::qPairFromPairStub    <K,T>));
            QMetaType::registerConverter<QPair<K,T>, ORM_QVariantPair>(&(Containers_Impl::toQPairStub          <K,T>));
        }
        template <template <typename,typename> class C, typename K, typename T> void registerAssociative()
        {
            registerQPair<K,T>();

            Config::addContainerPairType(qMetaTypeId<K>(), qMetaTypeId<T>(),
                                         qMetaTypeId<C<K,T>>() ? qMetaTypeId<C<K,T>>() : qRegisterMetaType<C<K,T>>());
            QMetaType::registerConverter<C<K,T>,  QList<ORM_QVariantPair>>(&(Containers_Impl::qAssociativeToPairListStub          <K,T,C>));
            QMetaType::registerConverter<QVariantMap             , C<K,T>>(&(Containers_Impl::qAssociativeFromQVariantAssociative <K,T,C>));
            QMetaType::registerConverter<QVariantList            , C<K,T>>(&(Containers_Impl::qAssociativeFromQVariantList        <K,T,C>));
            QMetaType::registerConverter<QList <ORM_QVariantPair>, C<K,T>>(&(Containers_Impl::qAssociativeFromPairListStub        <K,T,C>));
        }

        template<template <typename> class Container> void registerPrimitiveTypeContainer() {
            Config::addPrimitiveType(qRegisterMetaType<Container<         bool       >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<unsigned char       >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<  signed char       >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<         char       >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<unsigned short      >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<  signed short      >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<         short      >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<unsigned int        >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<  signed int        >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<         int        >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<unsigned long long  >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<  signed long long  >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<         long long  >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<         float      >>());
            Config::addPrimitiveType(qRegisterMetaType<Container<         double     >>());

            QMetaType::registerConverter< Container<         bool       >, QString >(&(Containers_Impl::primitiveToString<          bool       , Container>));
            QMetaType::registerConverter< Container<unsigned char       >, QString >(&(Containers_Impl::primitiveToString< unsigned char       , Container>));
            QMetaType::registerConverter< Container<  signed char       >, QString >(&(Containers_Impl::primitiveToString<   signed char       , Container>));
            QMetaType::registerConverter< Container<unsigned short      >, QString >(&(Containers_Impl::primitiveToString< unsigned short      , Container>));
            QMetaType::registerConverter< Container<  signed short      >, QString >(&(Containers_Impl::primitiveToString<   signed short      , Container>));
            QMetaType::registerConverter< Container<unsigned int        >, QString >(&(Containers_Impl::primitiveToString< unsigned int        , Container>));
            QMetaType::registerConverter< Container<  signed int        >, QString >(&(Containers_Impl::primitiveToString<   signed int        , Container>));
            QMetaType::registerConverter< Container<unsigned long long  >, QString >(&(Containers_Impl::primitiveToString< unsigned long long  , Container>));
            QMetaType::registerConverter< Container<  signed long long  >, QString >(&(Containers_Impl::primitiveToString<   signed long long  , Container>));
            QMetaType::registerConverter< Container<         float      >, QString >(&(Containers_Impl::primitiveToString<          float      , Container>));
            QMetaType::registerConverter< Container<         double     >, QString >(&(Containers_Impl::primitiveToString<          double     , Container>));

            QMetaType::registerConverter<QString, Container<         bool       >>(&(Containers_Impl::stringToPrimitive<          bool       , Container>));
            QMetaType::registerConverter<QString, Container<unsigned char       >>(&(Containers_Impl::stringToPrimitive< unsigned char       , Container>));
            QMetaType::registerConverter<QString, Container<  signed char       >>(&(Containers_Impl::stringToPrimitive<   signed char       , Container>));
            QMetaType::registerConverter<QString, Container<unsigned short      >>(&(Containers_Impl::stringToPrimitive< unsigned short      , Container>));
            QMetaType::registerConverter<QString, Container<  signed short      >>(&(Containers_Impl::stringToPrimitive<   signed short      , Container>));
            QMetaType::registerConverter<QString, Container<unsigned int        >>(&(Containers_Impl::stringToPrimitive< unsigned int        , Container>));
            QMetaType::registerConverter<QString, Container<  signed int        >>(&(Containers_Impl::stringToPrimitive<   signed int        , Container>));
            QMetaType::registerConverter<QString, Container<unsigned long long  >>(&(Containers_Impl::stringToPrimitive< unsigned long long  , Container>));
            QMetaType::registerConverter<QString, Container<  signed long long  >>(&(Containers_Impl::stringToPrimitive<   signed long long  , Container>));
            QMetaType::registerConverter<QString, Container<         float      >>(&(Containers_Impl::stringToPrimitive<          float      , Container>));
            QMetaType::registerConverter<QString, Container<         double     >>(&(Containers_Impl::stringToPrimitive<          double     , Container>));
        }
    }

    namespace Impl
    {
        template <typename T> static typename std::enable_if<!std::is_base_of<QObject,T>::value, int>::type Register(const char * c)
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
            Config::addPointerStub(orm::Pointers::registerTypePointers<T>());
            orm::Containers::registerSequentialContainers<T>();
            return type;
        }
        template <typename T> static typename std::enable_if< std::is_base_of<QObject,T>::value, int>::type Register(const char *)
        {
            Q_ASSERT(T::staticMetaObject.constructorCount());
            Config::addPointerStub(orm::Pointers::registerTypePointers<T>());
            return qMetaTypeId<QObject*>();
        }
        template <typename T> static typename std::enable_if<!std::is_base_of<QObject,T>::value, int>::type RegisterEx(const char * c)
        {
            int type = Register<T>(c);
            Config::addPointerStub(orm::Pointers::registerTypeSmartPointers<T>());
            return type;
        }
        template <typename T> static typename std::enable_if< std::is_base_of<QObject,T>::value, int>::type RegisterEx(const char *)
        {
            Register<T>(nullptr);
            Config::addPointerStub(orm::Pointers::registerTypeSmartPointers<T>());
            return qMetaTypeId<QObject*>();
        }
        void     registerPrimitiveTypeContainers();




        bool       withRowid     (const QMetaObject &meta);
        bool       isPrimaryKey  (const QMetaObject & obj, int property);
        bool       isRowID       (const QString & property);

        bool       isEnumeration            (int meta_type_id);
        bool       isGadget                 (int meta_type_id);
        bool       isQObject                (int meta_type_id);
        bool       isQObject                (QMetaObject const& meta);
        bool       isPointer                (int meta_type_id);
        bool       isWeakPointer            (int meta_type_id);
        bool       isIgnored                (int meta_type_id);
        bool       isPrimitive              (int meta_type_id);
        bool       isPrimitiveType          (int meta_type_id);
        bool       isPrimitiveString        (int meta_type_id);
        bool       isPrimitiveRaw           (int meta_type_id);
        bool       isSequentialContainer    (int meta_type_id);
        bool       isPair                   (int meta_type_id);
        bool       isAssociativeContainer   (int meta_type_id);
        bool       isTypeForTable           (int meta_type_id);

        bool       shouldSkipMeta           (QMetaProperty const& property, QObject const* object = nullptr);
        bool       shouldSkipReadMeta       (QMetaProperty const& property, QObject const* object = nullptr);
        bool       shouldSkipWriteMeta      (QMetaProperty const& property, QObject const* object = nullptr);

        int        typeToValueType                       (int meta_type_id); // T* -> T
        int        getSequentialContainerStoredType      (int meta_type_id); // std::list<T> -> T
        int        getAssociativeContainerStoredKeyType  (int meta_type_id); // std::map<K,T> -> K
        int        getAssociativeContainerStoredValueType(int meta_type_id); // std::map<K,T> -> T
        int        getAssociativeContainerStoredType     (int meta_type_id, int property); // property: 0 - key, 1 - value, 2 - orm_rowid

        int        actualMetaTypeIDOnce                  (int meta_type_id);
        int        actualMetaTypeID                      (int meta_type_id); // QSharedPointer<T>* -> T or T*
        QList<int> actualMetaTypeIDQueue                 (int meta_type_id);

        QMetaObject const* meta_object(int meta_type_id);

        bool       write            (int meta_type_id, int property_index, QVariant      & writeInto, QVariant const& value);
        QVariant   read             (int meta_type_id, int property_index, QVariant const&  readFrom);
        QVariant   makeStoreQVariant(int meta_type_id);

        template <typename Foo>
        void for_each_property(int meta_type_id, Foo foo) {
            if (isSequentialContainer(meta_type_id)) {
                foo(getSequentialContainerStoredType(meta_type_id));
                return;
            }
            if (isAssociativeContainer(meta_type_id) || isPair(meta_type_id)) {
                foo(getAssociativeContainerStoredKeyType(meta_type_id));
                foo(getAssociativeContainerStoredValueType(meta_type_id));
                return;
            }
            foo(meta_type_id);
        }
        template <typename Foo>
        void for_each(int meta_type_id, QVariant & value, Foo foo) {
            if (isSequentialContainer(meta_type_id)) {
                if (value.isValid()) {
                    QSequentialIterable sequentialIterable = value.value<QSequentialIterable>();
                    int index = 0;
                    for (QVariant listElement : sequentialIterable) {
                        foo(getSequentialContainerStoredType(meta_type_id), listElement, index++);
                    }
                }
                return;
            }
            if (isAssociativeContainer(meta_type_id) || isPair(meta_type_id)) {
                if (value.canConvert<orm::Containers::ORM_QVariantPair>()) {
                    QVariant var = value;
                    var.convert(qMetaTypeId<orm::Containers::ORM_QVariantPair>());
                    foo(meta_type_id, var, 0);
                    return;
                }
                if (value.canConvert<QList<orm::Containers::ORM_QVariantPair>>()) {
                    QList<orm::Containers::ORM_QVariantPair> pairList = value.value<QList<orm::Containers::ORM_QVariantPair>>();
                    int index = 0;
                    for (orm::Containers::ORM_QVariantPair & ormPairValue : pairList) {
                        QVariant variantPair = QVariant::fromValue(ormPairValue);
                        foo(meta_type_id, variantPair, index++);
                    }
                    return;
                }
            }
            if (isPointer(meta_type_id)) {
                if (value.value<orm::Pointers::VoidPointer>().data == nullptr) {
                    return;
                }
            }
            foo(meta_type_id, value, 0);
        }

    }



    template <typename T>  int  Register  (const char * c)
    {
        return Impl::Register<T>(c);
    }
    template <typename T>  int  RegisterEx  (const char * c)
    {
        return Impl::RegisterEx<T>(c);
    }
}
class QDebug;
QDebug operator<< (QDebug debug, orm::ORM::TableEntry const& te);
QDebug operator<< (QDebug dbg, orm::Config const& config);
QDebug operator<< (QDebug dbg, orm::Containers::ORM_QVariantPair const& pair);
QDebug operator<< (QDebug dbg, QList<orm::Containers::ORM_QVariantPair> const& pair);


uint qHash(orm::Containers::ORM_QVariantPair const& variantPair) Q_DECL_NOEXCEPT;
Q_DECLARE_METATYPE(orm::Containers::ORM_QVariantPair)
Q_DECLARE_METATYPE(orm::Pointers::NewStub)
Q_DECLARE_METATYPE(orm::Pointers::VoidPointer)

#endif // ORM_H
