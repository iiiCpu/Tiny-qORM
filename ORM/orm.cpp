/*********************************************
 *          made by iCpu with love           *
 * *******************************************/

#include "orm.h"

#include <QDebug>
#include <QDataStream>
#include <QMetaProperty>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QRegularExpression>
#include <QCoreApplication>

QDebug operator << (QDebug debug, orm::ORM::TableEntry const& te)
{
    debug << endl
          << te.orig_meta_type_id << "(" << te.norm_meta_type_id << ") " << te.field_name
          << te.property_name << " (" << te.property_index
          << ") isField=" << te.is_field << " isObject=" << te.is_QObject
          << te.is_primary_key << "  " << te.properties.size() << " properties {";
    for (auto const&i : te.properties) {
        debug << i << endl;
    }
    debug << "}";
    return debug;
}

namespace orm
{

    namespace Impl
    {
        static QSet<int>                               pairTypes                                              ;
        static QMap<int, int>                          seqContTypeMap                                         ;
        static QMap<int, QPair<int, int>>              assContTypeMap                                         ;
        static QMap<int, orm::Pointers::PointerStub>   pointerMap                                             ;
        static QSet<int>                               ignoredTypes                                           ;
        static QSet<int>                               primitiveContainers                                    ;
        static QSet<int>                               primitiveStringContainers                              ;
        static QSet<int>                               primitiveRawContainers                                 ;

        static QString                                 orm_rowidName             = QString("orm_rowid")       ;
        static QString                                 orm_parentRowidName       = QString("parent_orm_rowid");
        static bool                                    orm_once                  = true                       ;


        inline bool isEnumeration(int meta_type_id)
        {
            QMetaType metatype(meta_type_id);
            return (metatype.flags() & QMetaType::IsEnumeration) == QMetaType::IsEnumeration;
        }

        inline bool isGadget(int meta_type_id)
        {
            Pointers::PointerStub stub = Config::getPointerStub(meta_type_id);
            if (!stub.T) {
                if (stub.pT) {
                    return false;
                }
                else {
                    QMetaType metatype(meta_type_id);
                    return (metatype.flags() & QMetaType::IsGadget) == QMetaType::IsGadget;
                }
            }
            QMetaType metatype(stub.T);
            return (metatype.flags() & QMetaType::IsGadget) == QMetaType::IsGadget;
        }

        inline bool isPointer(int meta_type_id)
        {
            QMetaType metatype(meta_type_id);
            if (metatype.flags() & QMetaType::PointerToQObject) {
                return true;
            }
#if QT_VERSION_MINOR < 10
            if (Impl::pointerMap.contains(meta_type_id) && Impl::pointerMap[meta_type_id].T != meta_type_id) {
                return true;
            }
#else
            if (metatype.flags() & QMetaType::PointerToGadget) {
                return true;
            }
#endif
            return Impl::pointerMap.contains(meta_type_id) && Impl::pointerMap[meta_type_id].T != meta_type_id;
        }
        inline bool isWeakPointer(int meta_type_id)
        {
            if (Impl::pointerMap.contains(meta_type_id) &&
                    (Impl::pointerMap[meta_type_id].WT == meta_type_id ||
                     Impl::pointerMap[meta_type_id].wT == meta_type_id)) {
                return true;
            }
            return false;
        }

        int typeToValueType(int meta_type_id)
        {
            if (Impl::pointerMap.contains(meta_type_id)) {
                return Impl::pointerMap[meta_type_id].T;
            }
            return 0;
        }
        int typeToPointerType(int meta_type_id)
        {
            if (Impl::pointerMap.contains(meta_type_id)) {
                return Impl::pointerMap[meta_type_id].pT;
            }
            return 0;
        }

        bool isPrimitiveType(int meta_type_id)
        {
            return Impl::primitiveContainers.contains(meta_type_id);
        }
        bool isPrimitiveString(int meta_type_id)
        {
            return Impl::primitiveStringContainers.contains(meta_type_id);
        }
        bool isPrimitiveRaw(int meta_type_id)
        {
            return Impl::primitiveRawContainers.contains(meta_type_id);
        }

        bool isSequentialContainer(int meta_type_id)
        {
            return Impl::seqContTypeMap.keys().contains(meta_type_id);
        }

        bool isPair(int meta_type_id)
        {
            return Impl::pairTypes.contains(meta_type_id);
        }
        bool isAssociativeContainer(int meta_type_id)
        {
            return Impl::assContTypeMap.contains(meta_type_id);
        }

        int getSequentialContainerStoredType(int meta_type_id)
        {
            if (!Impl::seqContTypeMap.contains(meta_type_id)) return 0;
            return Impl::seqContTypeMap[meta_type_id];
        }

        int getAssociativeContainerStoredKeyType(int meta_type_id)
        {
            if (!Impl::assContTypeMap.contains(meta_type_id)) return 0;
            return Impl::assContTypeMap[meta_type_id].first;
        }
        int getAssociativeContainerStoredValueType(int meta_type_id)
        {
            if (!Impl::assContTypeMap.contains(meta_type_id)) return 0;
            return Impl::assContTypeMap[meta_type_id].second;
        }
        int getAssociativeContainerStoredType(int meta_type_id, int property) // 0 - key, 1 - value, 2 - orm_rowid
        {
            switch (property) {
            case 0: return getAssociativeContainerStoredKeyType(meta_type_id);
            case 1: return getAssociativeContainerStoredValueType(meta_type_id);
            case 2: return qMetaTypeId<long long>();
            default: return 0;
            }
        }

#if QT_VERSION_MINOR < 7
        bool isQObject(int meta_type_id)
        {
            return isQObject(*QMetaType::metaObjectForType(meta_type_id));
        }
        bool isQObject(QMetaObject const& meta)
        {
            QMetaObject const* qobject = QMetaType::metaObjectForType(QMetaType::QObjectStar);
            QMetaObject const* super = &meta;
            while (super) {
                if (super->className() == qobject->className()){
                    return true;
                }
                super = super->superClass();
            }
            return false;
        }
#else
        bool isQObject(int meta_type_id)
        {
            const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
            if (meta) {
                return meta->inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
            }
            return false;
        }
        bool isQObject(QMetaObject const& meta)
        {
            return meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
        }
#endif

        bool isIgnored(int meta_type_id)
        {
            return ignoredTypes.contains(meta_type_id);
        }

        bool isPrimitive(int meta_type_id)
        {
            return  isPrimitiveType(meta_type_id) ||
                    isPrimitiveString(meta_type_id) ||
                    isPrimitiveRaw(meta_type_id);
        }
        bool isTypeForTable(int meta_type_id)
        {
            if (isPrimitive(meta_type_id)) {
                return false;
            }
            return  isPointer(meta_type_id) ||
                    isGadget(meta_type_id) ||
                    isSequentialContainer(meta_type_id) ||
                    isAssociativeContainer(meta_type_id) ||
                    isPair(meta_type_id);
        }

        bool shouldSkipReadMeta(QMetaProperty const& property, QObject const* object)
        {
            return !property.isReadable();// || !property.isStored(object);
        }

        bool shouldSkipWriteMeta(QMetaProperty const& property, QObject const* object)
        {
            return !property.isWritable() || !property.isStored(object);
        }
        bool shouldSkipMeta(QMetaProperty const& property, QObject const* object)
        {
            return shouldSkipReadMeta(property, object) || shouldSkipWriteMeta(property, object);
        }
        bool shouldSkipMeta(QMetaObject const& obj, int property, QObject const* object)
        {
            for (int i = 0; i < obj.classInfoCount(); ++i) {
                if (    QString(obj.classInfo(i).name() ) == QString(orm_classInfo_skip) &&
                        QString(obj.classInfo(i).value()) == QString(obj.property(property).name())) {
                    return true;
                }
            }
            return shouldSkipReadMeta(obj.property(property), object) || shouldSkipWriteMeta(obj.property(property), object);
        }

        QString customTableName(QMetaObject const& obj)
        {
            for (int i = 0; i < obj.classInfoCount(); ++i) {
                if (QString(obj.classInfo(i).name() ) == QString(orm_classInfo_tableName)) {
                    return QString(obj.classInfo(i).value());
                }
            }
            return QString();
        }
        bool dontUseParentName(QMetaObject const& obj)
        {
            for (int i = 0; i < obj.classInfoCount(); ++i) {
                if (QString(obj.classInfo(i).name() ) == QString(orm_classInfo_tableNameNoParent)) {
                    return QString(obj.classInfo(i).value()) == "true";
                }
            }
            return false;
        }
        bool isPrimaryKey(QMetaObject const& obj, int property)
        {
            for (int i = 0; i < obj.classInfoCount(); ++i) {
                if (    QString(obj.classInfo(i).name() ) == QString(orm_classInfo_key) &&
                        QString(obj.classInfo(i).value()) == QString(obj.property(property).name())) {
                    return true;
                }
            }
            return false;
        }
        bool isRowID(const QString & property)
        {
            return QString(Impl::orm_rowidName) == property;
        }
        bool isRowID(QMetaObject const& obj, int property)
        {
            for (int i = 0; i < obj.classInfoCount(); ++i) {
                if (    QString(obj.classInfo(i).name() ) == QString(orm_classInfo_rowid) &&
                        QString(obj.classInfo(i).value()) == QString(obj.property(property).name())) {
                    return true;
                }
            }
            return isRowID(obj.property(property).name());
        }
        bool isUnique(QMetaObject const& obj, int property)
        {
            for (int i = 0; i < obj.classInfoCount(); ++i) {
                if (    QString(obj.classInfo(i).name() ) == QString(orm_classInfo_unique) &&
                        QString(obj.classInfo(i).value()) == QString(obj.property(property).name())) {
                    return true;
                }
            }
            return false;
        }
        bool isNotNull(QMetaObject const& obj, int property)
        {
            if (isPrimaryKey(obj, property)) {
                return true;
            }
            for (int i = 0; i < obj.classInfoCount(); ++i) {
                if (    QString(obj.classInfo(i).name() ) == QString(orm_classInfo_not_null) &&
                        QString(obj.classInfo(i).value()) == QString(obj.property(property).name())) {
                    return true;
                }
            }
            return false;
        }
        bool isAutoincrement(QMetaObject const& obj, int property)
        {
            for (int i = 0; i < obj.classInfoCount(); ++i) {
                if (    QString(obj.classInfo(i).name() ) == QString(orm_classInfo_autoincrement) &&
                        QString(obj.classInfo(i).value()) == QString(obj.property(property).name())) {
                    return true;
                }
            }
            return QString(Impl::orm_rowidName) == QString(obj.property(property).name());
        }
        bool withRowid(const QMetaObject &meta)
        {
            for (int i = 0; i < meta.propertyCount(); ++i) {
                QMetaProperty property = meta.property(i);
                if (shouldSkipMeta(property)) {
                    continue;
                }
                if (isPrimaryKey(meta, i)) {
                    return true;
                }
            }
            return false;
        }

        int actualMetaTypeIDOnce(int meta_type_id)
        {
            if (Impl::isEnumeration(meta_type_id)) {
                return qMetaTypeId<int>();
            }
            if (Config::isIgnoredType(meta_type_id)) {
                return 0;
            }
            if (Config::isPrimitiveRawType(meta_type_id)) {
                return qMetaTypeId<QByteArray>();
            }
            if (Config::isPrimitiveStringType(meta_type_id)) {
                return qMetaTypeId<QString>();
            }
            if (isPrimitiveType(meta_type_id)) {
                return meta_type_id;
            }
            if (Impl::isAssociativeContainer(meta_type_id) ||
                    meta_type_id == qMetaTypeId<QList<orm::Containers::ORM_QVariantPair>>()) {
                return qMetaTypeId<QList<orm::Containers::ORM_QVariantPair>>();
            }
            if (Impl::isPair(meta_type_id) ||
                    meta_type_id == qMetaTypeId<orm::Containers::ORM_QVariantPair>()) {
                return qMetaTypeId<orm::Containers::ORM_QVariantPair>();
            }
            if (Impl::isSequentialContainer(meta_type_id)) {
                return Impl::getSequentialContainerStoredType(meta_type_id);
            }
            if (Impl::isWeakPointer(meta_type_id)) {
                return 0;
            }
            orm::Pointers::PointerStub stab = Config::getPointerStub(meta_type_id);
            if (stab.T) {
                return stab.T;
            }
            if (stab.pT) {
                return stab.pT;
            }
            return meta_type_id;
        }
        int actualMetaTypeID(int meta_type_id)
        {
            int id = actualMetaTypeIDOnce(meta_type_id);
            while (id != meta_type_id) {
                int i = actualMetaTypeIDOnce(meta_type_id);
                meta_type_id = id;
                id = i;
            }
            return id;
        }
        QList<int> actualMetaTypeIDQueue(int meta_type_id)
        {
            QList<int> ret;
            ret << meta_type_id;
            int id = actualMetaTypeIDOnce(meta_type_id);
            while (id != meta_type_id) {
                int i = actualMetaTypeIDOnce(meta_type_id);
                meta_type_id = id;
                ret << meta_type_id;
                id = i;
            }
            if (!id) {
                ret.clear();
            }
            return ret;
        }

        QMetaObject const* meta_object(int meta_type_id)
        {
            QMetaObject const* meta_ptr = QMetaType::metaObjectForType(meta_type_id);
            if (meta_ptr) {
                return meta_ptr;
            }
            meta_ptr = QMetaType::metaObjectForType(orm::Impl::actualMetaTypeID(meta_type_id));
            if (meta_ptr) {
                return meta_ptr;
            }
            return nullptr;
        }




        bool write(int meta_id, int property_index, QVariant & writeInto, const QVariant & value)
        {
            QVariant val;
            if (        orm::Impl::isPair(meta_id)
                    ||  orm::Impl::isAssociativeContainer(meta_id)
                    ||  meta_id == qMetaTypeId<orm::Containers::ORM_QVariantPair>()) {
                if (        (    Impl::isSequentialContainer(value.userType())
                             ||  value.userType() == qMetaTypeId<QVariantList>())
                        &&  value.userType() != qMetaTypeId<QList<orm::Containers::ORM_QVariantPair>>()
                        && !Impl::isSequentialContainer(Impl::getAssociativeContainerStoredType(meta_id,property_index))) {
                    QVariantList v = value.value<QVariantList>();
                    if (!v.isEmpty()) {
                        val = v.first();
                    }
                }
                else {
                    val = value;
                }
                //if (        value.userType() == qMetaTypeId<QVariantList>()
                return orm::Containers::ORM_QVariantPair::staticMetaObject.property(property_index).writeOnGadget(writeInto.data(), val);
            }
            QMetaObject const* obj = orm::Impl::meta_object(meta_id);
            if (!obj) {
                qDebug() << "Can't write anything to metatype " << meta_id;
                return false;
            }
            if (        value.userType() == qMetaTypeId<QVariantList>()
                    && !Impl::isSequentialContainer(obj->property(property_index).userType())
                    && !Impl::isAssociativeContainer(obj->property(property_index).userType())) {
                QVariantList v = value.value<QVariantList>();
                if (!v.isEmpty()) {
                    val = v.first();
                }
            }
            else {
                val = value;
            }
            if (        obj->property(property_index).userType() != val.userType()
                    &&  val.isValid()
                    && !val.canConvert(obj->property(property_index).userType())) {
                qDebug() << "Can not convert " << QMetaType::typeName(val.userType()) << "(" << val.userType() << ") to "
                         << QMetaType::typeName(obj->property(property_index).userType()) << "(" << obj->property(property_index).userType() << ") for "
                         << QMetaType::typeName(meta_id) << "::" << obj->property(property_index).name() << " (" << meta_id << ")";
                return false;
            }
            if (Impl::isQObject(meta_id)) {
                QObject * object = writeInto.value<QObject*>();
                if (Impl::shouldSkipWriteMeta(obj->property(property_index), object)) {
                    return false;
                }
                return obj->property(property_index).write(object, val);
            }
            else {
                if (writeInto.canConvert(qMetaTypeId<orm::Pointers::VoidPointer>())) {
                    return obj->property(property_index).writeOnGadget(writeInto.value<orm::Pointers::VoidPointer>().data, val);
                }
                else {
                    return obj->property(property_index).writeOnGadget(writeInto.data(), val);
                }
            }
        }

        QVariant read(int meta_id, int property_index, QVariant const&  readFrom)
        {
            QVariant result;
            QMetaObject const* obj = orm::Impl::meta_object(meta_id);
            if (!obj) {
                if (Impl::isPair(meta_id) || Impl::isAssociativeContainer(meta_id)) {
                    result = readFrom.value<orm::Containers::ORM_QVariantPair>()[property_index];
                }
                else {
                    qDebug() << "Can't read anything from metatype " << meta_id;
                }
            }
            else {
                if (Impl::isQObject(meta_id)) {
                    QObject const* object = readFrom.value<QObject*>();
                    if (!object) {
                        qDebug() << "Error converting data" << meta_id << QMetaType::typeName(meta_id) << property_index << obj->property(property_index).name() << readFrom;
                        return QVariant();
                    }
                    if (!Impl::shouldSkipReadMeta(obj->property(property_index), object)) {
                        result = obj->property(property_index).read(object);
                    }
                }
                else {
                    if (readFrom.canConvert(qMetaTypeId<orm::Pointers::VoidPointer>())) {
                        result = obj->property(property_index).readOnGadget(readFrom.value<orm::Pointers::VoidPointer>().data);
                    }
                    else {
                        result = obj->property(property_index).readOnGadget(readFrom.data());
                    }
                }
                if (result.isNull()) {
                    return QVariant();
                }

                if (Impl::isPrimitiveString(obj->property(property_index).userType())) {
                    result.convert(qMetaTypeId<QString>());
                }
                if (Impl::isPrimitiveRaw(obj->property(property_index).userType())) {
                    QByteArray array;
                    QDataStream stream(&array, QIODevice::ReadWrite);
                    stream << result;
                    result = array;
                }
            }
            return result;
        }

        QVariant makeStoreQVariantExact(int metatype)
        {
            QVariant o;
            QMetaObject const* meta = QMetaType::metaObjectForType(metatype);
            if (meta) {
                if (Impl::isQObject(*meta)) {
                    QObject * obj = meta->newInstance();
#ifdef QT_DEBUG
                    obj->setParent(QCoreApplication::instance());
#endif
                    return QVariant::fromValue(obj);
                }
                if (!Impl::isQObject(*meta)) {
                    return QVariant(metatype, nullptr);
                }
            }
            else {
                if (isPair(metatype) || isAssociativeContainer(metatype)) {
                    return QVariant::fromValue(orm::Containers::ORM_QVariantPair());
                }
                if (isSequentialContainer(metatype) && (isPair(getSequentialContainerStoredType(metatype)) || isAssociativeContainer(getSequentialContainerStoredType(metatype)))) {
                    return QVariant::fromValue(orm::Containers::ORM_QVariantPair());
                }
                orm::Pointers::PointerStub stub = orm::Config::getPointerStub(metatype);
                int type = stub.pT;
                meta = QMetaType::metaObjectForType(type);
                if (meta) {
                    if (Impl::isQObject(*meta)) {
                        QObject * obj = meta->newInstance();
#ifdef QT_DEBUG
                        obj->setParent(QCoreApplication::instance());
                        if ((obj)) {
                            QObject::connect(obj, &QObject::destroyed, [obj](QObject * d) { qDebug() << "automaticly deleted " << obj << d << " Check for memory leaks."; });
                        }
#endif
                        o = QVariant::fromValue(obj);
                    }
                    else {
                        o = QVariant::fromValue(orm::Pointers::NewStub());
                        o.convert(stub.pT);
                    }
                }
                else {
                    return QVariant(metatype, nullptr);
                }
            }
            if (o.isValid()) {
                o.convert(qMetaTypeId<orm::Pointers::VoidPointer>());
                o.convert(metatype);
                return o;
            }
            return QVariant();
        }

        QVariant makeStoreQVariant(int metatype)
        {
            if (isPair(metatype) || isAssociativeContainer(metatype)) {
                return QVariant::fromValue(orm::Containers::ORM_QVariantPair());
            }
            if (isSequentialContainer(metatype)) {
                return makeStoreQVariant(Impl::getSequentialContainerStoredType(metatype));
            }
            if (!Impl::isPointer(metatype)) {
                return QVariant(metatype, nullptr);
            }
            orm::Pointers::PointerStub stub = orm::Config::getPointerStub(metatype);
            QMetaObject const* meta = QMetaType::metaObjectForType(stub.pT);
            if (meta && Impl::isQObject(*meta)) {
                    QObject * obj = meta->newInstance();
#ifdef QT_DEBUG
                    obj->setParent(QCoreApplication::instance());
#endif
                    return QVariant::fromValue(obj);
            }
            if (stub.pT) {
                QVariant o = QVariant::fromValue(orm::Pointers::NewStub());
                o.convert(stub.pT);
                return o;
            }
            return QVariant();
        }

        void registerPrimitiveTypeContainers()
        {
            orm::Containers::registerPrimitiveTypeContainer<QList>();
            orm::Containers::registerPrimitiveTypeContainer<QVector>();
        }


    }





    orm::Pointers::PointerStub Config::getPointerStub(int metaTypeID)
    {
        if (Impl::pointerMap.contains(metaTypeID)) return Impl::pointerMap[metaTypeID];
        return orm::Pointers::PointerStub();
    }
    void Config::addPointerStub(const orm::Pointers::PointerStub & stub)
    {
        if (stub. T) Impl::pointerMap[stub. T] = stub;
        if (stub.pT) Impl::pointerMap[stub.pT] = stub;
        if (stub.ST) Impl::pointerMap[stub.ST] = stub;
        if (stub.WT) Impl::pointerMap[stub.WT] = stub;
        if (stub.sT) Impl::pointerMap[stub.sT] = stub;
        if (stub.wT) Impl::pointerMap[stub.wT] = stub;
    }

    void Config::addPairType(int firstTypeID, int secondTypeID, int pairTypeID)
    {
        addContainerPairType(firstTypeID, secondTypeID, pairTypeID);
        if (!Impl::pairTypes.contains(pairTypeID)) {
            Impl::pairTypes << pairTypeID;
        }
    }
    void Config::addSeqContainerType(int seqTypeID, int innerTypeID)
    {
        if (!Impl::seqContTypeMap.contains(seqTypeID)) {
            Impl::seqContTypeMap[seqTypeID] = innerTypeID;
        }
    }
    void Config::addContainerPairType(int firstTypeID, int secondTypeID, int pairTypeID)
    {
        if (!Impl::assContTypeMap.contains(pairTypeID)) {
            Impl::assContTypeMap[pairTypeID] = QPair<int, int>(firstTypeID, secondTypeID);
        }
    }

    void Config::addIgnoredType(int meta_type_id)
    {
        Impl::ignoredTypes << meta_type_id;
    }

    bool Config::isIgnoredType(int meta_type_id)
    {
        return Impl::ignoredTypes.contains(meta_type_id);
    }

    void Config::removeIgnoredType(int meta_type_id)
    {
        Impl::primitiveContainers.remove(meta_type_id);
    }

    void Config::addPrimitiveType(int meta_type_id)
    {
        Impl::primitiveContainers << meta_type_id;
    }

    bool Config::isPrimitiveType(int meta_type_id)
    {
        return Impl::isPrimitiveType(meta_type_id);
    }

    void Config::removePrimitiveType(int meta_type_id)
    {
        Impl::primitiveContainers.remove(meta_type_id);
    }

    void Config::addPrimitiveStringType(int meta_type_id)
    {
        Q_ASSERT_X( QMetaType::hasRegisteredConverterFunction(meta_type_id, qMetaTypeId<QString>()) &&
                    QMetaType::hasRegisteredConverterFunction(qMetaTypeId<QString>(), meta_type_id),
                    "addPrimitiveStringType", "Needs registred converters T->QString->T");
        Impl::primitiveStringContainers << meta_type_id;
    }

    bool Config::isPrimitiveStringType(int meta_type_id)
    {
        return Impl::isPrimitiveString(meta_type_id);
    }

    void Config::removePrimitiveStringType(int meta_type_id)
    {
        Impl::primitiveStringContainers.remove(meta_type_id);
    }

    void Config::addPrimitiveRawType(int meta_type_id)
    {
        Impl::primitiveRawContainers << meta_type_id;
    }

    bool Config::isPrimitiveRawType(int meta_type_id)
    {
        return Impl::isPrimitiveRaw(meta_type_id);
    }

    void Config::removePrimitiveRawType(int meta_type_id)
    {
        Impl::primitiveRawContainers.remove(meta_type_id);
    }













    ORM::ORM(const QString & dbname) : m_databaseName(QSqlDatabase::defaultConnection)
    {
        if (!dbname.isEmpty()) {
            m_databaseName = dbname;
        }
        if (Impl::orm_once) {
            Impl::orm_once = false;
            orm::Pointers::registerTypePointers<QObject>();
            orm::Pointers::registerTypeSmartPointers<QObject>();
            Config::addIgnoredType<void>();
            Config::addPrimitiveType<bool     >();
            Config::addPrimitiveType<int      >();
            Config::addPrimitiveType<uint     >();
            Config::addPrimitiveType<qlonglong >();
            Config::addPrimitiveType<qulonglong>();
            Config::addPrimitiveType<double   >();
            Config::addPrimitiveType<long     >();
            Config::addPrimitiveType<short    >();
            Config::addPrimitiveType<char     >();
            Config::addPrimitiveType<ulong    >();
            Config::addPrimitiveType<ushort   >();
            Config::addPrimitiveType<uchar    >();
            Config::addPrimitiveType<float    >();
            Config::addPrimitiveType<signed char    >();
            Config::addIgnoredType<std::nullptr_t>();
            Config::addPrimitiveType<void*>();
            Config::addPrimitiveRawType<QChar>();
            Config::addPrimitiveType<QString>();
            Config::addPrimitiveRawType<QStringList>();
            Config::addPrimitiveType<QByteArray>();
            Config::addPrimitiveRawType<QBitArray>();
            Config::addPrimitiveType<QDate>();
            Config::addPrimitiveType<QTime>();
            Config::addPrimitiveType<QDateTime>();
            Config::addPrimitiveType<QUrl>();
            Config::addPrimitiveRawType<QLocale>();
            Config::addPrimitiveRawType<QRect>();
            Config::addPrimitiveRawType<QRectF>();
            Config::addPrimitiveRawType<QSize>();
            Config::addPrimitiveRawType<QSizeF>();
            Config::addPrimitiveRawType<QLine>();
            Config::addPrimitiveRawType<QLineF>();
            Config::addPrimitiveRawType<QPoint>();
            Config::addPrimitiveRawType<QPointF>();
            Config::addPrimitiveRawType<QRegExp>();
            Config::addPrimitiveRawType<QEasingCurve>();
            Config::addPrimitiveType<QUuid>();
            Config::addPrimitiveRawType<QVariant>();
            Config::addIgnoredType<QModelIndex>();
            Config::addPrimitiveRawType<QRegularExpression>();
            Config::addIgnoredType<QJsonValue>();
            Config::addIgnoredType<QJsonObject>();
            Config::addIgnoredType<QJsonArray>();
            Config::addIgnoredType<QJsonDocument>();
            Config::addIgnoredType<QPersistentModelIndex>();
            Config::addIgnoredType<QObject>();
            Config::addPrimitiveRawType<QVariantMap>();
            Config::addPrimitiveRawType<QVariantList>();
            Config::addPrimitiveRawType<QVariantHash>();
            Config::addPrimitiveRawType<QByteArrayList>();
            Config::addPrimitiveRawType<QFont>();
            Config::addPrimitiveRawType<QPixmap>();
            Config::addPrimitiveRawType<QBrush>();
            Config::addPrimitiveRawType<QColor>();
            Config::addPrimitiveRawType<QPalette>();
            Config::addPrimitiveRawType<QIcon>();
            Config::addPrimitiveRawType<QImage>();
            Config::addPrimitiveRawType<QPolygon>();
            Config::addPrimitiveRawType<QRegion>();
            Config::addPrimitiveRawType<QBitmap>();
            Config::addPrimitiveRawType<QCursor>();
            Config::addPrimitiveRawType<QKeySequence>();
            Config::addPrimitiveRawType<QPen>();
            Config::addPrimitiveRawType<QTextLength>();
            Config::addPrimitiveRawType<QTextFormat>();
            Config::addPrimitiveRawType<QMatrix>();
            Config::addPrimitiveRawType<QTransform>();
            Config::addPrimitiveRawType<QMatrix4x4>();
            Config::addPrimitiveRawType<QVector2D>();
            Config::addPrimitiveRawType<QVector3D>();
            Config::addPrimitiveRawType<QVector4D>();
            Config::addPrimitiveRawType<QQuaternion>();
            Config::addPrimitiveRawType<QPolygonF>();
            Config::addPrimitiveRawType<QSizePolicy>();

            qRegisterMetaType<orm::Pointers::VoidPointer>("orm::Pointers::VoidPointer");
            QMetaType::registerDebugStreamOperator<orm::Containers::ORM_QVariantPair>();
            QMetaType::registerDebugStreamOperator<QList<orm::Containers::ORM_QVariantPair>>();

            orm::Containers::registerPrimitiveTypeContainer<QList>();
            orm::Containers::registerPrimitiveTypeContainer<QVector>();
            //orm::Register<orm::containers::ORM_QVariantPair>();
        }
    }
    ORM::~ORM()
    {
    }


    QString ORM::databaseName() const
    {
        return m_databaseName;
    }
    void ORM::setDatabaseName(const QString & databaseName)
    {
        if (m_databaseName != databaseName) {
            m_databaseName = databaseName;
        }
    }

    void     ORM::id_create(int meta_type_id)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            ORM::TableEntry te = ORM_Impl::queryData(this, meta_type_id);
            ORM::ORM_Impl::meta_create(this, te);
        }
    }
    QVariant ORM::id_select(int meta_type_id)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            ORM::TableEntry te = ORM_Impl::queryData(this, meta_type_id);
            return ORM::ORM_Impl::meta_select(this, te);
        }
        return QVariant();
    }

    QVariant ORM::id_select_where(int meta_type_id, const QVariantMap & wheres)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            //return ORM::ORM_Impl::meta_select_where(this, meta_type_id, wheres);
        }
        return QVariant();
    }

    QVariant ORM::id_get(int meta_type_id, long long id)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            //return ORM::ORM_Impl::meta_get(this, meta_type_id, id);
        }
        return QVariant();
    }
    void     ORM::id_insert(int meta_type_id, QVariant &value)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            ORM::TableEntry te = ORM_Impl::queryData(this, meta_type_id);
            ORM::ORM_Impl::meta_insert(this, te, value);
        }
    }
    void     ORM::id_delete (int meta_type_id, QVariant &value)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            ORM::TableEntry te = ORM_Impl::queryData(this, meta_type_id);
            ORM::ORM_Impl::meta_delete(this, te, value);
        }
    }
    void     ORM::id_update(int meta_type_id, QVariant &value)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            ORM::TableEntry te = ORM_Impl::queryData(this, meta_type_id);
            ORM::ORM_Impl::meta_update(this, te, value);
        }
    }
    void     ORM::id_drop  (int meta_type_id)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            ORM::TableEntry te = ORM_Impl::queryData(this, meta_type_id);
            ORM::ORM_Impl::meta_drop(this, te);
        }
    }



    QString ORM::ORM_Impl::rowidName()
    {
        return Impl::orm_rowidName;
    }

    QString orm::ORM::ORM_Impl::parentRowidName()
    {
        return Impl::orm_parentRowidName;
    }



    void queryDataRecursive(ORM * orm, ORM::TableEntry * parent, ORM::TableEntry & me)
    {
        if (orm && orm->m_bufferization) {
            if (!parent) {
                if (orm->m_types.contains(me.orig_meta_type_id) && orm->m_types[me.orig_meta_type_id].contains(QString())) {
                    me = orm->m_types[me.orig_meta_type_id][QString()];
                    return;
                }
            }
            else {
                if (orm->m_types.contains(me.orig_meta_type_id) && orm->m_types[me.orig_meta_type_id].contains(parent->field_name)) {
                    me = orm->m_types[me.orig_meta_type_id][parent->field_name];
                    return;
                }
            }
        }
        QMetaObject const* meta_ptr = orm::Impl::meta_object(me.orig_meta_type_id);
        me.norm_meta_type_id = Impl::actualMetaTypeID(me.orig_meta_type_id);
        me.is_field = !Impl::isTypeForTable(me.orig_meta_type_id);

        if (me.is_field) {
            if (parent) {
                if (meta_ptr) {
                    me.is_primary_key   = Impl::isPrimaryKey   (*meta_ptr, me.property_index);
                    me.is_orm_row_id    = Impl::isRowID        (*meta_ptr, me.property_index);
                    me.is_unique        = Impl::isUnique       (*meta_ptr, me.property_index) | me.is_primary_key;
                    me.is_not_null      = Impl::isNotNull      (*meta_ptr, me.property_index) | me.is_primary_key;
                    me.is_autoincrement = Impl::isAutoincrement(*meta_ptr, me.property_index);
                }
                else {
                    me.is_orm_row_id    = Impl::isRowID        (me.property_name);
                    me.is_unique        = me.is_orm_row_id;
                    me.is_not_null      = me.is_orm_row_id;
                    me.is_autoincrement = me.is_orm_row_id;
                }
            }
            if (orm) {
                me.field_name = orm->normalize(me.property_name);
            }
            else {
                me.field_name = me.property_name;
            }
            return;
        }
        me.is_QObject = Impl::isQObject(me.orig_meta_type_id);
        if (meta_ptr) {
            QString custom = Impl::customTableName(*meta_ptr);
            if (custom.isEmpty()) {
                custom = QMetaType::typeName(me.norm_meta_type_id);
            }
            if (!Impl::dontUseParentName(*meta_ptr)) {
                if (orm) {
                    if (parent) {
                            custom = orm->generate_table_name(parent->field_name, me.property_name, custom);
                    }
                    else {
                        custom = orm->generate_table_name(QString(), QString(), custom);
                    }
                }
            }
            me.field_name = custom;
            for (int i = 0; i < meta_ptr->propertyCount(); ++i) {
                QMetaProperty property = meta_ptr->property(i);
                if (!property.isStored() || !property.isWritable() || !property.isReadable()) {
                    continue;
                }
                int actualID = property.userType(); //Impl::actualMetaTypeID(property.userType());
                if (Impl::isIgnored(actualID)) {
                    continue;
                }
                ORM::TableEntry entry;
                entry.property_index = i;
                entry.property_name = property.name();
                entry.orig_meta_type_id = property.userType();
                queryDataRecursive(orm, &me, entry);
                me.properties[i] = entry;
                me.has_orm_row_id |= entry.is_orm_row_id;
                me.has_primary_key |= entry.is_primary_key;
            }
        }
        else {
            if (parent) {
                me.field_name = orm->generate_table_name(parent->field_name, me.property_name, QMetaType::typeName(me.norm_meta_type_id));
            }
            else {
                me.field_name = orm->generate_table_name(QString(), QString(), QMetaType::typeName(me.norm_meta_type_id));
                me.property_name = QMetaType::typeName(me.orig_meta_type_id);
                me.property_index = -1;
            }
            if (orm::Impl::isPair(me.orig_meta_type_id) || orm::Impl::isAssociativeContainer(me.orig_meta_type_id)) {
                me.is_container = orm::Impl::isAssociativeContainer(me.orig_meta_type_id);
                me.norm_meta_type_id = qMetaTypeId<orm::Containers::ORM_QVariantPair>();
                ORM::TableEntry entry0;
                entry0.property_index = 2;
                entry0.property_name = "rowid";
                entry0.field_name = Impl::orm_rowidName;
                entry0.orig_meta_type_id = qMetaTypeId<long long>();
                entry0.norm_meta_type_id = qMetaTypeId<long long>();
                entry0.is_primary_key = false;
                entry0.is_orm_row_id = true;
                me.has_orm_row_id = true;
                me.properties[2] = entry0;
                for (int column = 0; column < 2; ++column) {
                    int type = (column == 0)
                            ? Impl::getAssociativeContainerStoredKeyType(me.orig_meta_type_id)
                            : Impl::getAssociativeContainerStoredValueType(me.orig_meta_type_id);
                    if (Impl::isIgnored(type)) {
                        continue;
                    }
                    QString name = (column == 0) ? "key" : "value";
                    ORM::TableEntry entry;
                    entry.property_index = column;
                    entry.property_name = name;
                    entry.orig_meta_type_id = type;
                    queryDataRecursive(orm, &me, entry);
                    me.properties[entry.property_index] = entry;
                }
            }
            if (orm::Impl::isSequentialContainer(me.orig_meta_type_id)) {
                me.is_container = true;
                for (int column = 0; column < 2; ++column) {
                    int type = (column == 0)
                            ? qMetaTypeId<int>()
                            : Impl::getSequentialContainerStoredType(me.orig_meta_type_id);
                    if (Impl::isIgnored(type)) {
                        continue;
                    }
                    QString name = (column == 0) ? "index" : "value";
                    ORM::TableEntry entry;
                    entry.property_index = column;
                    entry.property_name = name;
                    entry.orig_meta_type_id = type;
                    queryDataRecursive(orm, &me, entry);
                    me.properties[entry.property_index] = entry;
                }
            }
        }
        if (orm && orm->m_bufferization && !me.is_field) {
            if (parent) {
                orm->m_types[me.orig_meta_type_id][parent->field_name] = me;
            }
            else {
                orm->m_types[me.orig_meta_type_id][QString()] = me;
            }
        }
    }

    ORM::TableEntry ORM::ORM_Impl::queryData(ORM * orm, int meta_type_id)
    {
        ORM::TableEntry ret;
        if (orm::Impl::isIgnored(meta_type_id)) {
            return ret;
        }
        ret.orig_meta_type_id = meta_type_id;
        queryDataRecursive(orm, nullptr, ret);
        return ret;
    }

    void ORM::ORM_Impl::meta_create(ORM * orm, ORM::TableEntry & entry, TableEntry * parent)
    {
        if (!entry.orig_meta_type_id || entry.is_field) {
            qDebug() << "Invalid metatype entry" << entry.orig_meta_type_id << parent;
            return;
        }
        QString queryText = orm->m_createQueries.contains(entry.field_name)
                ? orm->m_createQueries[entry.field_name]
                : orm->generate_create_query(entry, parent);
        if (!orm->m_createQueries.contains(entry.field_name) && orm->m_bufferization) {
            orm->m_createQueries[entry.field_name] = queryText;
        }
        QSqlQuery query(QSqlDatabase::database(orm->m_databaseName, true));
        query.exec(queryText);

        if (query.lastError().isValid()) {
            qDebug() << entry.orig_meta_type_id << entry.field_name;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << queryText;
            qDebug() << query.executedQuery();
            qDebug() << query.boundValues();
            return;
        }
        for (auto & i : entry.properties) {
            if (!i.is_field) {
                meta_create(orm, i, &entry);
            }
        }
    }

    void     ORM::ORM_Impl::meta_insert(ORM * orm, TableEntry & entry, QVariant &value, TableEntry * parent, const QVariant & parentVal)
    {
        if (!value.isValid()) {
            qDebug() << "Invalid value for insert of " << entry.field_name;
            return;
        }
        if (!entry.orig_meta_type_id || entry.is_field) {
            qDebug() << "Invalid metatype entry" << entry.orig_meta_type_id << parent;
            return;
        }
        QString queryText = orm->m_insertQueries.contains(entry.field_name) ? orm->m_insertQueries[entry.field_name] : orm->generate_insert_query(entry, parent);
        if (!orm->m_insertQueries.contains(entry.field_name) && orm->m_bufferization) {
            orm->m_insertQueries[entry.field_name] = queryText;
        }
        QSqlQuery query(QSqlDatabase::database(orm->m_databaseName, true));
        query.prepare(queryText);
        if (parent) {
            if (parent->has_orm_row_id) {
                for (auto i : parent->properties) {
                    if (i.is_orm_row_id) {
                        QVariant temp = Impl::read(parent->orig_meta_type_id, i.property_index, parentVal);
                        if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                            temp.convert(qMetaTypeId<QString>());
                        }
                        if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                            QByteArray array;
                            QDataStream stream(&array, QIODevice::ReadWrite);
                            stream << temp;
                            temp = array;
                            //w.convert(qMetaTypeId<QByteArray>());
                        }
                        query.bindValue(QString(":parent_") + i.field_name, temp);
                    }
                }
            }
            else if (parent->has_primary_key) {
                for (auto i : parent->properties) {
                    if (i.is_primary_key) {
                        QVariant temp = Impl::read(parent->orig_meta_type_id, i.property_index, parentVal);
                        if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                            temp.convert(qMetaTypeId<QString>());
                        }
                        if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                            QByteArray array;
                            QDataStream stream(&array, QIODevice::ReadWrite);
                            stream << temp;
                            temp = array;
                            //w.convert(qMetaTypeId<QByteArray>());
                        }
                        query.bindValue(QString(":parent_") + i.field_name, temp);
                    }
                }
            }
        }
        for (auto i : entry.properties) {
            if (!i.is_field) {
                continue;
            }
            QVariant temp = Impl::read(entry.orig_meta_type_id, i.property_index, value);
            if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                temp.convert(qMetaTypeId<QString>());
            }
            if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                QByteArray array;
                QDataStream stream(&array, QIODevice::ReadWrite);
                stream << temp;
                temp = array;
                //w.convert(qMetaTypeId<QByteArray>());
            }
            query.bindValue(QString(":") + i.field_name, temp);
        }
        query.exec();
        if (query.lastError().isValid()) {
            qDebug() << entry.orig_meta_type_id << entry.field_name;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.executedQuery();
            qDebug() << query.boundValues().keys();
            qDebug() << query.boundValues();
        }
        long long ormRowid = orm->get_last_inserted_rowid(query);
        if (entry.has_orm_row_id) {
            int id = -1;
            for (int i = 0; i < entry.properties.size(); ++i) {
                if (entry.properties[i].is_orm_row_id) {
                    id = i;
                    break;
                }
            }
            if (id >= 0) {
                Impl::write(entry.orig_meta_type_id,id, value, ormRowid);
            }
        }
        query.finish();
        for (auto i : entry.properties) {
            if (i.is_field) {
                continue;
            }
            QVariant temp = Impl::read(entry.orig_meta_type_id, i.property_index, value);
            orm::Impl::for_each(i.orig_meta_type_id, temp, [&orm, &i, &entry, &value](int meta_type_id, QVariant & var, int index) {
                meta_insert(orm, i, var, &entry, value);
            });
        }
    }

    QVariant ORM::ORM_Impl::meta_select(ORM * orm, TableEntry & entry, TableEntry * parent, const QVariant & parentVal)
    {
        if (!entry.orig_meta_type_id || entry.is_field) {
            qDebug() << "Invalid metatype entry" << entry << parent;
            return QVariant();
        }
        QString queryText = orm->m_selectQueries.contains(entry.field_name)
                ? orm->m_selectQueries[entry.field_name]
                : orm->generate_select_query(entry, parent);
        if (!orm->m_selectQueries.contains(entry.field_name) && orm->m_bufferization) {
            orm->m_selectQueries[entry.field_name] = queryText;
        }

        QSqlQuery query(QSqlDatabase::database(orm->m_databaseName, true));
        query.prepare(queryText);

        if (parent) {
            if (parent->has_orm_row_id) {
                for (auto i : parent->properties) {
                    if (i.is_orm_row_id) {
                        QVariant temp = Impl::read(parent->orig_meta_type_id, i.property_index, parentVal);
                        if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                            temp.convert(qMetaTypeId<QString>());
                        }
                        if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                            QByteArray array;
                            QDataStream stream(&array, QIODevice::ReadWrite);
                            stream << temp;
                            temp = array;
                            //w.convert(qMetaTypeId<QByteArray>());
                        }
                        query.bindValue(QString(":parent_") + i.field_name, temp);
                    }
                }
            }
            else if (parent->has_primary_key) {
                for (auto i : parent->properties) {
                    if (i.is_primary_key) {
                        QVariant temp = Impl::read(parent->orig_meta_type_id, i.property_index, parentVal);
                        if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                            temp.convert(qMetaTypeId<QString>());
                        }
                        if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                            QByteArray array;
                            QDataStream stream(&array, QIODevice::ReadWrite);
                            stream << temp;
                            temp = array;
                            //w.convert(qMetaTypeId<QByteArray>());
                        }
                        query.bindValue(QString(":parent_") + i.field_name, temp);
                    }
                }
            }
        }
        query.exec();
        if (query.lastError().isValid()) {
            qDebug() << entry << entry.field_name;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.executedQuery();
            qDebug() << query.boundValues();
        }

        QVariantList resultList;
        while (query.next()) {
            QVariant variant = Impl::makeStoreQVariant(entry.orig_meta_type_id);
            if (!variant.isValid()){
                qWarning() << "select: Unable to create instance of type " << entry.orig_meta_type_id;
                break;
            }
            if ( entry.is_QObject && variant.value<QObject*>() == nullptr) {
                qWarning() << "select: Unable to create instance of QObject " << entry.orig_meta_type_id;
                break;
            }
            long long orm_rowid = 0;
            for (auto const& i : entry.properties) {
                if (i.is_field) {
                    QVariant queryValue = query.value(i.field_name);
                    if (Impl::isEnumeration(i.orig_meta_type_id)) {
                        queryValue.convert(i.orig_meta_type_id);
                    }
                    if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                        queryValue.convert(i.orig_meta_type_id);
                    }
                    if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                        QVariant temp;
                        QByteArray array = queryValue.toByteArray();
                        QDataStream stream(&array,QIODevice::ReadWrite);
                        stream >> temp;
                        queryValue = temp;
                    }
                    if (!orm_rowid && i.is_primary_key && queryValue.canConvert<long long>()) {
                        orm_rowid = queryValue.toLongLong();
                    }
                    Impl::write(entry.norm_meta_type_id, i.property_index, variant, queryValue);
                }
            }
            for (auto & i : entry.properties) {
                if (!i.is_field) {
                    QVariant queryValue = meta_select(orm, i, &entry, variant).toList();
                    if (queryValue.isValid()) {
                        Impl::write(entry.norm_meta_type_id, i.property_index, variant, queryValue);
                    }
                }
            }
            resultList << variant;
        }
        return QVariant(resultList);
    }

    void ORM::ORM_Impl::meta_update(ORM * orm, ORM::TableEntry & entry, QVariant & value, ORM::TableEntry * parent, const QVariant & parentVal)
    {
        if (!value.isValid()) {
            qDebug() << "Invalid value for update of " << entry.field_name;
            return;
        }
        if (!entry.orig_meta_type_id || entry.is_field) {
            qDebug() << "Invalid metatype entry" << entry.orig_meta_type_id << parent;
            qDebug() << entry.field_name << entry.orig_meta_type_id << entry.norm_meta_type_id << entry.is_field << entry.is_QObject;
            qDebug() << Impl::isTypeForTable(entry.orig_meta_type_id);
            qDebug() << Impl::isPointer(entry.orig_meta_type_id)
                     << Impl::isGadget(entry.orig_meta_type_id)
                     << Impl::isSequentialContainer(entry.orig_meta_type_id)
                     << Impl::isAssociativeContainer(entry.orig_meta_type_id)
                     << Impl::isPair(entry.orig_meta_type_id);
            return;
        }
        QString queryText = orm->m_updateQueries.contains(entry.field_name) ? orm->m_updateQueries[entry.field_name] : orm->generate_update_query(entry, parent);
        if (!orm->m_updateQueries.contains(entry.field_name) && orm->m_bufferization) {
            orm->m_updateQueries[entry.field_name] = queryText;
        }
        QSqlQuery query(QSqlDatabase::database(orm->m_databaseName, true));
        query.prepare(queryText);

        if (parent) {
            if (parent->has_orm_row_id) {
                for (auto i : parent->properties) {
                    if (i.is_orm_row_id) {
                        QVariant temp = Impl::read(parent->orig_meta_type_id, i.property_index, parentVal);
                        if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                            temp.convert(qMetaTypeId<QString>());
                        }
                        if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                            QByteArray array;
                            QDataStream stream(&array, QIODevice::ReadWrite);
                            stream << temp;
                            temp = array;
                            //w.convert(qMetaTypeId<QByteArray>());
                        }
                        query.bindValue(QString(":parent_") + i.field_name, temp);
                    }
                }
            }
            else if (parent->has_primary_key) {
                for (auto i : parent->properties) {
                    if (i.is_primary_key) {
                        QVariant temp = Impl::read(parent->orig_meta_type_id, i.property_index, parentVal);
                        if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                            temp.convert(qMetaTypeId<QString>());
                        }
                        if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                            QByteArray array;
                            QDataStream stream(&array, QIODevice::ReadWrite);
                            stream << temp;
                            temp = array;
                            //w.convert(qMetaTypeId<QByteArray>());
                        }
                        query.bindValue(QString(":parent_") + i.field_name, temp);
                    }
                }
            }
        }
        for (auto i : entry.properties) {
            if (!i.is_field) {
                continue;
            }
            QVariant temp = Impl::read(entry.orig_meta_type_id, i.property_index, value);
            if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                temp.convert(qMetaTypeId<QString>());
            }
            if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                QByteArray array;
                QDataStream stream(&array, QIODevice::ReadWrite);
                stream << temp;
                temp = array;
                //w.convert(qMetaTypeId<QByteArray>());
            }
            query.bindValue(QString(":") + i.field_name, temp);
        }
        query.exec();
        if (query.lastError().isValid()) {
            qDebug() << entry.orig_meta_type_id << entry.field_name;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.executedQuery();
            qDebug() << query.boundValues();
        }
        long long ormRowid = orm->get_last_inserted_rowid(query);
        query.finish();
        for (auto i : entry.properties) {
            if (i.is_field) {
                continue;
            }
            QVariant temp = Impl::read(entry.orig_meta_type_id, i.property_index, value);
            orm::Impl::for_each(i.orig_meta_type_id, temp, [&orm, &i, &entry, &value](int meta_type_id, QVariant & var, int index){
                meta_update(orm, i, var, &entry, value);
            });
        }
    }

    void ORM::ORM_Impl::meta_delete(ORM * orm, ORM::TableEntry & entry, QVariant & value, ORM::TableEntry * parent, const QVariant & parentVal)
    {
        if (!value.isValid()) {
            qDebug() << "Invalid value for delete of " << entry.field_name;
            return;
        }
        if (!entry.orig_meta_type_id || entry.is_field) {
            qDebug() << "Invalid metatype entry" << entry.orig_meta_type_id << parent;
            qDebug() << entry.field_name << entry.orig_meta_type_id << entry.norm_meta_type_id << entry.is_field << entry.is_QObject;
            qDebug() << Impl::isTypeForTable(entry.orig_meta_type_id);
            qDebug() << Impl::isPointer(entry.orig_meta_type_id)
                     << Impl::isGadget(entry.orig_meta_type_id)
                     << Impl::isSequentialContainer(entry.orig_meta_type_id)
                     << Impl::isAssociativeContainer(entry.orig_meta_type_id)
                     << Impl::isPair(entry.orig_meta_type_id);
            return;
        }
        for (auto i : entry.properties) {
            if (i.is_field) {
                continue;
            }
            QVariant temp = Impl::read(entry.orig_meta_type_id, i.property_index, value);
            orm::Impl::for_each(i.orig_meta_type_id, temp, [&orm, &i, &entry, &value](int meta_type_id, QVariant & var, int index){
                meta_delete(orm, i, var, &entry, value);
            });
        }
        QString queryText = orm->m_deleteQueries.contains(entry.field_name) ? orm->m_deleteQueries[entry.field_name] : orm->generate_delete_query(entry, parent);
        if (!orm->m_deleteQueries.contains(entry.field_name) && orm->m_bufferization) {
            orm->m_deleteQueries[entry.field_name] = queryText;
        }
        QSqlQuery query(QSqlDatabase::database(orm->m_databaseName, true));
        query.prepare(queryText);

        if (parent) {
            if (parent->has_orm_row_id) {
                for (auto i : parent->properties) {
                    if (i.is_orm_row_id) {
                        QVariant temp = Impl::read(parent->orig_meta_type_id, i.property_index, parentVal);
                        if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                            temp.convert(qMetaTypeId<QString>());
                        }
                        if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                            QByteArray array;
                            QDataStream stream(&array, QIODevice::ReadWrite);
                            stream << temp;
                            temp = array;
                            //w.convert(qMetaTypeId<QByteArray>());
                        }
                        query.bindValue(QString(":parent_") + i.field_name, temp);
                    }
                }
            }
            else if (parent->has_primary_key) {
                for (auto i : parent->properties) {
                    if (i.is_primary_key) {
                        QVariant temp = Impl::read(parent->orig_meta_type_id, i.property_index, parentVal);
                        if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                            temp.convert(qMetaTypeId<QString>());
                        }
                        if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                            QByteArray array;
                            QDataStream stream(&array, QIODevice::ReadWrite);
                            stream << temp;
                            temp = array;
                            //w.convert(qMetaTypeId<QByteArray>());
                        }
                        query.bindValue(QString(":parent_") + i.field_name, temp);
                    }
                }
            }
        }
        for (auto i : entry.properties) {
            if (!i.is_field) {
                continue;
            }
            QVariant temp = Impl::read(entry.orig_meta_type_id, i.property_index, value);
            if (Impl::isPrimitiveString(i.orig_meta_type_id)) {
                temp.convert(qMetaTypeId<QString>());
            }
            if (Impl::isPrimitiveRaw(i.orig_meta_type_id)) {
                QByteArray array;
                QDataStream stream(&array, QIODevice::ReadWrite);
                stream << temp;
                temp = array;
                //w.convert(qMetaTypeId<QByteArray>());
            }
            query.bindValue(QString(":") + i.field_name, temp);
        }
        query.exec();
        if (query.lastError().isValid()) {
            qDebug() << entry.orig_meta_type_id << entry.field_name;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.executedQuery();
            qDebug() << query.boundValues();
        }
        query.finish();
    }

    void ORM::ORM_Impl::meta_drop(ORM * orm, ORM::TableEntry & entry, ORM::TableEntry * parent)
    {
        if (!entry.orig_meta_type_id || entry.is_field) {
            qDebug() << "Invalid metatype entry" << entry.orig_meta_type_id << parent;
            return;
        }
        for (auto & i : entry.properties) {
            if (!i.is_field) {
                meta_drop(orm, i, &entry);
            }
        }
        QString queryText = orm->m_dropQueries.contains(entry.field_name)
                ? orm->m_dropQueries[entry.field_name]
                : orm->generate_drop_query(entry, parent);
        if (!orm->m_dropQueries.contains(entry.field_name) && orm->m_bufferization) {
            orm->m_dropQueries[entry.field_name] = queryText;
        }
        QSqlQuery query(QSqlDatabase::database(orm->m_databaseName, true));
        query.exec(queryText);

        if (query.lastError().isValid()) {
            qDebug() << entry.orig_meta_type_id << entry.field_name;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.executedQuery();
            qDebug() << query.boundValues();
        }
    }

}








uint qHash(const orm::Containers::ORM_QVariantPair &variantPair) Q_DECL_NOEXCEPT
{
    return qHash(variantPair.key.data());
}

QDebug operator<<(QDebug dbg, const orm::Config &)
{
    QDebugStateSaver s(dbg);
    dbg.nospace().noquote() << "ORM Config:\n";
    dbg.nospace().noquote() << "General:\n";
    dbg.nospace().noquote() << "  ROW ID field name: \"" << orm::Impl::orm_rowidName << "\"\n";
    dbg.nospace().noquote() << "  Parent foreing key field name: \"" << orm::Impl::orm_parentRowidName << "\"\n";
    dbg.nospace().noquote() << "  ORM initiated " << !orm::Impl::orm_once << "\n";

    dbg.nospace().noquote() << "Pair types:\n";
    for (auto i : orm::Impl::pairTypes) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    dbg << "Associative container types:\n" ;
    for (auto i = orm::Impl::assContTypeMap.begin(); i != orm::Impl::assContTypeMap.end(); ++i) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i.key()) << " (" << i.key() << ") -> <" <<
                                   QMetaType::typeName(i.value().first) << " (" << i.value().first << "), " <<
                                   QMetaType::typeName(i.value().second) << " (" << i.value().second << ")>\n";
    }
    //    static QMap<int, orm::pointers::ORMPointerStub> pointerMap;
    dbg.nospace().noquote() << "Registred pointers:\n";
    for (auto i = orm::Impl::pointerMap.begin(); i != orm::Impl::pointerMap.end(); ++i) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i.key()) << " (" << i.key() << ") : "
                                                                                              "Raw[" << i.value().pT << "], "
                                                                                                                        "QShared[" << i.value().ST << "], "
                                                                                                                                                      "QWeak[" << i.value().WT << "], "
                                                                                                                                                                                  "std::shared[" << i.value().sT << "], "
                                                                                                                                                                                                                    "std::weak[" << i.value().wT << "]\n";
    }

    dbg.nospace().noquote() << "Ignored types:\n";
    for (auto i : orm::Impl::ignoredTypes) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    dbg.nospace().noquote() << "Primitive types (auto):\n";
    for (auto i : orm::Impl::primitiveContainers) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    dbg.nospace().noquote() << "Primitive types (as string):\n";
    for (auto i : orm::Impl::primitiveStringContainers) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    dbg.nospace().noquote() << "Primitive types (as blob):\n";
    for (auto i : orm::Impl::primitiveRawContainers) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    return dbg;


}

QDebug operator<<(QDebug dbg, const orm::Containers::ORM_QVariantPair & pair)
{
    return dbg << "Pair(" << pair.key << ", " << pair.value << ")";
}

QDebug operator<< (QDebug dbg, QList<orm::Containers::ORM_QVariantPair> const& pair)
{
    for (int i = 0; i < pair.size(); ++i) dbg << pair[i];
    return dbg;
}
