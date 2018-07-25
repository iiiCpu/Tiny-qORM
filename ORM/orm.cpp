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
    return (*(QVariant(static_cast<QVariant::Type>(metatypeid)).value<QSequentialIterable>()).end()).userType();
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
    inline bool isRowID(const QString & prop)
    {
        return QString("orm_rowid") == prop;
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
    inline bool checkQueryErrors(QSqlQuery & query, QString const& meta, QString table_name, QString property_name = QString())
    {
        Q_UNUSED(property_name)
        if (query.lastError().isValid()) {
            qDebug() << meta << table_name;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.lastQuery();
            qDebug() << query.boundValues();
            return true;
        }
        return false;
    }
    inline bool    write(bool isQObject, QVariant      & writeInto, QMetaProperty property, QVariant const& value)
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
    inline QVariant read(bool isQObject, QVariant const&  readFrom, QMetaProperty property)
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


uint qHash(const ORMQVariantPair &v) noexcept
{
    return qHash(v.key.data());
}







QString ORM::normalize(const QString & str) const
{
    QString s = str;
    static QRegularExpression regExp1 {"(.)([A-Z]+)"};
    static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};
    static QRegularExpression regExp3 {"[:;,.<>]+"};
    return "_" + s.replace(regExp1, "\\1_\\2").replace(regExp2, "\\1_\\2").toLower().replace(regExp3, "_");
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


QString ORM::generate_table_name(const QString &parent_name, const QString &property_name, const QString &class_name) const
{
    QString table_name = parent_name + normalize(class_name);
    QString pName = normalize(property_name);
    if (pName.size()) {
        table_name += pName;
    }
    return table_name;
}
long long ORM::get_last_inserted_rowid(QSqlQuery &query) const
{
    Q_UNUSED(query)
    QSqlQuery query2(QSqlDatabase::database(m_databaseName, true));
    query2.exec("select last_insert_rowid() as rowid;");
    if (h::checkQueryErrors(query2, "", "", "")) {
        return 0;
    }
    if (query2.next()) {
        return query2.value("rowid").toLongLong();
    }
    return 0;
}
long long ORM::get_last_updated_rowid (QSqlQuery &query) const
{
    Q_UNUSED(query)
    QSqlQuery query2(QSqlDatabase::database(m_databaseName, true));
    query2.exec("select changes() as ch;");
    if (h::checkQueryErrors(query2, "", "", "")) {
        return 0;
    }
    if (query2.next()) {
        return query2.value("ch").toLongLong();
    }
    return 0;
}

QString ORM::generate_create_query(QString const& parent_name, QString const& property_name, const QString &class_name, const QStringList &names, const QList<int> &types) const
{
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    bool with_orm_rowid = false;
    for (int i = 0; i < names.count(); ++i) {
        if (q::isRowID(names[i])) {
            if (with_orm_rowid) {
                qWarning() << "multiplie orm_rowid properties in " << table_name;
            }
            with_orm_rowid = true;
            continue;
        }
    }
    QString query_text = QString("CREATE TABLE IF NOT EXISTS %1 (").arg(table_name);
    QStringList query_columns;
    for (int i = 0; i < names.count(); ++i) {
        if (q::isRowID(names[i])) {
            query_columns << normalize(names[i]) + " INTEGER PRIMARY KEY ";
            continue;
        }
        query_columns << normalize(names[i]) + " " + TYPE(types[i]);
    }
    if (parent_name != "t") {
        query_text += " parent_orm_rowid INTEGER REFERENCES "
                + parent_name + " (" + normalize("orm_rowid") + ") ";
        if (query_columns.size()) {
            query_text += ", ";
        }
    }
    if (query_columns.size()) {
        query_text += query_columns.join(", ");
    }
    if (with_orm_rowid) {
        //query_text += ", UNIQUE (_orm_rowid) ON CONFLICT REPLACE ";
    }
    query_text += ");";
    return query_text;
}
QString ORM::generate_select_query(QString const& parent_name, QString const& property_name, const QString &class_name, const QStringList &names, const QList<int> &types, bool parent_orm_rowid) const
{
    Q_UNUSED(types)
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    bool with_orm_rowid = false;
    for (int i = 0; i < names.count(); ++i) {
        if (q::isRowID(names[i])) {
            if (with_orm_rowid) {
                qWarning() << "multiplie orm_rowid properties in " << table_name;
            }
            with_orm_rowid = true;
            continue;
        }
    }
    QStringList query_columns;
    QString query_text = "SELECT ";
    for (int i = 0; i < names.size(); ++i) {
        query_columns << normalize(names[i]);
    }
    if (parent_name != "t") {
        query_text += " parent_orm_rowid ";
        if (query_columns.size()) {
            query_text += " , ";
        }
    }
    if (query_columns.size()) {
        query_text += query_columns.join(", ");
    }
    query_text += " FROM " + table_name;
    if (parent_orm_rowid) {
        query_text += " WHERE parent_orm_rowid = :parent_orm_rowid ";
    }
    query_text += ";";
    return query_text;
}
QString ORM::generate_insert_query(QString const& parent_name, QString const& property_name, const QString &class_name, const QStringList &names, const QList<int> &types, bool parent_orm_rowid) const
{
    Q_UNUSED(types)
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    QStringList t_names, t_values;
    if (parent_orm_rowid) {
        t_names << "parent_orm_rowid";
        t_values << ":parent_orm_rowid";
    }
    for (int i = 0; i < names.size(); ++i) {
        t_names << normalize(names[i]);
        t_values << ":" + names[i]; // let it be like this
    }
    QStringList str;
    for (int i = 0; i < names.size(); ++i) {
        //str << names[i] + " = " + values[i];
    }
    return QString("INSERT INTO %1 (%2) VALUES (%3);") //ON CONFLICT DO UPDATE SET %4 WHERE _orm_rowid = :_orm_rowid
            .arg(table_name).arg(t_names.join(", ")).arg(t_values.join(", "));//.arg(str.join(", "));
}
QString ORM::generate_update_query(QString const& parent_name, QString const& property_name, const QString &class_name, const QStringList &names, const QList<int> &types, bool parent_orm_rowid) const
{
    Q_UNUSED(types)
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    QString query_text = QString("UPDATE OR IGNORE %1 SET ").arg(table_name);
    QStringList sets;
    for (int i = 0; i < names.size(); ++i) {
        sets << normalize(names[i]) + " = :" + names[i];
    }
    query_text += sets.join(',') + " WHERE _orm_rowid = :orm_rowid ";
    if (parent_orm_rowid) {
        query_text += " AND parent_orm_rowid = :parent_orm_rowid ";
    }
    query_text += ";";
    return query_text;
}
QString ORM::generate_delete_query(QString const& parent_name, QString const& property_name, const QString &class_name, bool parent_orm_rowid) const
{
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    if (!parent_orm_rowid) {
        return QString("DELETE FROM %1 WHERE _orm_rowid = :orm_rowid;").arg(table_name);
    }
    else {
        return QString("DELETE FROM %1 WHERE parent_orm_rowid = :parent_orm_rowid;").arg(table_name);
    }
}
QString ORM::generate_drop_query  (QString const& parent_name, QString const& property_name, const QString &class_name) const
{
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    return QString("DROP TABLE IF EXISTS %1;").arg(table_name);
}




ORM::ORM(const QString & dbname) : m_databaseName(QSqlDatabase::defaultConnection)
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

void     ORM::create(int metatypeid)
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
void     ORM::insert(int metatypeid, QVariant &v)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_insert(*o, v);
    }
}
void     ORM::delet (int metatypeid, QVariant &v)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_delete(*o, v);
    }
}
void     ORM::update(int metatypeid, QVariant &v)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_update(*o, v);
    }
}
void     ORM::drop  (int metatypeid)
{
    const QMetaObject * o = QMetaType::metaObjectForType(metatypeid);
    if (o) {
        meta_drop(*o);
    }
}

void     ORM::meta_create(const QMetaObject &meta, QString const& parent_name, QString const& property_name)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    bool with_orm_rowid = false;
    QStringList query_columns;
    QList<int> query_columns_types;
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!p.isStored() || !p.isWritable() || !p.isReadable()) {
            continue;
        }
        if (q::isRowID(p.name())) {
            if (with_orm_rowid) {
                qWarning() << "multiplie orm_rowid properties in " << meta.className();
            }
            with_orm_rowid = true;
        }
        if (isPrimitive(p.userType())) {
            query_columns << p.name();
            query_columns_types << p.userType();
            continue;
        }
        if (isGadget(p.userType()) || isPointer(p.userType()) || isSequentialContainer(p.userType()) ||
                isAssociativeContainer(p.userType()) || isPair(p.userType())) {
            continue;
        }
        query_columns << p.name();
        query_columns_types << p.userType();
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_create_query(parent_name, property_name, QString(meta.className()), query_columns, query_columns_types));
    h::checkQueryErrors(query, meta.className(), table_name);

    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (isPrimitive(p.userType())) {
            continue;
        }
        if (isGadget(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using structure fields without using orm_rowid is not supported. "
                              "Type " << table_name << " property " << p.name();
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
            if (o) {
                meta_create(*o, table_name, p.name());
                continue;
            }
        }
        if (isPointer(p.userType())) {
            if (!isWeakPointer(p.userType())) {
                if (!with_orm_rowid) {
                    qWarning() << "create: Using pointer fields without using orm_rowid is not supported. "
                                  "Type " << table_name << " property " << p.name();
                    continue;
                }
                const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
                if (o) {
                    meta_create(*o, table_name, p.name());
                    continue;
                }
            }
            continue;
        }
        if (isSequentialContainer(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using containers without using orm_rowid is not supported. "
                              "Type " << table_name << " property " << p.name();
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
            if (o) {
                meta_create(*o, table_name, p.name());
                continue;
            }
        }
        if (isAssociativeContainer(p.userType()) || isPair(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using containers without using orm_rowid is not supported. "
                              "Type " << table_name << " property " << p.name();
                continue;
            }
            meta_create_pair(p.userType(), table_name, p.name());
            continue;
        }
    }
}
void     ORM::meta_create_pair  (int usermetatype, QString const& parent_name, QString const& property_name)
{
    QString className = QMetaType::typeName(usermetatype);
    QString table_name = generate_table_name(parent_name, property_name, className);
    QStringList query_columns;
    QList<int> query_types;
    query_columns << "orm_rowid";
    query_types << qMetaTypeId<long long>();
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? assContTypeMap[usermetatype].first : assContTypeMap[usermetatype].second;
        QString name = j == 0 ? "key" : "value";
        if (isPrimitive(i)) {
            query_columns << name;
            query_types << i;
            continue;
        }
        if (isGadget(i) || isPointer(i) || isSequentialContainer(i) ||
                isAssociativeContainer(i) || isPair(i)) {
            continue;
        }
        query_columns << name;
        query_types << i;
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_create_query(parent_name, property_name, className, query_columns, query_types));
    h::checkQueryErrors(query, "", table_name);
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? assContTypeMap[usermetatype].first : assContTypeMap[usermetatype].second;
        QString name = j == 0 ? "key" : "value";
        if (isPrimitive(i)) {
            continue;
        }
        if (isGadget(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(i);
            if (o) {
                meta_create(*o, table_name, name);
                continue;
            }
        }
        if (isPointer(i)) {
            if (!isWeakPointer(i)) {
                const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(i));
                if (o) {
                    meta_create(*o, table_name, name);
                    continue;
                }
            }
            continue;
        }
        if (isSequentialContainer(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(i));
            if (o) {
                meta_create(*o, table_name, name);
                continue;
            }
        }
        if (isAssociativeContainer(i) || isPair(i)) {
            meta_create_pair(i, table_name, name);
            continue;
        }
    }
}

QVariant ORM::meta_select(const QMetaObject &meta, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = q::isQObject(meta);
    bool with_orm_rowid = q::withRowid(meta);
    if (!selectQueries.contains(table_name)) {
        QStringList query_columns;
        QList<int> query_types;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (!isPrimitive(p.userType()) && (!p.isStored() || isGadget(p.userType()) ||
                 isPointer(p.userType()) || isSequentialContainer(p.userType()) ||
                 isAssociativeContainer(p.userType()))) {
                continue;
            }
            query_columns << p.name();
            query_types << p.userType();
        }
        if (!with_orm_rowid && !query_columns.size()) {
            return QVariant();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_select_query(parent_name, property_name, meta.className(),
                                            query_columns, query_types, parent_orm_rowid != 0));
        selectQueries[table_name] = query;
    }
    QSqlQuery & query = selectQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
    }
    query.exec();
    h::checkQueryErrors(query, meta.className(), table_name, property_name);
    QVariantList list;
    while (query.next()) {
        QVariant v;
        if (isQObject) {
            if (!objectMap.contains(classtype)) {
                qWarning() << "select: " << meta.className() << " is not registered. "
                              "Call registerQObjectORM<" << meta.className() << ">(); first";
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
            orm_rowid = query.value(normalize("orm_rowid")).toLongLong();
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
                            QVariantList vl = meta_select(*o, table_name, QString(p.name()), orm_rowid).toList();
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
                            QVariantList vl = meta_select(*o, table_name, QString(p.name()), orm_rowid).toList();
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
                            h::write(isQObject, v, p, meta_select(*o, table_name, QString(p.name()), orm_rowid).toList());
                            continue;
                        }
                    }
                    if (isAssociativeContainer(p.userType())) {
                        QVariant qv = meta_select_pair(p.userType(), table_name, QString(p.name()), orm_rowid);
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
            h::write(isQObject, v, p, vv);
        }
        list << v;
    }
    return QVariant(list);
}
QVariant ORM::meta_select_pair  (int usermetatype, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(usermetatype);
    QString table_name = generate_table_name(parent_name, property_name, className);
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    if (!selectQueries.contains(table_name)) {
        QStringList query_columns;
        QList<int> query_types;
        query_columns << "orm_rowid";
        query_types << qMetaTypeId<long long>();
        for (int j = 0; j < 2; ++j) {
            int i = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (!isPrimitive(i) && (isGadget(i) || isPointer(i) ||
                                    isSequentialContainer(i) || isAssociativeContainer(i))) {
                continue;
            }
            query_columns << name;
            query_types << i;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_select_query(parent_name, property_name, className,
                                            query_columns, query_types, parent_orm_rowid != 0));
        selectQueries[table_name] = query;
    }
    QSqlQuery & query = selectQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
    }
    query.exec();
    h::checkQueryErrors(query, className, table_name, property_name);
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
                        QVariantList vl = meta_select(*o, table_name, name, pair.m_orm_rowid).toList();
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
                        QVariantList vl = meta_select(*o, table_name, name, pair.m_orm_rowid).toList();
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
                        pair[j] = meta_select(*o, table_name, name, pair.m_orm_rowid).toList();
                        continue;
                    }
                }
                if (isAssociativeContainer(i)) {
                    QVariant qv = meta_select_pair(i, table_name, name, pair.m_orm_rowid);
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

void     ORM::meta_insert(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    bool isQObject = meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    long long orm_rowid = 0;
    bool with_orm_rowid = q::withRowid(meta);
    if (!insertQueries.contains(table_name)) {
        QStringList names;
        QList<int> types;
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
            names << p.name();
            types << p.userType();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_insert_query(parent_name, property_name, meta.className(),
                                            names, types, parent_orm_rowid != 0));
        insertQueries[table_name] = query;
    }
    QSqlQuery & query = insertQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
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
        query.bindValue(QString(":") + p.name(), h::read(isQObject, v, p));
    }
    query.exec();
    if (h::checkQueryErrors(query, meta.className(), table_name, property_name)) {
        return;
    }

    if (with_orm_rowid) {
        orm_rowid = get_last_inserted_rowid(query);
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
                        meta_insert(*o, vv, table_name, QString(p.name()), orm_rowid);
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
                            meta_insert(*o, vvv, table_name, QString(p.name()), orm_rowid);
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
                                meta_insert(*o, vv, table_name, QString(p.name()), orm_rowid);
                            }
                        }
                        continue;
                    }
                }
                if (isAssociativeContainer(p.userType())) {
                    QVariant qv = h::read(isQObject, v, p);
                    if (qv.canConvert<ORMQVariantPair>()) {
                        qv.convert(qMetaTypeId<ORMQVariantPair>());
                        meta_insert_pair(p.userType(), qv, table_name, QString(p.name()), orm_rowid);
                        continue;
                    }
                    if (qv.canConvert<QList<ORMQVariantPair>>()) {
                        QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                        for (ORMQVariantPair & v : list) {
                            QVariant vv = QVariant::fromValue(v);
                            meta_insert_pair(p.userType(), vv, table_name, QString(p.name()), orm_rowid);
                        }
                        continue;
                    }
                }
            }
        }
    }
}
void     ORM::meta_insert_pair  (int usermetatype, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(usermetatype);
    QString table_name = generate_table_name(parent_name, property_name, className);
    ORMQVariantPair ops = v.value<ORMQVariantPair>();
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    if (!insertQueries.contains(table_name)) {
        QStringList names;
        QList<int> types;
        for (int j = 0; j < 2; ++j) {
            int i = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (!isPrimitive(i) && (isGadget(i) || isPointer(i) || isSequentialContainer(i) || isAssociativeContainer(i))) {
                continue;
            }
            names << name;
            types << i;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_insert_query(parent_name, property_name, className,
                                            names, types, parent_orm_rowid != 0));
        insertQueries[table_name] = query;
    }
    QSqlQuery & query = insertQueries[table_name];
    query.bindValue(":parent_orm_rowid", parent_orm_rowid);
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (!isPrimitive(i) && (isGadget(i) || isPointer(i) || isSequentialContainer(i) || isAssociativeContainer(i))) {
            continue;
        }
        query.bindValue(QString(":") + name, ops[j]);
    }
    query.exec();
    if (h::checkQueryErrors(query, className, table_name, property_name)) {
        return;
    }

    ops.m_orm_rowid = get_last_inserted_rowid(query);
    if (!ops.m_orm_rowid) {
        return;
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
                meta_insert(*o, ops[j], table_name, name, ops.m_orm_rowid);
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
                    meta_insert(*o, vvv, table_name, name, ops.m_orm_rowid);
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
                        meta_insert(*o, vv, table_name, name, ops.m_orm_rowid);
                    }
                }
                continue;
            }
        }
        if (isAssociativeContainer(i)) {
            QVariant qv = ops[j];
            if (qv.canConvert<ORMQVariantPair>()) {
                qv.convert(qMetaTypeId<ORMQVariantPair>());
                meta_insert_pair(i, qv, table_name, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<ORMQVariantPair>>()) {
                QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                for (ORMQVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_insert_pair(i, vv, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
}

void     ORM::meta_update(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    bool isQObject = q::isQObject(meta);
    if (!q::withRowid(meta)) {
        qWarning() << "update: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    if (!updateQueries.contains(table_name)) {
        QStringList sets;
        QStringList names;
        QList<int> types;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (!isPrimitive(p.userType()) && (!p.isStored() || isGadget(p.userType()) || isPointer(p.userType()) ||
                   isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
                continue;
            }
            if (q::isRowID(p.name())) {
                continue;
            }
            names << p.name();
            types << p.userType();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_update_query(parent_name, property_name, meta.className(),
                                            names, types, parent_orm_rowid != 0));
        updateQueries[table_name] = query;
    }
    QSqlQuery & query = updateQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
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
    if (h::checkQueryErrors(query, meta.className(), table_name, property_name)) {
        return;
    }

    long long orm_rowid = get_last_updated_rowid(query);
    if (orm_rowid == 0 && false) {
        meta_insert(meta, v, parent_name, property_name, parent_orm_rowid);
        return;
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
                meta_update(*o, vv, table_name, QString(p.name()), orm_rowid);
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
                    meta_update(*o, vvv, table_name, QString(p.name()), orm_rowid);
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
                        meta_update(*o, v, table_name, QString(p.name()), orm_rowid);
                    }
                }
                continue;
            }
        }
        if (isAssociativeContainer(p.userType())) {
            QVariant qv = h::read(isQObject, v, p);
            if (qv.canConvert<ORMQVariantPair>()) {
                qv.convert(qMetaTypeId<ORMQVariantPair>());
                meta_update_pair(p.userType(), qv, table_name, QString(p.name()), orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<ORMQVariantPair>>()) {
                QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                for (ORMQVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_update_pair(p.userType(), vv, table_name, QString(p.name()), orm_rowid);
                }
                continue;
            }
        }
    }
}
void     ORM::meta_update_pair  (int usermetatype, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(usermetatype);
    QString table_name = generate_table_name(parent_name, property_name, className);
    ORMQVariantPair ops = v.value<ORMQVariantPair>();
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    if (!updateQueries.contains(table_name)) {
        QStringList names;
        QList<int> types;
        for (int j = 0; j < 2; ++j) {
            int i = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (!isPrimitive(i) && (isGadget(i) || isPointer(i) || isSequentialContainer(i) || isAssociativeContainer(i))) {
                continue;
            }
            names << name;
            types << i;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_update_query(parent_name, property_name, className,
                                            names, types, parent_orm_rowid != 0));
        updateQueries[table_name] = query;
    }
    QSqlQuery & query = updateQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
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
    if (h::checkQueryErrors(query, className, table_name, property_name)) {
        return;
    }

    long long orm_rowid = get_last_updated_rowid(query);
    if (orm_rowid == 0 && false) {
        meta_insert_pair(usermetatype, v, parent_name, property_name, parent_orm_rowid);
        return;
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
                meta_update(*o, vv, table_name, name, ops.m_orm_rowid);
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
                    meta_update(*o, vvv, table_name, name, ops.m_orm_rowid);
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
                        meta_update(*o, v, table_name, name, ops.m_orm_rowid);
                    }
                }
                continue;
            }
        }
        if (isAssociativeContainer(i)) {
            QVariant qv = ops[j];
            if (qv.canConvert<ORMQVariantPair>()) {
                qv.convert(qMetaTypeId<ORMQVariantPair>());
                meta_update_pair(i, qv, table_name, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<ORMQVariantPair>>()) {
                QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                for (ORMQVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_update_pair(i, vv, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
}

void     ORM::meta_delete(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
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
                    meta_delete(*o, vv, table_name, QString(p.name()), orm_rowid);
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
                    meta_delete(*o, vv, table_name, QString(p.name()), orm_rowid);
                    continue;
                }
            }
            if (isSequentialContainer(p.userType())) {
                const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
                if (o) {
                    QSequentialIterable si = h::read(isQObject, v, p).value<QSequentialIterable>();
                    for (QVariant v : si) {
                        meta_delete(*o, v, table_name, QString(p.name()), orm_rowid);
                    }
                    continue;
                }
            }
            if (isAssociativeContainer(p.userType())) {
                QVariant qv = h::read(isQObject, v, p);
                if (qv.canConvert<ORMQVariantPair>()) {
                    qv.convert(qMetaTypeId<ORMQVariantPair>());
                    meta_delete_pair(p.userType(), qv, table_name, QString(p.name()), orm_rowid);
                    continue;
                }
                if (qv.canConvert<QList<ORMQVariantPair>>()) {
                    QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                    for (ORMQVariantPair & v : list) {
                        QVariant vv = QVariant::fromValue(v);
                        meta_delete_pair(p.userType(), vv, table_name, QString(p.name()), orm_rowid);
                    }
                    continue;
                }
            }
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(generate_delete_query(parent_name, property_name, QString(meta.className()), parent_orm_rowid));
    if (!parent_orm_rowid) {
        query.bindValue(":orm_rowid", orm_rowid);
    }
    else {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
    }
    query.exec();
    h::checkQueryErrors(query, meta.className(), table_name, property_name);
}
void     ORM::meta_delete_pair  (int usermetatype, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(usermetatype);
    QString table_name = generate_table_name(parent_name, property_name, className);
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
                meta_delete(*o, ops[j], table_name, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (isPointer(i)) {
            if (isWeakPointer(i)) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(i));
            if (o) {
                meta_delete(*o, ops[j], table_name, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (isSequentialContainer(i)) {
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(i));
            if (o) {
                QSequentialIterable si = ops[j].value<QSequentialIterable>();
                for (QVariant v : si) {
                    meta_delete(*o, v, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
        if (isAssociativeContainer(i)) {
            QVariant qv = ops[j];
            if (qv.canConvert<ORMQVariantPair>()) {
                qv.convert(qMetaTypeId<ORMQVariantPair>());
                meta_delete_pair(i, qv, table_name, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<ORMQVariantPair>>()) {
                QList<ORMQVariantPair> list = qv.value<QList<ORMQVariantPair>>();
                for (ORMQVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_delete_pair(i, vv, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(generate_delete_query(parent_name, property_name, className, parent_orm_rowid));
    if (!parent_orm_rowid) {
        query.bindValue(":orm_rowid", ops.m_orm_rowid);
    }
    else {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
    }
    query.exec();
    h::checkQueryErrors(query, className, table_name, property_name);
}

void     ORM::meta_drop  (const QMetaObject &meta, QString const& parent_name, const QString &property_name)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
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
            meta_drop_pair(p.userType(), table_name, p.name());
            continue;
        }
        if (o) {
            meta_drop(*o, table_name, p.name());
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_drop_query(parent_name, property_name, QString(meta.className())));
    h::checkQueryErrors(query, meta.className(), table_name);
}
void     ORM::meta_drop_pair    (int usermetatype, QString const& parent_name, QString const& property_name)
{
    QString className = QMetaType::typeName(usermetatype);
    QString table_name = generate_table_name(parent_name, property_name, className);
    int keyType = assContTypeMap[usermetatype].first;
    int valueType = assContTypeMap[usermetatype].second;
    for (int j = 0; j < 2; ++j) {
        int i = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
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
            meta_drop_pair(usermetatype, table_name, name);
            continue;
        }
        if (o) {
            meta_drop(*o, table_name, name);
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_drop_query(parent_name, property_name, className));
    h::checkQueryErrors(query, className, table_name);
}









