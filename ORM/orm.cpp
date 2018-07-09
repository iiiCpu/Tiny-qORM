/*********************************************
 *          made by iCpu with love           *
 * *******************************************/

#include "ORM.h"

#include <QDebug>
#include <QMetaProperty>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QRegularExpression>



static QMap<int, QPair<int, int>> pairTypeMap;
static QMap<int, QPair<int, int>> assContTypeMap;
static QMap<int, orm_qobjects::creators> objectMap;
static QMap<int, orm_pointers::ORMPointerStub> pointerMap;
static QSet<int> primitiveContainers;
static bool orm_once = true;

void orm_qobjects::addQObjectStub(int type, orm_qobjects::creators stub)
{
    if (!type) qWarning() << "registerQObject: Could not register type";
    if (type) objectMap[type] = stub;
}

void ORM::addPointerStub(const orm_pointers::ORMPointerStub & stub)
{
    if (stub. T) pointerMap[stub. T] = stub;
    if (stub.pT) pointerMap[stub.pT] = stub;
    if (stub.ST) pointerMap[stub.ST] = stub;
    if (stub.WT) pointerMap[stub.WT] = stub;
    if (stub.sT) pointerMap[stub.sT] = stub;
    if (stub.wT) pointerMap[stub.wT] = stub;
}

void ORM::addPairType(int firstType, int secondType, int pairType)
{
    if (!pairTypeMap.contains(pairType)) {
        pairTypeMap[pairType] = QPair<int, int>(firstType, secondType);
    }
}
void ORM::addContainerPairType(int firstType, int secondType, int pairType)
{
    if (!assContTypeMap.contains(pairType)) {
        assContTypeMap[pairType] = QPair<int, int>(firstType, secondType);
    }
}

void ORM::addPrimitiveType(int metaclassid)
{
    primitiveContainers << metaclassid;
}

void ORM::removePrimitiveType(int metaclassid)
{
    primitiveContainers.remove(metaclassid);
}

void registerPrimitiveTypeContainers()
{
    orm_containers::registerPrimitiveTypeContainer<QList>();
    orm_containers::registerPrimitiveTypeContainer<QVector>();
}





bool isEnumeration(int metatypeid) {
    QMetaType metatype(metatypeid);
    if (metatype.flags() & QMetaType::IsEnumeration) {
        return true;
    }
    return false;
}

bool isGadget(int metatypeid) {
    QMetaType metatype(metatypeid);
    if (metatype.flags() & QMetaType::IsGadget) {
        return true;
    }
    return false;
}

bool isPointer(int metatypeid) {
    QMetaType metatype(metatypeid);
    if (metatype.flags() & QMetaType::PointerToGadget || metatype.flags() & QMetaType::PointerToQObject) {
        return true;
    }
    if (pointerMap.contains(metatypeid) && pointerMap[metatypeid].T != metatypeid) {
        return true;
    }
    return false;
}bool isWeakPointer(int metatypeid) {
    if (pointerMap.contains(metatypeid) && (pointerMap[metatypeid].WT == metatypeid || pointerMap[metatypeid].wT == metatypeid)) {
        return true;
    }
    return false;
}

int typeToValue(int metatypeid) {
    if (pointerMap.contains(metatypeid)) {
        return pointerMap[metatypeid].T;
    }
    return 0;
}

bool isPrimitive(int metatypeid) {
    return primitiveContainers.contains(metatypeid);
}

bool isSequentialContainer(int metatypeid)
{
    return QMetaType::hasRegisteredConverterFunction(metatypeid, qMetaTypeId<QtMetaTypePrivate::QSequentialIterableImpl>());
}

bool isPair(int metatypeid)
{
    return pairTypeMap.contains(metatypeid);
}
bool isAssociativeContainer(int metatypeid)
{
    return assContTypeMap.contains(metatypeid);
}

int getSequentialContainerStoredType(int metatypeid)
{
    return (*(QVariant((QVariant::Type)metatypeid).value<QSequentialIterable>()).end()).userType();
}

int getAssociativeContainerStoredKeyType(int metatypeid)
{
    if (!pairTypeMap.contains(metatypeid)) return 0;
    return pairTypeMap[metatypeid].first;
}

int getAssociativeContainerStoredValueType(int metatypeid)
{
    if (!pairTypeMap.contains(metatypeid)) return 0;
    return pairTypeMap[metatypeid].second;
}





namespace q
{
    inline bool isQObject(int metatypeid)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(metatypeid);
        if (meta) {
            return meta->inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
        }
        return false;
    }
    inline bool isQObject(QMetaObject const& meta)
    {
        return meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    }

    inline bool isTypeForTable(int metatypeid)
    {
        if (isPrimitive(metatypeid)) {
            return false;
        }
        return  isGadget(metatypeid) ||
                isSequentialContainer(metatypeid) ||
                isAssociativeContainer(metatypeid);
    }

    inline bool shouldSkipReadMeta(QMetaProperty const& property, QObject* obj = nullptr)
    {
        return !property.isReadable() || !property.isStored(obj);
    }

    inline bool shouldSkipWriteMeta(QMetaProperty const& property, QObject* obj = nullptr)
    {
        return !property.isWritable() || !property.isStored(obj);
    }
    inline bool shouldSkipMeta(QMetaProperty const& property, QObject* obj = nullptr)
    {
        return shouldSkipReadMeta(property, obj) || shouldSkipWriteMeta(property, obj);
    }

    inline bool isRowID(const QMetaProperty & prop)
    {
        return QString("orm_rowid") == QString(prop.name());
    }
    inline bool withRowid(const QMetaObject &meta)
    {
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (shouldSkipMeta(p)) {
                continue;
            }
            if (isRowID(p)) {
                return true;
            }
        }
        return false;
    }


}

namespace h
{
    inline bool checkQueryErrors(QSqlQuery & query, QString const& meta, QString myName, QString property_name = QString())
    {
        if (query.lastError().isValid()) {
            qDebug() << meta << myName;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.lastQuery();
            qDebug() << query.boundValues();
            return true;
        }
        return false;
    }

    inline bool write(bool isQObject, QVariant & writeInto, QMetaProperty property, QVariant const& value)
    {
        if (isQObject) {
            QObject * o = writeInto.value<QObject*>();
            if (q::shouldSkipWriteMeta(property, o)) {
                return false;
            }
            return property.write(writeInto.value<QObject*>(), value);
        }
        else {
            return property.writeOnGadget(writeInto.value<void*>(), value);
        }
    }
    inline bool writeData(bool isQObject, QVariant & writeInto, QMetaProperty property, QVariant const& value)
    {
        if (isQObject) {
            QObject * o = writeInto.value<QObject*>();
            if (q::shouldSkipWriteMeta(property, o)) {
                return false;
            }
            return property.write(writeInto.value<QObject*>(), value);
        }
        else {
            return property.writeOnGadget(writeInto.data(), value);
        }
    }

    inline QVariant read(bool isQObject, QVariant const& readFrom, QMetaProperty property)
    {
        if (isQObject) {
            QObject * o = readFrom.value<QObject*>();
            if (q::shouldSkipReadMeta(property, o)) {
                return QVariant();
            }
            return property.read(o);
        }
        else {
            return property.readOnGadget(readFrom.value<void*>());
        }
    }
}







QString ORM::databaseName() const
{
    return m_databaseName;
}

void ORM::setDatabaseName(const QString & databaseName)
{
    if (m_databaseName != databaseName) {
        m_databaseName = databaseName;
        insertQueries.clear();
        updateQueries.clear();
        selectQueries.clear();
    }
}

void ORM::create(int metatypeid)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_create(*o);
    }
}

QVariant ORM::select(int metatypeid)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
       return  meta_select(*o);
    }
    return QVariant();
}

void ORM::insert(int metatypeid, QVariant &v)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_insert(*o, v);
    }
}

void ORM::delet(int metatypeid, QVariant &v)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_delete(*o, v);
    }
}

void ORM::update(int metatypeid, QVariant &v)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_update(*o, v);
    }
}

void ORM::drop(int metatypeid)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_drop(*o);
    }
}

QString fromCamelCase(const QString & s)
{
    static QRegularExpression regExp1 {"(.)([A-Z]+)"};
    static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

    QString result = s;
    result.replace(regExp1, "\\1_\\2");
    result.replace(regExp2, "\\1_\\2");

    return result.toLower();
}

QString ORM::normalize(const QString & str) const
{
    static QRegularExpression regExp1 {"[:;,.<>]+"};
    return "_" + fromCamelCase(str).replace(regExp1, "_");
}






ORM::ORM(const QString & dbname)
    : m_databaseName(QSqlDatabase::defaultConnection)
{
    if (!dbname.isEmpty()) {
        m_databaseName = dbname;
    }
    if (orm_once) {
        orm_once = false;
        registerPrimitiveTypeContainers();
        registerTypeORM<ORMQVariantPair>();
        registerTypeORM<ORMPairStub>();
        registerTypeORM<ORMHashMapStub>();
        addPrimitiveType<QPalette>();
        addPrimitiveType<QFont>();
        addPrimitiveType<QLocale>();
        addPrimitiveType<QSizePolicy>();
    }
}

ORM::~ORM()
{
    //static QMap<int, orm_qobjects::creators> objectMap;
    //static QMap<int, orm_pointers::ORMPointerStub> pointerMap;
    //static QSet<int> primitiveContainers;

}


QString ORM::TYPE(int type_id) const {
    if (primitiveContainers.contains(type_id)) {
        return "TEXT";
    }
    switch(type_id) {
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Double:
    case QMetaType::Long:
    case QMetaType::Short:
    case QMetaType::Char:
    case QMetaType::ULong:
    case QMetaType::UShort:
    case QMetaType::UChar:
    case QMetaType::Float:
        return "NUMERIC";
    case QMetaType::QChar:
    case QMetaType::QString:
    case QMetaType::QDate:
    case QMetaType::QTime:
    case QMetaType::QDateTime:
    case QMetaType::QUrl:
        return "TEXT";
    case QMetaType::UnknownType:
    default:
        return "BLOB";
    }
}

void ORM::meta_create_pair(int metaclass, QString const& parent_name)
{
    QString className = QMetaType::typeName(metaclass);
    QString myName = parent_name + normalize(className);
    QString query_text = QString("CREATE TABLE IF NOT EXISTS %1 (").arg(myName);
    QStringList query_columns;
    query_text += "_orm_rowid INTEGER PRIMARY KEY, property_name TEXT, parent_orm_rowid INTEGER ";

    int keyType = assContTypeMap[metaclass].first;
    int valueType = assContTypeMap[metaclass].second;
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (isPrimitive(i)) {
            query_columns << normalize(name) + " " + TYPE(i);
            continue;
        }
        if (isGadget(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(i);
            if (o) {
                meta_create(*o, myName);
                continue;
            }
        }
        if (isPointer(i)) {
            if (!isWeakPointer(i)) {
                const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(i));
                if (o) {
                    meta_create(*o, myName);
                    continue;
                }
            }
            continue;
        }
        if (isSequentialContainer(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(i));
            if (o) {
                meta_create(*o, myName);
                continue;
            }
        }
        if (isAssociativeContainer(i) || isPair(i)) {
            meta_create_pair(i, myName);
        }
        query_columns << normalize(name) + " " + TYPE(i);
    }
    if (query_columns.size()) {
        query_text += ", " + query_columns.join(", ");
    }
    query_text += ");";
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(query_text);
    h::checkQueryErrors(query, "", myName);
}
QVariant ORM::meta_select_pair(int usermetatype, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(usermetatype);
    QString myName = parent_name + normalize(className);
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    if (!selectQueries.contains(myName)) {
        QStringList query_columns;
        QString query_text = "SELECT _orm_rowid, property_name, parent_orm_rowid ";
        for (int j = 0; j < 2; ++j) {
            int i = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (!isPrimitive(i) && (isGadget(i) || isPointer(i) ||
                   isSequentialContainer(i) || isAssociativeContainer(i))) {
                continue;
            }
            query_columns << normalize(name);
        }
        if (query_columns.size()) {
            query_text += " , " + query_columns.join(", ");
        }
        query_text += " FROM " + myName;
        if (parent_orm_rowid) {
            query_text += " WHERE parent_orm_rowid = :parent_orm_rowid AND property_name = :property_name;";
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(query_text);
        selectQueries[myName] = query;
    }
    QSqlQuery & query = selectQueries[myName];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    query.exec();
    h::checkQueryErrors(query, className, myName, property_name);
    QVariantList list;
    while (query.next()) {
        ORMQVariantPair pair;
        pair.m_orm_rowid = query.value("_orm_rowid").toLongLong();
        for (int j = 0; j < 2; ++j)
        {
            int i = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (!isPrimitive(i)) {
                if (isGadget(i)) {
                    const QMetaObject * o = QMetaType::metaObjectForType(i);
                    if (o) {
                        QVariantList vl = meta_select(*o, myName, name, pair.m_orm_rowid).toList();
                        if (vl.size()) {
                            pair[j] = vl.first();
                        }
                        continue;
                    }
                }
                if (isPointer(i)) {
                    if (isWeakPointer(i)) {
                        continue;
                    }
                    const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(i));
                    if (o) {
                        QVariantList vl = meta_select(*o, myName, name, pair.m_orm_rowid).toList();
                        if (vl.size()) {
                            if (vl.first().value<void*>()) {
                                pair[j] = vl.first();
                            }
                        }
                        continue;
                    }
                }
                if (isSequentialContainer(i)) {
                    const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(i));
                    if (o) {
                        pair[j] = meta_select(*o, myName, name, pair.m_orm_rowid).toList();
                        continue;
                    }
                }
                if (isAssociativeContainer(i)) {
                    QVariant qv = meta_select_pair(i, myName, name, pair.m_orm_rowid);
                    if (qv.canConvert(i)) {
                        qv.convert(i);
                        pair[j] = qv;
                    }
                    continue;
                }
            }
            QVariant vv = query.value(normalize(name));
            if (QMetaType::typeFlags(i).testFlag(QMetaType::IsEnumeration)) {
                vv.convert(i);
            }
            pair[j] = vv;
        }
        list << QVariant::fromValue(pair);
    }
    return QVariant(list);
}
void ORM::meta_insert_pair(int usermetatype, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(usermetatype);
    QString myName = parent_name + normalize(className);
    ORMQVariantPair ops = v.value<ORMQVariantPair>();
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    if (!insertQueries.contains(myName)) {
        QStringList names, values;
        names << "parent_orm_rowid" << "property_name";
        values <<":parent_orm_rowid" << ":property_name";
        for (int j = 0; j < 2; ++j) {
            int i = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (!isPrimitive(i) && (isGadget(i) || isPointer(i) || isSequentialContainer(i) || isAssociativeContainer(i))) {
                continue;
            }
            names << normalize(name);
            values << ":" + normalize(name);
        }
        QString query_text = QString("INSERT INTO %1 (%2) VALUES (%3);") //ON CONFLICT DO UPDATE SET %4 WHERE _orm_rowid = :_orm_rowid
                .arg(myName).arg(names.join(", ")).arg(values.join(", "));//.arg(str.join(", "));
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(query_text);
        insertQueries[myName] = query;
    }
    QSqlQuery & query = insertQueries[myName];
    query.bindValue(":parent_orm_rowid", parent_orm_rowid);
    query.bindValue(":property_name", property_name);
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (!isPrimitive(i) && (isGadget(i) || isPointer(i) || isSequentialContainer(i) || isAssociativeContainer(i))) {
            continue;
        }
        query.bindValue(QString(":") + normalize(name), ops[j]);
    }
    query.exec();
    if (h::checkQueryErrors(query, className, myName, property_name)) {
        return;
    }

    QSqlQuery query2(QSqlDatabase::database(m_databaseName, true));
    query2.exec("select last_insert_rowid() as rowid;");
    if (h::checkQueryErrors(query2, className, myName, property_name)) {
        return;
    }
    if (query2.next()) {
        ops.m_orm_rowid = query2.value("rowid").toLongLong();
    }
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (isPrimitive(i)) {
            continue;
        }
        if (isGadget(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(i);
            if (o) {
                meta_insert(*o, ops[j], myName, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (isPointer(i)) {
            if (isWeakPointer(i)) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(i));
            if (o) {
                QVariant vvv = ops[j];
                if (vvv.value<void*>()) {
                    vvv.convert(qMetaTypeId<void*>());
                    meta_insert(*o, vvv, myName, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
        if (isSequentialContainer(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(i));
            if (o) {
                QVariant qv = ops[j];
                if (qv.isValid()) {
                    QSequentialIterable si = qv.value<QSequentialIterable>();
                    for (QVariant vv : si) {
                        meta_insert(*o, vv, myName, name, ops.m_orm_rowid);
                    }
                }
                continue;
            }
        }
        if (isAssociativeContainer(i)) {
            QVariant qv = ops[j];
            if (qv.canConvert<ORMQVariantPair>()) {
                qv.convert(qMetaTypeId<ORMQVariantPair>());
                meta_insert_pair(i, qv, myName, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<ORMQVariantPair>>()) {
                QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                for (ORMQVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_insert_pair(i, vv, myName, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
}
void ORM::meta_update_pair(int usermetatype, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(usermetatype);
    QString myName = parent_name + normalize(className);
    ORMQVariantPair ops = v.value<ORMQVariantPair>();
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    if (!updateQueries.contains(myName)) {
        QString query_text = QString("UPDATE OR IGNORE %1 SET ").arg(myName);
        QStringList sets;
        for (int j = 0; j < 2; ++j) {
            int i = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (!isPrimitive(i) && (isGadget(i) || isPointer(i) || isSequentialContainer(i) || isAssociativeContainer(i))) {
                continue;
            }
            sets << normalize(name) + " = :" + name;
        }
        query_text += sets.join(',') + " WHERE _orm_rowid = :orm_rowid ";
        if (parent_orm_rowid) {
            query_text += " AND parent_orm_rowid = :parent_orm_rowid AND property_name = :property_name ";
        }
        query_text += ";";
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(query_text);
        updateQueries[myName] = query;
    }
    QSqlQuery & query = updateQueries[myName];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (!isPrimitive(i) && (isGadget(i) || isPointer(i) || isSequentialContainer(i) || isAssociativeContainer(i))) {
            continue;
        }
        query.bindValue(QString(":") + name, ops[j]);
    }
    query.exec();
    if (h::checkQueryErrors(query, className, myName, property_name)) {
        return;
    }

    QSqlQuery query2(QSqlDatabase::database(m_databaseName, true));
    query2.exec("select changes() as ch;");
    if (h::checkQueryErrors(query2, className, myName, property_name)) {
        return;
    }

    if (query2.next() && false) {
        if (query2.value("ch").toLongLong() == 0) {
            meta_insert_pair(usermetatype, v, parent_name, property_name, parent_orm_rowid);
            return;
        }
    }

    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (isPrimitive(i)) {
            continue;
        }
        if (isGadget(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(i);
            if (o) {
                QVariant vv = ops[j];
                meta_update(*o, vv, myName, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (isPointer(i)) {
            if (isWeakPointer(i)) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(i));
            if (o) {
                QVariant vvv = ops[j];
                if (vvv.value<void*>()) {
                    vvv.convert(qMetaTypeId<void*>());
                    meta_update(*o, vvv, myName, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
        if (isSequentialContainer(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(i));
            if (o) {
                QVariant qv = ops[j];
                if (qv.isValid()) {
                    QSequentialIterable si = qv.value<QSequentialIterable>();
                    for (QVariant v : si) {
                        meta_update(*o, v, myName, name, ops.m_orm_rowid);
                    }
                }
                continue;
            }
        }
        if (isAssociativeContainer(i)) {
            QVariant qv = ops[j];
            if (qv.canConvert<ORMQVariantPair>()) {
                qv.convert(qMetaTypeId<ORMQVariantPair>());
                meta_update_pair(i, qv, myName, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<ORMQVariantPair>>()) {
                QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                for (ORMQVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_update_pair(i, vv, myName, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
}
void ORM::meta_delete_pair(int usermetatype, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(usermetatype);
    QString myName = parent_name + normalize(className);
    ORMQVariantPair ops = v.value<ORMQVariantPair>();
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (isPrimitive(i)) {
            continue;
        }
        if (isGadget(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(i);
            if (o) {
                meta_delete(*o, ops[j], myName, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (isPointer(i)) {
            if (isWeakPointer(i)) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(i));
            if (o) {
                meta_delete(*o, ops[j], myName, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (isSequentialContainer(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(i));
            if (o) {
                QSequentialIterable si = ops[j].value<QSequentialIterable>();
                for (QVariant v : si) {
                    meta_delete(*o, v, myName, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
        if (isAssociativeContainer(i)) {
            QVariant qv = ops[j];
            if (qv.canConvert<ORMQVariantPair>()) {
                qv.convert(qMetaTypeId<ORMQVariantPair>());
                meta_delete_pair(i, qv, myName, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<ORMQVariantPair>>()) {
                QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                for (ORMQVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_delete_pair(i, vv, myName, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    if (!parent_orm_rowid) {
        QString query_text = QString("DELETE FROM %1 WHERE _orm_rowid = :orm_rowid;").arg(myName);
        query.prepare(query_text);
        query.bindValue(":orm_rowid", ops.m_orm_rowid);
    }
    else {
        QString query_text = QString("DELETE FROM %1 WHERE parent_orm_rowid = :parent_orm_rowid AND property_name = :property_name;").arg(myName);
        query.prepare(query_text);
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    query.exec();
    h::checkQueryErrors(query, className, myName, property_name);
}

void ORM::meta_drop_pair(int usermetatype, QString const& parent_name)
{
    QString className = QMetaType::typeName(usermetatype);
    QString myName = parent_name + normalize(className);
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    QString query_text = QString("DROP TABLE IF EXISTS %1;").arg(myName);
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        if (isPrimitive(i)) {
            continue;
        }
        const QMetaObject * o = nullptr;
        if (isGadget(i)) {
            o = QMetaType::metaObjectForType(i);
        }
        if (isPointer(i)) {
            if (isWeakPointer(i)) {
                continue;
            }
           o = QMetaType::metaObjectForType(typeToValue(i));
        }
        if (isSequentialContainer(i)) {
            o = QMetaType::metaObjectForType(getSequentialContainerStoredType(i));
        }
        if (isAssociativeContainer(i)) {
            meta_drop_pair(usermetatype, myName);
            continue;
        }
        if (o) {
            meta_drop(*o, myName);
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(query_text);
    h::checkQueryErrors(query, className, myName);
}

void ORM::meta_create(const QMetaObject &meta, QString const& parent_name)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    QString query_text = QString("CREATE TABLE IF NOT EXISTS %1 (").arg(myName);
    QStringList query_columns;
    bool with_orm_rowid = q::withRowid(meta);
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!p.isStored() || !p.isWritable() || q::isRowID(p)) {
            continue;
        }
        if (isPrimitive(p.userType())) {
            query_columns << normalize(p.name()) + " " + TYPE(p.type());
            continue;
        }
        if (isGadget(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using structure fields without using orm_rowid is not supported. Type " << meta.className() << " property " << p.name();
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
            if (o) {
                meta_create(*o, myName);
                continue;
            }
        }
        if (isPointer(p.userType())) {
            if (!isWeakPointer(p.userType())) {
                if (!with_orm_rowid) {
                    qWarning() << "create: Using pointer fields without using orm_rowid is not supported. Type " << meta.className() << " property " << p.name();
                    continue;
                }
                const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
                if (o) {
                    meta_create(*o, myName);
                    continue;
                }
            }
            continue;
        }
        if (isSequentialContainer(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using containers without using orm_rowid is not supported. Type " << meta.className() << " property " << p.name();
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
            if (o) {
                meta_create(*o, myName);
                continue;
            }
        }
        if (isAssociativeContainer(p.userType()) || isPair(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using containers without using orm_rowid is not supported. Type " << meta.className() << " property " << p.name();
                continue;
            }
            meta_create_pair(p.userType(), myName);
            continue;
        }
        query_columns << normalize(p.name()) + " " + TYPE(p.type());
    }
    if (with_orm_rowid) {
        query_text += "_orm_rowid INTEGER PRIMARY KEY, ";
    }
    query_text += " property_name TEXT, parent_orm_rowid INTEGER ";
    if (query_columns.size()) {
        query_text += ", " + query_columns.join(", ");
    }
    query_text += ");";
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(query_text);
    h::checkQueryErrors(query, meta.className(), myName);
}
QVariant ORM::meta_select(const QMetaObject &meta, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = q::isQObject(meta);
    bool with_orm_rowid = q::withRowid(meta);
    if (!selectQueries.contains(myName)) {
        QStringList query_columns;
        QString query_text = "SELECT property_name, parent_orm_rowid ";
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (!isPrimitive(p.userType()) && (!p.isStored() || isGadget(p.userType()) || isPointer(p.userType()) ||
                   isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
                continue;
            }
            query_columns << normalize(p.name());
        }
        if (!with_orm_rowid && !query_columns.size()) {
            return QVariant();
        }
        if (query_columns.size()) {
            query_text += " , " + query_columns.join(", ");
        }
        query_text += " FROM " + myName;
        if (parent_orm_rowid) {
            query_text += " WHERE parent_orm_rowid = :parent_orm_rowid AND property_name = :property_name ";
        }
        query_text += ";";
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(query_text);
        selectQueries[myName] = query;
    }
    QSqlQuery & query = selectQueries[myName];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    query.exec();
    h::checkQueryErrors(query, meta.className(), myName, property_name);
    QVariantList list;
    while (query.next()) {
        QVariant v;
        if (isQObject) {
            if (!objectMap.contains(classtype)) {
                qWarning() << "select: " << meta.className() << " is not registered. Call registerQObjectORM<" << meta.className() << ">(); first";
                break;
            }
            v = QVariant::fromValue(objectMap[classtype]());
        }
        else {
            v = QVariant((classtype), nullptr);
        }
        if (!v.isValid()){
            qWarning() << "select: Unable to create instance of type " << meta.className();
            break;
        }
        if (isQObject && v.value<QObject*>() == nullptr) {
            qWarning() << "select: Unable to create instance of QObject " << meta.className();
            break;
        }
        long long orm_rowid = 0;
        if (with_orm_rowid) {
            orm_rowid = query.value("_orm_rowid").toLongLong();
        }
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (q::shouldSkipWriteMeta(p)) {
                continue;
            }
            if (with_orm_rowid) {
                if (!isPrimitive(p.userType())) {
                    if (isGadget(p.userType())) {
                        const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
                        if (o) {
                            QVariantList vl = meta_select(*o, myName, QString(p.name()), orm_rowid).toList();
                            if (vl.size()) {
                                h::write(isQObject, v, p, vl.first());
                            }
                            continue;
                        }
                    }
                    if (isPointer(p.userType())) {
                        if (isWeakPointer(p.userType())) {
                            continue;
                        }
                        const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
                        if (o) {
                            QVariantList vl = meta_select(*o, myName, QString(p.name()), orm_rowid).toList();
                            if (vl.size()) {
                                if (vl.first().value<void*>()) {
                                    h::write(isQObject, v, p, vl.first());
                                }
                            }
                            continue;
                        }
                    }
                    if (isSequentialContainer(p.userType())) {
                        const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
                        if (o) {
                            h::write(isQObject, v, p, meta_select(*o, myName, QString(p.name()), orm_rowid).toList());
                            continue;
                        }
                    }
                    if (isAssociativeContainer(p.userType())) {
                        QVariant qv = meta_select_pair(p.userType(), myName, QString(p.name()), orm_rowid);
                        if (qv.canConvert(p.userType())) {
                            qv.convert(p.userType());
                            h::write(isQObject, v, p, qv);
                        }
                        continue;
                    }
                }
            }
            QVariant vv = query.value(normalize(p.name()));
            if (p.isEnumType()) {
                vv.convert(p.userType());
            }
            if (with_orm_rowid) {
                h::write(isQObject, v, p, vv);
            }
            else {
                h::writeData(isQObject, v, p, vv);
            }
        }
        list << v;
    }
    return QVariant(list);
}

void ORM::meta_insert(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    long long orm_rowid = 0;
    bool with_orm_rowid = q::withRowid(meta);
    if (!insertQueries.contains(myName)) {
        QStringList names, values;
        if (parent_orm_rowid) {
            names << "parent_orm_rowid" << "property_name";
            values << ":parent_orm_rowid" << ":property_name";
        }
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (!isPrimitive(p.userType()) && (!p.isStored() || !p.isReadable()  || isGadget(p.userType()) || isPointer(p.userType()) ||
                   isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
                continue;
            }
            if (q::isRowID(p)) {
                orm_rowid = h::read(isQObject, v, p).toLongLong();
                if (orm_rowid == 0 || orm_rowid == -1) {
                    continue;
                }
            }
            names << normalize(p.name());
            values << ":" + normalize(p.name());
        }
        QStringList str;
        for (int i = 0; i < names.size(); ++i) {
            //str << names[i] + " = " + values[i];
        }
        if (names.isEmpty()) {
            return; // TODO: replace with actual counter
        }
        QString query_text = QString("INSERT INTO %1 (%2) VALUES (%3);") //ON CONFLICT DO UPDATE SET %4 WHERE _orm_rowid = :_orm_rowid
                .arg(myName).arg(names.join(", ")).arg(values.join(", "));//.arg(str.join(", "));
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(query_text);
        insertQueries[myName] = query;
    }
    QSqlQuery & query = insertQueries[myName];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!isPrimitive(p.userType()) && (!p.isStored()  || !p.isReadable()  || isGadget(p.userType()) || isPointer(p.userType()) ||
             isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
            continue;
        }
        if (q::isRowID(p) && (orm_rowid == 0 || orm_rowid == -1)) {
            continue;
        }
        query.bindValue(QString(":") + normalize(p.name()), h::read(isQObject, v, p));
    }
    query.exec();
    if (h::checkQueryErrors(query, meta.className(), myName, property_name)) {
        return;
    }

    if (with_orm_rowid) {
        QSqlQuery query2(QSqlDatabase::database(m_databaseName, true));
        query2.exec("select last_insert_rowid() as rowid;");
        if (h::checkQueryErrors(query2, meta.className(), myName, property_name)) {
            return;
        }
        if (query2.next()) {
            orm_rowid = query2.value("rowid").toLongLong();
        }
        if (orm_rowid) {
            for (int i = 0; i < meta.propertyCount(); ++i) {
                QMetaProperty p = meta.property(i);
                if (!p.isStored() || !p.isReadable() || isPrimitive(p.userType())) {
                    continue;
                }
                if (isGadget(p.userType())) {
                    const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
                    if (o) {
                        QVariant vv = h::read(isQObject, v, p);
                        meta_insert(*o, vv, myName, QString(p.name()), orm_rowid);
                        continue;
                    }
                }
                if (isPointer(p.userType())) {
                    if (isWeakPointer(p.userType())) {
                        continue;
                    }
                    const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
                    if (o) {
                        QVariant vvv = p.readOnGadget(v.value<void*>());
                        if (vvv.value<void*>()) {
                            vvv.convert(qMetaTypeId<void*>());
                            meta_insert(*o, vvv, myName, QString(p.name()), orm_rowid);
                        }
                        continue;
                    }
                }
                if (isSequentialContainer(p.userType())) {
                    const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
                    if (o) {
                        QVariant qv = h::read(isQObject, v, p);
                        if (qv.isValid()) {
                            QSequentialIterable si = qv.value<QSequentialIterable>();
                            for (QVariant vv : si) {
                                meta_insert(*o, vv, myName, QString(p.name()), orm_rowid);
                            }
                        }
                        continue;
                    }
                }
                if (isAssociativeContainer(p.userType())) {
                    QVariant qv = h::read(isQObject, v, p);
                    if (qv.canConvert<ORMQVariantPair>()) {
                        qv.convert(qMetaTypeId<ORMQVariantPair>());
                        meta_insert_pair(p.userType(), qv, myName, QString(p.name()), orm_rowid);
                        continue;
                    }
                    if (qv.canConvert<QList<ORMQVariantPair>>()) {
                        QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                        for (ORMQVariantPair & v : list) {
                            QVariant vv = QVariant::fromValue(v);
                            meta_insert_pair(p.userType(), vv, myName, QString(p.name()), orm_rowid);
                        }
                        continue;
                    }
                }
            }
        }
    }
}

void ORM::meta_update(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = q::isQObject(meta);
    if (!q::withRowid(meta)) {
        qWarning() << "update: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    if (!updateQueries.contains(myName)) {
        QString query_text = QString("UPDATE OR IGNORE %1 SET ").arg(myName);
        QStringList sets;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (!isPrimitive(p.userType()) && (!p.isStored() || isGadget(p.userType()) || isPointer(p.userType()) ||
                   isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
                continue;
            }
            if (p.name() == QString("_orm_rowid")) {
                continue;
            }
            sets << normalize(p.name()) + " = :" + p.name();
        }
        query_text += sets.join(',') + " WHERE _orm_rowid = :orm_rowid ";
        if (parent_orm_rowid) {
            query_text += " AND parent_orm_rowid = :parent_orm_rowid AND property_name = :property_name ";
        }
        query_text += ";";
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(query_text);
        updateQueries[myName] = query;
    }
    QSqlQuery & query = updateQueries[myName];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!isPrimitive(p.userType()) && (!p.isStored() || !p.isReadable() || isGadget(p.userType()) || isPointer(p.userType()) ||
               isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
            continue;
        }
        query.bindValue(QString(":") + p.name(), h::read(isQObject, v, p));
    }
    query.exec();
    if (h::checkQueryErrors(query, meta.className(), myName, property_name)) {
        return;
    }

    QSqlQuery query2(QSqlDatabase::database(m_databaseName, true));
    query2.exec("select changes() as ch;");
    if (h::checkQueryErrors(query2, meta.className(), myName, property_name)) {
        return;
    }

    long long orm_rowid = 0;
    if (query2.next() && false) {
        if (query2.value("ch").toLongLong() == 0) {
            meta_insert(meta, v, parent_name, property_name, parent_orm_rowid);
            return;
        }
    }

    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!p.isStored() || !p.isReadable() || isPrimitive(p.userType())) {
            continue;
        }
        if (isGadget(p.userType())) {
            const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
            if (o) {
                QVariant vv = h::read(isQObject, v, p);
                meta_update(*o, vv, myName, QString(p.name()), orm_rowid);
                continue;
            }
        }
        if (isPointer(p.userType())) {
            if (isWeakPointer(p.userType())) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
            if (o) {
                QVariant vvv = h::read(isQObject, v, p);
                if (vvv.value<void*>()) {
                    vvv.convert(qMetaTypeId<void*>());
                    meta_update(*o, vvv, myName, QString(p.name()), orm_rowid);
                }
                continue;
            }
        }
        if (isSequentialContainer(p.userType())) {
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
            if (o) {
                QVariant qv = h::read(isQObject, v, p);
                if (qv.isValid()) {
                    QSequentialIterable si = qv.value<QSequentialIterable>();
                    for (QVariant v : si) {
                        meta_update(*o, v, myName, QString(p.name()), orm_rowid);
                    }
                }
                continue;
            }
        }
        if (isAssociativeContainer(p.userType())) {
            QVariant qv = h::read(isQObject, v, p);
            if (qv.canConvert<ORMQVariantPair>()) {
                qv.convert(qMetaTypeId<ORMQVariantPair>());
                meta_update_pair(p.userType(), qv, myName, QString(p.name()), orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<ORMQVariantPair>>()) {
                QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                for (ORMQVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_update_pair(p.userType(), vv, myName, QString(p.name()), orm_rowid);
                }
                continue;
            }
        }
    }
}

void ORM::meta_delete(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = q::isQObject(meta);
    long long orm_rowid = 0;
    if (!q::withRowid(meta)) {
        qWarning() << "delete: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!isPrimitive(p.userType()) && (!p.isStored() || !p.isReadable() || isGadget(p.userType()) || isPointer(p.userType()) ||
               isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
            continue;
        }
        if (q::isRowID(p)) {
            orm_rowid = h::read(isQObject, v, p).toLongLong();
        }
    }
    if (!orm_rowid && !parent_orm_rowid) {
        return;
    }
    if (orm_rowid) {
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (!p.isStored() || !p.isReadable() || isPrimitive(p.userType())) {
                continue;
            }
            if (isGadget(p.userType())) {
                const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
                if (o) {
                    QVariant vv = h::read(isQObject, v, p);
                    meta_delete(*o, vv, myName, QString(p.name()), orm_rowid);
                    continue;
                }
            }
            if (isPointer(p.userType())) {
                if (isWeakPointer(p.userType())) {
                    continue;
                }
                const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
                if (o) {
                    QVariant vv = h::read(isQObject, v, p);
                    meta_delete(*o, vv, myName, QString(p.name()), orm_rowid);
                    continue;
                }
            }
            if (isSequentialContainer(p.userType())) {
                const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
                if (o) {
                    QSequentialIterable si = h::read(isQObject, v, p).value<QSequentialIterable>();
                    for (QVariant v : si) {
                        meta_delete(*o, v, myName, QString(p.name()), orm_rowid);
                    }
                    continue;
                }
            }
            if (isAssociativeContainer(p.userType())) {
                QVariant qv = h::read(isQObject, v, p);
                if (qv.canConvert<ORMQVariantPair>()) {
                    qv.convert(qMetaTypeId<ORMQVariantPair>());
                    meta_delete_pair(p.userType(), qv, myName, QString(p.name()), orm_rowid);
                    continue;
                }
                if (qv.canConvert<QList<ORMQVariantPair>>()) {
                    QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                    for (ORMQVariantPair & v : list) {
                        QVariant vv = QVariant::fromValue(v);
                        meta_delete_pair(p.userType(), vv, myName, QString(p.name()), orm_rowid);
                    }
                    continue;
                }
            }
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    if (!parent_orm_rowid) {
        QString query_text = QString("DELETE FROM %1 WHERE _orm_rowid = :orm_rowid;").arg(myName);
        query.prepare(query_text);
        query.bindValue(":orm_rowid", orm_rowid);
    }
    else {
        QString query_text = QString("DELETE FROM %1 WHERE parent_orm_rowid = :parent_orm_rowid AND property_name = :property_name;").arg(myName);
        query.prepare(query_text);
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    query.exec();
    h::checkQueryErrors(query, meta.className(), myName, property_name);
}

void ORM::meta_drop(const QMetaObject &meta, QString const& parent_name)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    QString query_text = QString("DROP TABLE IF EXISTS %1;").arg(myName);
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!p.isStored() || isPrimitive(p.userType())) {
            continue;
        }
        const QMetaObject * o = nullptr;
        if (isGadget(p.userType())) {
            o = QMetaType::metaObjectForType(p.userType());
        }
        if (isPointer(p.userType())) {
            if (isWeakPointer(p.userType())) {
                continue;
            }
           o = QMetaType::metaObjectForType(typeToValue(p.userType()));
        }
        if (isSequentialContainer(p.userType())) {
            o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
        }
        if (isAssociativeContainer(p.userType()) || isPair(p.userType())) {
            meta_drop_pair(p.userType(), myName);
            continue;
        }
        if (o) {
            meta_drop(*o, myName);
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(query_text);
    h::checkQueryErrors(query, meta.className(), myName);
}

uint qHash(const ORMQVariantPair &v) noexcept
{
    return qHash(v.key.data());
}
