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

namespace ORM_Impl
{
    static QSet<int>                               pairTypes                                              ;
    static QMap<int, QPair<int, int>>              assContTypeMap                                         ;
    static QMap<int, orm_qobjects::creators>       objectMap                                              ;
    static QMap<int, orm_pointers::ORMPointerStub> pointerMap                                             ;
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
        QMetaType metatype(meta_type_id);
        return (metatype.flags() & QMetaType::IsGadget) == QMetaType::IsGadget;
    }

    inline bool isPointer(int meta_type_id)
    {
        QMetaType metatype(meta_type_id);
        if (metatype.flags() & QMetaType::PointerToGadget || metatype.flags() & QMetaType::PointerToQObject) {
            return true;
        }
        return ORM_Impl::pointerMap.contains(meta_type_id) && ORM_Impl::pointerMap[meta_type_id].T != meta_type_id;
    }
    inline bool isWeakPointer(int meta_type_id)
    {
        if (ORM_Impl::pointerMap.contains(meta_type_id) &&
                (ORM_Impl::pointerMap[meta_type_id].WT == meta_type_id ||
                 ORM_Impl::pointerMap[meta_type_id].wT == meta_type_id)) {
            return true;
        }
        return false;
    }

    inline int pointerToValue(int meta_type_id)
    {
        if (ORM_Impl::pointerMap.contains(meta_type_id)) {
            return ORM_Impl::pointerMap[meta_type_id].T;
        }
        return 0;
    }

    inline bool shouldSkip(int meta_type_id)
    {
        return ORM_Impl::ignoredTypes.contains(meta_type_id);
    }
    inline bool isPrimitiveType(int meta_type_id)
    {
        return ORM_Impl::primitiveContainers.contains(meta_type_id);
    }
    inline bool isPrimitiveString(int meta_type_id)
    {
        return ORM_Impl::primitiveStringContainers.contains(meta_type_id);
    }
    inline bool isPrimitiveRaw(int meta_type_id)
    {
        return ORM_Impl::primitiveRawContainers.contains(meta_type_id);
    }

    inline bool isSequentialContainer(int meta_type_id)
    {
        return QMetaType::hasRegisteredConverterFunction(meta_type_id, qMetaTypeId<QtMetaTypePrivate::QSequentialIterableImpl>());
    }

    inline bool isPair(int meta_type_id)
    {
        return ORM_Impl::pairTypes.contains(meta_type_id);
    }
    inline bool isAssociativeContainer(int meta_type_id)
    {
        return ORM_Impl::assContTypeMap.contains(meta_type_id);
    }

    inline int getSequentialContainerStoredType(int meta_type_id)
    {
        return (*(QVariant(static_cast<QVariant::Type>(meta_type_id)).value<QSequentialIterable>()).end()).userType();
    }

    inline int getAssociativeContainerStoredKeyType(int meta_type_id)
    {
        if (!ORM_Impl::assContTypeMap.contains(meta_type_id)) return 0;
        return ORM_Impl::assContTypeMap[meta_type_id].first;
    }
    inline int getAssociativeContainerStoredValueType(int meta_type_id)
    {
        if (!ORM_Impl::assContTypeMap.contains(meta_type_id)) return 0;
        return ORM_Impl::assContTypeMap[meta_type_id].second;
    }


    inline bool isQObject(int meta_type_id)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
        if (meta) {
            return meta->inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
        }
        return false;
    }
    inline bool isQObject(QMetaObject const& meta)
    {
        return meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    }

    inline bool isIgnored(int meta_type_id)
    {
        return ignoredTypes.contains(meta_type_id);
    }

    inline bool isPrimitive(int meta_type_id)
    {
        return  isPrimitiveType(meta_type_id) ||
                isPrimitiveString(meta_type_id) ||
                isPrimitiveRaw(meta_type_id);
    }
    inline bool isTypeForTable(int meta_type_id)
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

    inline bool shouldSkipReadMeta(QMetaProperty const& property, QObject* object = nullptr)
    {
        return !property.isReadable() || !property.isStored(object);
    }

    inline bool shouldSkipWriteMeta(QMetaProperty const& property, QObject* object = nullptr)
    {
        return !property.isWritable() || !property.isStored(object);
    }
    inline bool shouldSkipMeta(QMetaProperty const& property, QObject* object = nullptr)
    {
        return shouldSkipReadMeta(property, object) || shouldSkipWriteMeta(property, object);
    }

    inline bool isRowID(const QMetaProperty & property)
    {
        return QString(ORM_Impl::orm_rowidName) == QString(property.name());
    }
    inline bool isRowID(const QString & property)
    {
        return QString(ORM_Impl::orm_rowidName) == property;
    }
    inline bool withRowid(const QMetaObject &meta)
    {
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (shouldSkipMeta(property)) {
                continue;
            }
            if (isRowID(property)) {
                return true;
            }
        }
        return false;
    }



    inline bool checkQueryErrors(QSqlQuery & query, QString const& meta, QString tableName, QString property_name = QString())
    {
        Q_UNUSED(property_name)
        if (query.lastError().isValid()) {
            qDebug() << meta << tableName;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.executedQuery();
            qDebug() << query.boundValues();
            return true;
        }
        return false;
    }
    inline bool    write(bool isQObject, QVariant      & writeInto, QMetaProperty property, QVariant const& value)
    {
        if (isQObject) {
            QObject * object = writeInto.value<QObject*>();
            if (ORM_Impl::shouldSkipWriteMeta(property, object)) {
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
        QVariant result;
        if (isQObject) {
            QObject * object = readFrom.value<QObject*>();
            if (!ORM_Impl::shouldSkipReadMeta(property, object)) {
                result =  property.read(object);
            }
        }
        else {
            result = property.readOnGadget(readFrom.value<void*>());
        }
        return result;
    }



    void registerPrimitiveTypeContainers()
    {
        orm_containers::registerPrimitiveTypeContainer<QList>();
        orm_containers::registerPrimitiveTypeContainer<QVector>();
    }

}



void orm_qobjects::addQObjectStub(int meta_type_id, orm_qobjects::creators stub)
{
    if (!meta_type_id) qWarning() << "registerQObject: Could not register type 0";
    if (meta_type_id)  ORM_Impl::objectMap[meta_type_id] = stub;
}
bool orm_qobjects::hasQObjectStub(int meta_type_id)
{
    return ORM_Impl::objectMap.contains(meta_type_id);
}



orm_pointers::ORMPointerStub ORM_Config::getPointerStub(int metaTypeID)
{
    if (ORM_Impl::pointerMap.contains(metaTypeID)) return ORM_Impl::pointerMap[metaTypeID];
    return orm_pointers::ORMPointerStub();
}
void ORM_Config::addPointerStub(const orm_pointers::ORMPointerStub & stub)
{
    if (stub. T) ORM_Impl::pointerMap[stub. T] = stub;
    if (stub.pT) ORM_Impl::pointerMap[stub.pT] = stub;
    if (stub.ST) ORM_Impl::pointerMap[stub.ST] = stub;
    if (stub.WT) ORM_Impl::pointerMap[stub.WT] = stub;
    if (stub.sT) ORM_Impl::pointerMap[stub.sT] = stub;
    if (stub.wT) ORM_Impl::pointerMap[stub.wT] = stub;
}

void ORM_Config::addPairType(int firstTypeID, int secondTypeID, int pairTypeID)
{
    addContainerPairType(firstTypeID, secondTypeID, pairTypeID);
    if (!ORM_Impl::pairTypes.contains(pairTypeID)) {
        ORM_Impl::pairTypes << pairTypeID;
    }
}
void ORM_Config::addContainerPairType(int firstTypeID, int secondTypeID, int pairTypeID)
{
    if (!ORM_Impl::assContTypeMap.contains(pairTypeID)) {
        ORM_Impl::assContTypeMap[pairTypeID] = QPair<int, int>(firstTypeID, secondTypeID);
    }
}

void ORM_Config::addIgnoredType(int meta_type_id)
{
    ORM_Impl::ignoredTypes << meta_type_id;
}

bool ORM_Config::isIgnoredType(int meta_type_id)
{
    return ORM_Impl::ignoredTypes.contains(meta_type_id);
}

void ORM_Config::removeIgnoredType(int meta_type_id)
{
    ORM_Impl::primitiveContainers.remove(meta_type_id);
}

void ORM_Config::addPrimitiveType(int meta_type_id)
{
    ORM_Impl::primitiveContainers << meta_type_id;
}

bool ORM_Config::isPrimitiveType(int meta_type_id)
{
    return ORM_Impl::isPrimitiveType(meta_type_id);
}

void ORM_Config::removePrimitiveType(int meta_type_id)
{
    ORM_Impl::primitiveContainers.remove(meta_type_id);
}

void ORM_Config::addPrimitiveStringType(int meta_type_id)
{
    Q_ASSERT_X( QMetaType::hasRegisteredConverterFunction(meta_type_id, qMetaTypeId<QString>()) &&
                QMetaType::hasRegisteredConverterFunction(qMetaTypeId<QString>(), meta_type_id),
                "addPrimitiveStringType", "Needs registred converters T->QString->T");
    ORM_Impl::primitiveStringContainers << meta_type_id;
}

bool ORM_Config::isPrimitiveStringType(int meta_type_id)
{
    return ORM_Impl::isPrimitiveString(meta_type_id);
}

void ORM_Config::removePrimitiveStringType(int meta_type_id)
{
    ORM_Impl::primitiveStringContainers.remove(meta_type_id);
}

void ORM_Config::addPrimitiveRawType(int meta_type_id)
{
    ORM_Impl::primitiveRawContainers << meta_type_id;
}

bool ORM_Config::isPrimitiveRawType(int meta_type_id)
{
    return ORM_Impl::isPrimitiveRaw(meta_type_id);
}

void ORM_Config::removePrimitiveRawType(int meta_type_id)
{
    ORM_Impl::primitiveRawContainers.remove(meta_type_id);
}



uint qHash(const orm_containers::ORM_QVariantPair &variantPair) noexcept
{
    return qHash(variantPair.key.data());
}










ORM::ORM(const QString & dbname) : m_databaseName(QSqlDatabase::defaultConnection)
{
    if (!dbname.isEmpty()) {
        m_databaseName = dbname;
    }
    if (ORM_Impl::orm_once) {
        ORM_Impl::orm_once = false;
        ORM_Impl::registerPrimitiveTypeContainers();
        ormRegisterType<orm_containers::ORM_QVariantPair>();
        ORM_Config::addPrimitiveRawType<QChar>();
        ORM_Config::addPrimitiveRawType<QStringList>();
        ORM_Config::addPrimitiveRawType<QBitArray>();
        ORM_Config::addPrimitiveRawType<QLocale>();
        ORM_Config::addPrimitiveRawType<QRect>();
        ORM_Config::addPrimitiveRawType<QRectF>();
        ORM_Config::addPrimitiveRawType<QSize>();
        ORM_Config::addPrimitiveRawType<QSizeF>();
        ORM_Config::addPrimitiveRawType<QLine>();
        ORM_Config::addPrimitiveRawType<QLineF>();
        ORM_Config::addPrimitiveRawType<QPoint>();
        ORM_Config::addPrimitiveRawType<QPointF>();
        ORM_Config::addPrimitiveRawType<QRegExp>();
        ORM_Config::addPrimitiveRawType<QEasingCurve>();
        ORM_Config::addPrimitiveRawType<QVariant>();
        ORM_Config::addIgnoredType<QModelIndex>();
        ORM_Config::addIgnoredType<QJsonValue>();
        ORM_Config::addIgnoredType<QJsonObject>();
        ORM_Config::addIgnoredType<QJsonArray>();
        ORM_Config::addIgnoredType<QJsonDocument>();
        ORM_Config::addIgnoredType<QPersistentModelIndex>();
        ORM_Config::addPrimitiveRawType<QRegularExpression>();
        ORM_Config::addPrimitiveRawType<QByteArrayList>();
        ORM_Config::addPrimitiveRawType<QFont>();
        ORM_Config::addPrimitiveRawType<QPixmap>();
        ORM_Config::addPrimitiveRawType<QBrush>();
        ORM_Config::addPrimitiveRawType<QColor>();
        ORM_Config::addPrimitiveRawType<QPalette>();
        ORM_Config::addPrimitiveRawType<QIcon>();
        ORM_Config::addPrimitiveRawType<QImage>();
        ORM_Config::addPrimitiveRawType<QPolygon>();
        ORM_Config::addPrimitiveRawType<QRegion>();
        ORM_Config::addPrimitiveRawType<QBitmap>();
        ORM_Config::addPrimitiveRawType<QCursor>();
        ORM_Config::addPrimitiveRawType<QKeySequence>();
        ORM_Config::addPrimitiveRawType<QPen>();
        ORM_Config::addPrimitiveRawType<QTextLength>();
        ORM_Config::addPrimitiveRawType<QTextFormat>();
        ORM_Config::addPrimitiveRawType<QMatrix>();
        ORM_Config::addPrimitiveRawType<QTransform>();
        ORM_Config::addPrimitiveRawType<QMatrix4x4>();
        ORM_Config::addPrimitiveRawType<QVector2D>();
        ORM_Config::addPrimitiveRawType<QVector3D>();
        ORM_Config::addPrimitiveRawType<QVector4D>();
        ORM_Config::addPrimitiveRawType<QQuaternion>();
        ORM_Config::addPrimitiveRawType<QPolygonF>();
        ORM_Config::addPrimitiveRawType<QSizePolicy>();
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

void     ORM::create(int meta_type_id)
{
    const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
    if (meta) {
        meta_create(*meta);
    }
}
QVariant ORM::select(int meta_type_id)
{
    const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
    if (meta) {
       return  meta_select(*meta);
    }
    return QVariant();
}

QVariant ORM::get(int meta_type_id, long long id)
{
    const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
    if (meta) {
       return  meta_get(*meta, id);
    }
    return QVariant();
}
void     ORM::insert(int meta_type_id, QVariant &value)
{
    const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
    if (meta) {
        meta_insert(*meta, value);
    }
}
void     ORM::delet (int meta_type_id, QVariant &value)
{
    const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
    if (meta) {
        meta_delete(*meta, value);
    }
}
void     ORM::update(int meta_type_id, QVariant &value)
{
    const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
    if (meta) {
        meta_update(*meta, value);
    }
}
void     ORM::drop  (int meta_type_id)
{
    const QMetaObject * meta = QMetaType::metaObjectForType(meta_type_id);
    if (meta) {
        meta_drop(*meta);
    }
}




QString ORM::normalize(const QString & str, QueryType queryType) const
{
    Q_UNUSED(queryType)
    QString s = str;
    static QRegularExpression regExp1 {"(.)([A-Z]+)"};
    static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};
    static QRegularExpression regExp3 {"[:;,.<>]+"};
    return "_" + s.replace(regExp1, "\\1_\\2").replace(regExp2, "\\1_\\2").toLower().replace(regExp3, "_");
}

QString ORM::normalizeVar(const QString &str, int meta_type_id, QueryType queryType) const
{
    Q_UNUSED(meta_type_id)
    Q_UNUSED(queryType)
    return str;
}

QString ORM::sqlType(int meta_type_id) const {
    if (ORM_Impl::isPrimitiveType(meta_type_id)) {
        return "TEXT";
    }
    if (ORM_Impl::isPrimitiveString(meta_type_id)) {
        return "TEXT";
    }
    if (ORM_Impl::isPrimitiveRaw(meta_type_id)) {
        return "BLOB";
    }
    switch(meta_type_id) {
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
    case QMetaType::QChar:
        return "NUMERIC";
    case QMetaType::Float:
        return "FLOAT";
    case QMetaType::QString:
    case QMetaType::QDate:
    case QMetaType::QTime:
    case QMetaType::QDateTime:
    case QMetaType::QUrl:
    case QMetaType::QUuid:
        return "TEXT";
    case QMetaType::UnknownType:
    default:
        return "BLOB";
    }
}


QString ORM::generate_table_name(const QString &parent_name, const QString &property_name, const QString &class_name, QueryType query_type) const
{
    Q_UNUSED(query_type)
    return parent_name + normalize(class_name, query_type) + normalize(property_name, query_type);
}
long long ORM::get_last_inserted_rowid(QSqlQuery &query) const
{
    return query.lastInsertId().toLongLong();
}
long long ORM::get_changes (QSqlQuery &query) const
{
    return query.numRowsAffected();
}

long long ORM::get_last_rowid(QString table_name) const
{

    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    QString name = normalize(ORM_Impl::orm_rowidName, QueryType::Select);
    query.exec("SELECT " + name + " FROM " + table_name + " ORDER BY " + name + " DESC LIMIT 1;" );
    ORM_Impl::checkQueryErrors(query, "", table_name);
    if (!query.next()) {
        return 0;
    }
    return query.value(0).toLongLong();
}

QString ORM::generate_create_query(QString const& parent_name,
        QString const& property_name, const QString &class_name,
        const QStringList &names, const QList<int> &types) const
{
    QString tableName = generate_table_name(parent_name, property_name, class_name, QueryType::Create);
    bool withOrmRowID = false;
    for (int i = 0; i < names.count(); ++i) {
        if (ORM_Impl::isRowID(names[i])) {
            if (withOrmRowID) {
                qWarning() << "multiplie orm_rowid properties in " << tableName;
            }
            withOrmRowID = true;
            continue;
        }
    }
    QString queryText = QString("CREATE TABLE IF NOT EXISTS %1 (").arg(tableName);
    QStringList queryColumns;
    for (int i = 0; i < names.count(); ++i) {
        if (ORM_Impl::isRowID(names[i])) {
            queryColumns << normalize(names[i], QueryType::Create) + " INTEGER PRIMARY KEY ";
            continue;
        }
        queryColumns << normalize(names[i], QueryType::Create) + " " + sqlType(types[i]);
    }
    if (parent_name != "t") {
        queryText += " " + ORM_Impl::orm_parentRowidName + " INTEGER REFERENCES "
                + parent_name + " (" + normalize(ORM_Impl::orm_rowidName, QueryType::Create) + ") ";
        if (queryColumns.size()) {
            queryText += ", ";
        }
    }
    if (queryColumns.size()) {
        queryText += queryColumns.join(", ");
    }
    queryText += ");";
    return queryText;
}
QString ORM::generate_select_query(QString const& parent_name,
        QString const& property_name, const QString &class_name,
        const QStringList &names, const QList<int> &types,
        bool parent_orm_rowid) const
{
    Q_UNUSED(types)
    QString tableName = generate_table_name(parent_name, property_name, class_name, QueryType::Select);
    bool withOrmRowID = false;
    for (int i = 0; i < names.count(); ++i) {
        if (ORM_Impl::isRowID(names[i])) {
            if (withOrmRowID) {
                qWarning() << "multiplie orm_rowid properties in " << tableName;
            }
            withOrmRowID = true;
            continue;
        }
    }
    QStringList queryColumns;
    QString queryText = "SELECT ";
    for (int i = 0; i < names.size(); ++i) {
        queryColumns << normalize(names[i], QueryType::Select);
    }
    if (parent_name != "t") {
        queryText += " " + ORM_Impl::orm_parentRowidName + " ";
        if (queryColumns.size()) {
            queryText += " , ";
        }
    }
    if (queryColumns.size()) {
        queryText += queryColumns.join(", ");
    }
    queryText += " FROM " + tableName;
    if (parent_orm_rowid) {
        queryText += " WHERE " + ORM_Impl::orm_parentRowidName + " = :" + ORM_Impl::orm_parentRowidName + " ";
    }
    queryText += ";";
    return queryText;
}

QString ORM::generate_select_where_query(const QString &parent_name,
        const QString &property_name, const QString &class_name,
        const QStringList &names, const QList<int> &types,
        const QVariantMap &wheres) const
{
    Q_UNUSED(types)
    if (wheres.isEmpty()) {
        return generate_select_query(parent_name, property_name, class_name, names, types, false);
    }
    QString tableName = generate_table_name(parent_name, property_name, class_name, QueryType::Select);
    bool withOrmRowID = false;
    for (int i = 0; i < names.count(); ++i) {
        if (ORM_Impl::isRowID(names[i])) {
            if (withOrmRowID) {
                qWarning() << "multiplie orm_rowid properties in " << class_name << "(" << tableName << ")";
            }
            withOrmRowID = true;
            continue;
        }
    }
    QStringList queryColumns, querySets;
    QString queryText = "SELECT ";
    for (int i = 0; i < names.size(); ++i) {
        queryColumns << normalize(names[i], QueryType::Select);
    }
    if (parent_name != "t") {
        queryText += " " + ORM_Impl::orm_parentRowidName + " ";
        if (queryColumns.size()) {
            queryText += " , ";
        }
    }
    if (queryColumns.size()) {
        queryText += queryColumns.join(", ");
    }
    queryText += " FROM " + tableName;
    for (auto where = wheres.begin(); where != wheres.end(); ++where) {
        querySets << normalize(where.key(), QueryType::Select) + " = " +
                      normalizeVar(":" + where.key(), where.value().userType(), QueryType::Select);
    }
    queryText += ";";
    return queryText;
}

QString ORM::generate_get_query(const QString &parent_name,
        const QString &property_name, const QString &class_name,
        const QStringList &names, const QList<int> &types) const
{
    Q_UNUSED(types)
    QString tableName = generate_table_name(parent_name, property_name, class_name, QueryType::Select);
    bool withOrmRowID = false;
    for (int i = 0; i < names.count(); ++i) {
        if (ORM_Impl::isRowID(names[i])) {
            if (withOrmRowID) {
                qWarning() << "multiplie orm_rowid properties in " << class_name << "(" << tableName << ")";
            }
            withOrmRowID = true;
            continue;
        }
    }
    QStringList queryColumns;
    QString queryText = "SELECT ";
    for (int i = 0; i < names.size(); ++i) {
        queryColumns << normalize(names[i], QueryType::Select);
    }
    if (parent_name != "t") {
        queryText += " " + ORM_Impl::orm_rowidName + " ";
        if (queryColumns.size()) {
            queryText += " , ";
        }
    }
    if (queryColumns.size()) {
        queryText += queryColumns.join(", ");
    }
    queryText += " FROM " + tableName + " WHERE " + normalize(ORM_Impl::orm_rowidName, QueryType::Select) +
            " = :" + ORM_Impl::orm_rowidName + " LIMIT 1;";
    return queryText;
}
QString ORM::generate_insert_query(QString const& parent_name,
        QString const& property_name, const QString &class_name,
        const QStringList &names, const QList<int> &types,
        bool parent_orm_rowid) const
{
    Q_UNUSED(types)
    QString tableName = generate_table_name(parent_name, property_name, class_name, QueryType::Insert);
    QStringList queryColumns, queryBindings;
    if (parent_orm_rowid) {
        queryColumns << ORM_Impl::orm_parentRowidName;
        queryBindings << ":" + ORM_Impl::orm_parentRowidName;
    }
    for (int i = 0; i < names.size(); ++i) {
        queryColumns << normalize(names[i], QueryType::Insert);
        queryBindings << normalizeVar(":" + names[i], types[i], QueryType::Insert); // let it be like this
    }
    // INFO: on SQLite UPSERT (since 3.24.0 (2018-06-04))
  //QStringList upsert;
  //for (int i = 0; i < t_names.size(); ++i) {
  //      upsert << t_names[i] + " = " + t_values[i];
  //}
    return QString("INSERT INTO %1 (%2) VALUES (%3);") //ON CONFLICT DO UPDATE SET %4 WHERE _orm_rowid = :_orm_rowid
            .arg(tableName).arg(queryColumns.join(", ")).arg(queryBindings.join(", "));//.arg(upsert.join(", "));
}
QString ORM::generate_update_query(QString const& parent_name,
        QString const& property_name, const QString &class_name,
        const QStringList &names, const QList<int> &types,
        bool parent_orm_rowid) const
{
    Q_UNUSED(types)
    QString tableName = generate_table_name(parent_name, property_name, class_name, QueryType::Update);
    QString queryText = QString("UPDATE OR IGNORE %1 SET ").arg(tableName);
    QStringList querySets;
    for (int i = 0; i < names.size(); ++i) {
        querySets << normalize(names[i], QueryType::Update) + " = " + normalizeVar(":" + names[i], types[i], QueryType::Update);
    }
    queryText += querySets.join(',') + " WHERE " + normalize(ORM_Impl::orm_rowidName, QueryType::Update) + " = :" + ORM_Impl::orm_rowidName + " ";
    if (parent_orm_rowid) {
        queryText += " AND " + ORM_Impl::orm_parentRowidName + " = :" + ORM_Impl::orm_parentRowidName + " ";
    }
    queryText += ";";
    return queryText;
}
QString ORM::generate_delete_query(QString const& parent_name,
        QString const& property_name, const QString &class_name,
        bool parent_orm_rowid) const
{
    QString tableName = generate_table_name(parent_name, property_name, class_name, QueryType::Delete);
    if (!parent_orm_rowid) {
        return QString("DELETE FROM %1 WHERE " + normalize(ORM_Impl::orm_rowidName, QueryType::Delete) +
                       " = :" + ORM_Impl::orm_rowidName + " ").arg(tableName);
    }
    else {
        return QString("DELETE FROM %1 WHERE " + ORM_Impl::orm_parentRowidName +
                       " = :" + ORM_Impl::orm_parentRowidName + ";").arg(tableName);
    }
}
QString ORM::generate_drop_query  (QString const& parent_name,
        QString const& property_name, const QString &class_name) const
{
    QString tableName = generate_table_name(parent_name, property_name, class_name, QueryType::Drop);
    return QString("DROP TABLE IF EXISTS %1;").arg(tableName);
}


void     ORM::meta_create(const QMetaObject &meta, QString const& parent_name,
                          QString const& property_name)
{
    QString tableName = generate_table_name(parent_name, property_name, QString(meta.className()),
                                             QueryType::Create);
    bool withOrmRowID = false;
    QStringList queryColumns;
    QList<int> queryTypes;
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty property = meta.property(i);
        if (ORM_Impl::isIgnored(property.userType())) {
            continue;
        }
        if (!property.isStored() || !property.isWritable() || !property.isReadable()) {
            continue;
        }
        if (ORM_Impl::isRowID(property.name())) {
            if (withOrmRowID) {
                qWarning() << "multiplie orm_rowid properties in " << meta.className();
            }
            withOrmRowID = true;
        }
        if (ORM_Impl::isPrimitiveType(property.userType())) {
            queryColumns << property.name();
            queryTypes << property.userType();
            continue;
        }
        if (ORM_Impl::isPrimitiveString(property.userType())) {
            queryColumns << property.name();
            queryTypes << qMetaTypeId<QString>();
            continue;
        }
        if (ORM_Impl::isPrimitiveRaw(property.userType())) {
            queryColumns << property.name();
            queryTypes << qMetaTypeId<QByteArray>();
            continue;
        }
        if (ORM_Impl::isTypeForTable(property.userType())) {
            continue;
        }
        queryColumns << property.name();
        queryTypes << property.userType();
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_create_query(parent_name, property_name, meta.className(),
                                     queryColumns, queryTypes));
    ORM_Impl::checkQueryErrors(query, meta.className(), tableName);

    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty property = meta.property(i);
        if (ORM_Impl::isIgnored(property.userType())) {
            continue;
        }
        if (ORM_Impl::isPrimitive(property.userType())) {
            continue;
        }
        if (ORM_Impl::isGadget(property.userType())) {
            if (!withOrmRowID) {
                qWarning() << "create: Using structure fields without using orm_rowid is not supported. "
                              "Type " << tableName << " property " << property.name();
                continue;
            }
            const QMetaObject * metaObject = QMetaType::metaObjectForType(property.userType());
            if (metaObject) {
                meta_create(*metaObject, tableName, property.name());
                continue;
            }
        }
        if (ORM_Impl::isPointer(property.userType())) {
            if (!ORM_Impl::isWeakPointer(property.userType())) {
                if (!withOrmRowID) {
                    qWarning() << "create: Using pointer fields without using orm_rowid is not supported. "
                                  "Type " << tableName << " property " << property.name();
                    continue;
                }
                const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(property.userType()));
                if (metaObject) {
                    meta_create(*metaObject, tableName, property.name());
                    continue;
                }
            }
            continue;
        }
        if (ORM_Impl::isSequentialContainer(property.userType())) {
            if (!withOrmRowID) {
                qWarning() << "create: Using containers without using orm_rowid is not supported. "
                              "Type " << tableName << " property " << property.name();
                continue;
            }
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(property.userType()));
            if (metaObject) {
                meta_create(*metaObject, tableName, property.name());
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(property.userType()) || ORM_Impl::isPair(property.userType())) {
            if (!withOrmRowID) {
                qWarning() << "create: Using containers without using orm_rowid is not supported. "
                              "Type " << tableName << " property " << property.name();
                continue;
            }
            meta_create_pair(property.userType(), tableName, property.name());
            continue;
        }
    }
}
void     ORM::meta_create_pair  (int meta_type_id, QString const& parent_name, QString const& property_name)
{
    QString className = QMetaType::typeName(meta_type_id);
    QString tableName = generate_table_name(parent_name, property_name, className, QueryType::Create);
    QStringList queryColumns;
    QList<int> queryTypes;
    queryColumns << ORM_Impl::orm_rowidName;
    queryTypes << qMetaTypeId<long long>();
    for (int column = 0; column < 2; ++column) {
        int type = (column == 0)
                ? ORM_Impl::getAssociativeContainerStoredKeyType(meta_type_id)
                : ORM_Impl::getAssociativeContainerStoredValueType(meta_type_id);
        QString name = (column == 0) ? "key" : "value";
        if (ORM_Impl::isIgnored(type)) {
            continue;
        }
        if (ORM_Impl::isPrimitiveType(type)) {
            queryColumns << name;
            queryTypes << type;
            continue;
        }
        if (ORM_Impl::isPrimitiveString(type)) {
            queryColumns << name;
            queryTypes << qMetaTypeId<QString>();
            continue;
        }
        if (ORM_Impl::isPrimitiveRaw(type)) {
            queryColumns << name;
            queryTypes << qMetaTypeId<QByteArray>();
            continue;
        }
        if (ORM_Impl::isTypeForTable(type)) {
            continue;
        }
        queryColumns << name;
        queryTypes << type;
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_create_query(parent_name, property_name, className, queryColumns, queryTypes));
    ORM_Impl::checkQueryErrors(query, "", tableName);
    for (int column = 0; column < 2; ++column) {
        int type = (column == 0)
                ? ORM_Impl::getAssociativeContainerStoredKeyType(meta_type_id)
                : ORM_Impl::getAssociativeContainerStoredValueType(meta_type_id);
        QString name = column == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(type)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(type)) {
            continue;
        }
        if (ORM_Impl::isGadget(type)) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(type);
            if (metaObject) {
                meta_create(*metaObject, tableName, name);
                continue;
            }
        }
        if (ORM_Impl::isPointer(type)) {
            if (!ORM_Impl::isWeakPointer(type)) {
                const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(type));
                if (metaObject) {
                    meta_create(*metaObject, tableName, name);
                    continue;
                }
            }
            continue;
        }
        if (ORM_Impl::isSequentialContainer(type)) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(type));
            if (metaObject) {
                meta_create(*metaObject, tableName, name);
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(type) || ORM_Impl::isPair(type)) {
            meta_create_pair(type, tableName, name);
            continue;
        }
    }
}

QVariant ORM::meta_select(const QMetaObject &meta, QString const& parent_name,
         QString const& property_name, long long parent_orm_rowid)
{
    QString tableName = generate_table_name(parent_name,
         property_name, QString(meta.className()), QueryType::Select);
    int classType = QMetaType::type(meta.className());
    bool isQObject = ORM_Impl::isQObject(meta);
    bool withOrmRowID = ORM_Impl::withRowid(meta);
    if (!selectQueries.contains(tableName)) {
        QStringList queryColumns;
        QList<int> queryTypes;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (ORM_Impl::isIgnored(property.userType())) {
                continue;
            }
            if (!property.isStored() || ORM_Impl::isTypeForTable(property.userType())) {
                continue;
            }
            if (ORM_Impl::isPrimitiveString(property.userType())) {
                queryColumns << property.name();
                queryTypes << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(property.userType())) {
                queryColumns << property.name();
                queryTypes << qMetaTypeId<QByteArray>();
                continue;
            }
            queryColumns << property.name();
            queryTypes << property.userType();
        }
        if (!withOrmRowID && !queryColumns.size()) {
            return QVariant();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_select_query(parent_name, property_name, meta.className(),
                                            queryColumns, queryTypes, parent_orm_rowid != 0));
        selectQueries[tableName] = query;
    }
    QSqlQuery & query = selectQueries[tableName];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, meta.className(), tableName, property_name);
    QVariantList resultList;
    while (query.next()) {
        QVariant variant;
        if (isQObject) {
            if (!ORM_Impl::objectMap.contains(classType)) {
                qWarning() << "select: " << meta.className() << " is not registered. "
                              "Call registerQObjectORM<" << meta.className() << ">(); first";
                break;
            }
            variant = QVariant::fromValue(ORM_Impl::objectMap[classType]());
        }
        else {
            variant = QVariant((classType), nullptr);
        }
        if (!variant.isValid()){
            qWarning() << "select: Unable to create instance of type " << meta.className();
            break;
        }
        if (isQObject && variant.value<QObject*>() == nullptr) {
            qWarning() << "select: Unable to create instance of QObject " << meta.className();
            break;
        }
        long long orm_rowid = 0;
        if (withOrmRowID) {
            orm_rowid = query.value(normalize(ORM_Impl::orm_rowidName, QueryType::Select)).toLongLong();
        }
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (ORM_Impl::isIgnored(property.userType())) {
                continue;
            }
            if (ORM_Impl::shouldSkipWriteMeta(property)) {
                continue;
            }
            if (withOrmRowID) {
                if (!ORM_Impl::isPrimitive(property.userType())) {
                    if (ORM_Impl::isGadget(property.userType())) {
                        const QMetaObject * metaObject = QMetaType::metaObjectForType(property.userType());
                        if (metaObject) {
                            QVariantList variantList = meta_select(*metaObject, tableName, QString(property.name()), orm_rowid).toList();
                            if (variantList.size()) {
                                ORM_Impl::write(isQObject, variant, property, variantList.first());
                            }
                            continue;
                        }
                    }
                    if (ORM_Impl::isPointer(property.userType())) {
                        if (ORM_Impl::isWeakPointer(property.userType())) {
                            continue;
                        }
                        const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(property.userType()));
                        if (metaObject) {
                            qDebug() << "beforeWrite" << property.name();
                            QVariantList variantList = meta_select(*metaObject, tableName, QString(property.name()), orm_rowid).toList();
                            if (variantList.size()) {
                                if (variantList.first().value<void*>()) {
                                    ORM_Impl::write(isQObject, variant, property, variantList.first());
                                    qDebug() << "Write " << property.name() << variantList.first().data();
                                }
                            }
                            qDebug() << "afterWrite" << property.name();
                            continue;
                        }
                    }
                    if (ORM_Impl::isSequentialContainer(property.userType())) {
                        const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(property.userType()));
                        if (metaObject) {
                            ORM_Impl::write(isQObject, variant, property, meta_select(*metaObject, tableName, QString(property.name()), orm_rowid).toList());
                            continue;
                        }
                    }
                    if (ORM_Impl::isAssociativeContainer(property.userType()) || ORM_Impl::isPair(property.userType())) {
                        QVariant variantResult = meta_select_pair(property.userType(), tableName, QString(property.name()), orm_rowid);
                        if (variantResult.canConvert(property.userType())) {
                            variantResult.convert(property.userType());
                            ORM_Impl::write(isQObject, variant, property, variantResult);
                        }
                        continue;
                    }
                }
            }
            QVariant queryValue = query.value(normalize(property.name(), QueryType::Select));
            if (property.isEnumType()) {
                queryValue.convert(property.userType());
            }
            if (ORM_Impl::isPrimitiveString(property.userType())) {
                queryValue.convert(property.userType());
            }
            if (ORM_Impl::isPrimitiveRaw(property.userType())) {
                QVariant temp;
                QByteArray array = queryValue.toByteArray();
                QDataStream stream(&array,QIODevice::ReadWrite);
                stream >> temp;
                queryValue = temp;
            }
            ORM_Impl::write(isQObject, variant, property, queryValue);
        }
        resultList << variant;
    }
    query.finish();
    return QVariant(resultList);
}

QVariant ORM::meta_select_where(const QMetaObject &meta, const QVariantMap &wheres, const QString &parent_name, const QString &property_name)
{
    if (wheres.isEmpty()) {
        return meta_select(meta, parent_name, property_name, 0);
    }
    QString tableName = generate_table_name(parent_name, property_name, QString(meta.className()),
                                             QueryType::Select);
    int classType = QMetaType::type(meta.className());
    bool isQObject = ORM_Impl::isQObject(meta);
    bool withOrmRowID = ORM_Impl::withRowid(meta);
    QStringList queryColumns;
    QList<int> queryTypes;
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty property = meta.property(i);
        if (ORM_Impl::isIgnored(property.userType())) {
            continue;
        }
        if (!property.isStored() || ORM_Impl::isTypeForTable(property.userType())) {
            continue;
        }
        if (ORM_Impl::isPrimitiveString(property.userType())) {
            queryColumns << property.name();
            queryTypes << qMetaTypeId<QString>();
            continue;
        }
        if (ORM_Impl::isPrimitiveRaw(property.userType())) {
            queryColumns << property.name();
            queryTypes << qMetaTypeId<QByteArray>();
            continue;
        }
        queryColumns << property.name();
        queryTypes << property.userType();
    }
    if (!withOrmRowID && !queryColumns.size()) {
        return QVariant();
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(generate_select_where_query(parent_name, property_name, meta.className(),
                                        queryColumns, queryTypes, wheres));
    for (auto where = wheres.begin(); where != wheres.end(); ++where) {
        query.bindValue(":" + where.key(), where.value());
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, meta.className(), tableName, property_name);
    QVariantList resultList;
    while (query.next()) {
        QVariant variant;
        if (isQObject) {
            if (!ORM_Impl::objectMap.contains(classType)) {
                qWarning() << "select: " << meta.className() << " is not registered. "
                              "Call registerQObjectORM<" << meta.className() << ">(); first";
                break;
            }
            variant = QVariant::fromValue(ORM_Impl::objectMap[classType]());
        }
        else {
            variant = QVariant((classType), nullptr);
        }
        if (!variant.isValid()){
            qWarning() << "select: Unable to create instance of type " << meta.className();
            break;
        }
        if (isQObject && variant.value<QObject*>() == nullptr) {
            qWarning() << "select: Unable to create instance of QObject " << meta.className();
            break;
        }
        long long ormRowid = 0;
        if (withOrmRowID) {
            ormRowid = query.value(normalize(ORM_Impl::orm_rowidName, QueryType::Select)).toLongLong();
        }
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (ORM_Impl::isIgnored(property.userType())) {
                continue;
            }
            if (ORM_Impl::shouldSkipWriteMeta(property)) {
                continue;
            }
            if (withOrmRowID) {
                if (!ORM_Impl::isPrimitive(property.userType())) {
                    if (ORM_Impl::isGadget(property.userType())) {
                        const QMetaObject * metaObject = QMetaType::metaObjectForType(property.userType());
                        if (metaObject) {
                            QVariantList variantList = meta_select(*metaObject, tableName, QString(property.name()), ormRowid).toList();
                            if (variantList.size()) {
                                ORM_Impl::write(isQObject, variant, property, variantList.first());
                            }
                            continue;
                        }
                    }
                    if (ORM_Impl::isPointer(property.userType())) {
                        if (ORM_Impl::isWeakPointer(property.userType())) {
                            continue;
                        }
                        const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(property.userType()));
                        if (metaObject) {
                            QVariantList variantList = meta_select(*metaObject, tableName, QString(property.name()), ormRowid).toList();
                            if (variantList.size()) {
                                if (variantList.first().value<void*>()) {
                                    ORM_Impl::write(isQObject, variant, property, variantList.first());
                                }
                            }
                            continue;
                        }
                    }
                    if (ORM_Impl::isSequentialContainer(property.userType())) {
                        const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(property.userType()));
                        if (metaObject) {
                            ORM_Impl::write(isQObject, variant, property, meta_select(*metaObject, tableName, QString(property.name()), ormRowid).toList());
                            continue;
                        }
                    }
                    if (ORM_Impl::isAssociativeContainer(property.userType()) || ORM_Impl::isPair(property.userType())) {
                        QVariant variantResult = meta_select_pair(property.userType(), tableName, QString(property.name()), ormRowid);
                        if (variantResult.canConvert(property.userType())) {
                            variantResult.convert(property.userType());
                            ORM_Impl::write(isQObject, variant, property, variantResult);
                        }
                        continue;
                    }
                }
            }
            QVariant queryValue = query.value(normalize(property.name(), QueryType::Select));
            if (property.isEnumType()) {
                queryValue.convert(property.userType());
            }
            if (ORM_Impl::isPrimitiveString(property.userType())) {
                queryValue.convert(property.userType());
            }
            if (ORM_Impl::isPrimitiveRaw(property.userType())) {
                QVariant temp;
                QByteArray array = queryValue.toByteArray();
                QDataStream stream(&array,QIODevice::ReadWrite);
                stream >> temp;
                queryValue = temp;
            }
            ORM_Impl::write(isQObject, variant, property, queryValue);
        }
        resultList << variant;
    }
    query.finish();
    return QVariant(resultList);
}

QVariant ORM::meta_get(const QMetaObject &meta, long long id, const QString &parent_name, const QString &property_name)
{
    QString tableName = generate_table_name(parent_name, property_name, QString(meta.className()),
                                             QueryType::Select);
    int classType = QMetaType::type(meta.className());
    bool isQObject = ORM_Impl::isQObject(meta);
    if (!ORM_Impl::withRowid(meta)) {
        qWarning() << "get: operation needs ROWID to work.";
        return QVariant();
    }
    if (!getQueries.contains(tableName)) {
        QStringList queryColumns;
        QList<int> queryTypes;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (ORM_Impl::isIgnored(property.userType())) {
                continue;
            }
            if (!property.isStored() || ORM_Impl::isTypeForTable(property.userType())) {
                continue;
            }
            if (ORM_Impl::isPrimitiveString(property.userType())) {
                queryColumns << property.name();
                queryTypes << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(property.userType())) {
                queryColumns << property.name();
                queryTypes << qMetaTypeId<QByteArray>();
                continue;
            }
            queryColumns << property.name();
            queryTypes << property.userType();
        }
        if (!queryColumns.size()) {
            return QVariant();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_get_query(parent_name, property_name, meta.className(),
                                            queryColumns, queryTypes));
        getQueries[tableName] = query;
    }
    QSqlQuery & query = getQueries[tableName];
    query.bindValue(":" + normalize(ORM_Impl::orm_rowidName, QueryType::Select), id);
    query.exec();
    ORM_Impl::checkQueryErrors(query, meta.className(), tableName, property_name);
    QVariantList list;
    while (query.next()) {
        QVariant variant;
        if (isQObject) {
            if (!ORM_Impl::objectMap.contains(classType)) {
                qWarning() << "get: " << meta.className() << " is not registered. "
                              "Call registerQObjectORM<" << meta.className() << ">(); first";
                break;
            }
            variant = QVariant::fromValue(ORM_Impl::objectMap[classType]());
        }
        else {
            variant = QVariant((classType), nullptr);
        }
        if (!variant.isValid()){
            qWarning() << "get: Unable to create instance of type " << meta.className();
            break;
        }
        if (isQObject && variant.value<QObject*>() == nullptr) {
            qWarning() << "get: Unable to create instance of QObject " << meta.className();
            break;
        }
        long long ormRowid = query.value(normalize(ORM_Impl::orm_rowidName, QueryType::Select)).toLongLong();
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (ORM_Impl::isIgnored(property.userType())) {
                continue;
            }
            if (ORM_Impl::shouldSkipWriteMeta(property)) {
                continue;
            }
            if (!ORM_Impl::isPrimitive(property.userType())) {
                if (ORM_Impl::isGadget(property.userType())) {
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(property.userType());
                    if (metaObject) {
                        QVariantList variantList = meta_select(*metaObject, tableName, QString(property.name()), ormRowid).toList();
                        if (variantList.size()) {
                            ORM_Impl::write(isQObject, variant, property, variantList.first());
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isPointer(property.userType())) {
                    if (ORM_Impl::isWeakPointer(property.userType())) {
                        continue;
                    }
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(property.userType()));
                    if (metaObject) {
                        QVariantList variantList = meta_select(*metaObject, tableName, QString(property.name()), ormRowid).toList();
                        if (variantList.size()) {
                            if (variantList.first().value<void*>()) {
                                ORM_Impl::write(isQObject, variant, property, variantList.first());
                            }
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isSequentialContainer(property.userType())) {
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(property.userType()));
                    if (metaObject) {
                        ORM_Impl::write(isQObject, variant, property, meta_select(*metaObject, tableName, QString(property.name()), ormRowid).toList());
                        continue;
                    }
                }
                if (ORM_Impl::isAssociativeContainer(property.userType()) || ORM_Impl::isPair(property.userType())) {
                    QVariant variantResult = meta_select_pair(property.userType(), tableName, QString(property.name()), ormRowid);
                    if (variantResult.canConvert(property.userType())) {
                        variantResult.convert(property.userType());
                        ORM_Impl::write(isQObject, variant, property, variantResult);
                    }
                    continue;
                }
            }
            QVariant queryValue = query.value(normalize(property.name(), QueryType::Select));
            if (property.isEnumType()) {
                queryValue.convert(property.userType());
            }
            if (ORM_Impl::isPrimitiveString(property.userType())) {
                queryValue.convert(property.userType());
            }
            if (ORM_Impl::isPrimitiveRaw(property.userType())) {
                QVariant temp;
                QByteArray array = queryValue.toByteArray();
                QDataStream stream(&array,QIODevice::ReadWrite);
                stream >> temp;
                queryValue = temp;
            }
            ORM_Impl::write(isQObject, variant, property, queryValue);
        }
        list << variant;
    }
    query.finish();
    return QVariant(list);
}
QVariant ORM::meta_select_pair  (int meta_type_id, QString const& parent_name, QString const& property_name,
                                 long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(meta_type_id);
    QString tableName = generate_table_name(parent_name, property_name, className, QueryType::Select);
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(meta_type_id);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(meta_type_id);
    if (!selectQueries.contains(tableName)) {
        QStringList queryColumns;
        QList<int> queryTypes;
        queryColumns << ORM_Impl::orm_rowidName;
        queryTypes << qMetaTypeId<long long>();
        for (int column = 0; column < 2; ++column) {
            int type = column == 0 ? keyType : valueType;
            QString name = column == 0 ? "key" : "value";
            if (ORM_Impl::isIgnored(type)) {
                continue;
            }
            if (ORM_Impl::isTypeForTable(type)) {
                continue;
            }
            queryColumns << name;
            if (ORM_Impl::isPrimitiveString(type)) {
                queryTypes << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(type)) {
                queryTypes << qMetaTypeId<QByteArray>();
                continue;
            }
            queryTypes << type;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_select_query(parent_name, property_name, className,
                                            queryColumns, queryTypes, parent_orm_rowid != 0));
        selectQueries[tableName] = query;
    }
    QSqlQuery & query = selectQueries[tableName];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, className, tableName, property_name);
    QVariantList resultList;
    while (query.next()) {
        orm_containers::ORM_QVariantPair pair;
        long long pairOrmRowID = query.value(normalize(ORM_Impl::orm_rowidName, QueryType::Select)).toLongLong();
        for (int column = 0; column < 2; ++column) {
            int type = column == 0 ? keyType : valueType;
            QString name = column == 0 ? "key" : "value";
            if (ORM_Impl::isIgnored(type)) {
                continue;
            }
            if (!ORM_Impl::isPrimitive(type)) {
                if (ORM_Impl::isGadget(type)) {
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(type);
                    if (metaObject) {
                        QVariantList variantList = meta_select(*metaObject, tableName, name, pairOrmRowID).toList();
                        if (variantList.size()) {
                            pair[column] = variantList.first();
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isPointer(type)) {
                    if (ORM_Impl::isWeakPointer(type)) {
                        continue;
                    }
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(type));
                    if (metaObject) {
                        QVariantList variantList = meta_select(*metaObject, tableName, name, pairOrmRowID).toList();
                        if (variantList.size()) {
                            if (variantList.first().value<void*>()) {
                                pair[column] = variantList.first();
                            }
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isSequentialContainer(type)) {
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(type));
                    if (metaObject) {
                        pair[column] = meta_select(*metaObject, tableName, name, pairOrmRowID).toList();
                        continue;
                    }
                }
                if (ORM_Impl::isAssociativeContainer(type) || ORM_Impl::isPair(type)) {
                    QVariant variantResult = meta_select_pair(type, tableName, name, pairOrmRowID);
                    if (variantResult.canConvert(type)) {
                        variantResult.convert(type);
                        pair[column] = variantResult;
                    }
                    continue;
                }
            }
            QVariant queryResult = query.value(normalize(name, QueryType::Select));
            if (QMetaType::typeFlags(type).testFlag(QMetaType::IsEnumeration)) {
                queryResult.convert(type);
            }
            if (ORM_Impl::isPrimitiveString(type)) {
                queryResult.convert(type);
            }
            if (ORM_Impl::isPrimitiveRaw(type)) {
                QVariant temp;
                QByteArray array = queryResult.toByteArray();
                QDataStream stream(&array,QIODevice::ReadWrite);
                stream >> temp;
                queryResult = temp;
            }
            pair[column] = queryResult;
        }
        resultList << QVariant::fromValue(pair);
    }
    query.finish();
    return QVariant(resultList);
}

void     ORM::meta_insert(const QMetaObject &meta, QVariant &value, QString const& parent_name,
                          QString const& property_name, long long parent_orm_rowid)
{
    QString tableName = generate_table_name(parent_name, property_name, QString(meta.className()),
                                             QueryType::Insert);
    bool isQObject = meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    long long ormRowid = 0;
    bool withOrmRowID = ORM_Impl::withRowid(meta);
    if (!insertQueries.contains(tableName)) {
        QStringList queryColumns;
        QList<int> queryTypes;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (ORM_Impl::isIgnored(property.userType())) {
                continue;
            }
            if (ORM_Impl::isTypeForTable(property.userType()) || !property.isStored() || !property.isReadable()) {
                continue;
            }
            if (ORM_Impl::isRowID(property)) {
                ormRowid = ORM_Impl::read(isQObject, value, property).toLongLong();
                if (ormRowid == 0 || ormRowid == -1) {
                    continue;
                }
            }
            queryColumns << property.name();
            if (ORM_Impl::isPrimitiveString(property.userType())) {
                queryTypes << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(property.userType())) {
                queryTypes << qMetaTypeId<QByteArray>();
                continue;
            }
            queryTypes << property.userType();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_insert_query(parent_name, property_name, meta.className(),
                                            queryColumns, queryTypes, parent_orm_rowid != 0));
        insertQueries[tableName] = query;
    }
    QSqlQuery & query = insertQueries[tableName];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty property = meta.property(i);
        if (ORM_Impl::isIgnored(property.userType())) {
            continue;
        }
        if (!property.isStored() || !property.isReadable() || ORM_Impl::isTypeForTable(property.userType())) {
            continue;
        }
        if (ORM_Impl::isRowID(property) && (ormRowid == 0 || ormRowid == -1)) {
            continue;
        }
        QVariant temp = ORM_Impl::read(isQObject, value, property);
        if (ORM_Impl::isPrimitiveString(property.userType())) {
            temp.convert(qMetaTypeId<QString>());
        }
        if (ORM_Impl::isPrimitiveRaw(property.userType())) {
            QByteArray array;
            QDataStream stream(&array, QIODevice::ReadWrite);
            stream << temp;
            temp = array;
            //w.convert(qMetaTypeId<QByteArray>());
        }
        query.bindValue(QString(":") + property.name(), temp);
    }
    query.exec();
    if (ORM_Impl::checkQueryErrors(query, meta.className(), tableName, property_name)) {
        return;
    }

    if (withOrmRowID) {
        ormRowid = get_last_inserted_rowid(query);
        //qDebug() << ormRowid << meta.className() << parent_orm_rowid << parent_name;
        if (ormRowid) {
            for (int i = 0; i < meta.propertyCount(); ++i) {
                QMetaProperty property = meta.property(i);
                if (ORM_Impl::isIgnored(property.userType())) {
                    continue;
                }
                if (!property.isStored() || !property.isReadable() || ORM_Impl::isPrimitive(property.userType())) {
                    continue;
                }
                if (ORM_Impl::isGadget(property.userType())) {
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(property.userType());
                    if (metaObject) {
                        QVariant gadget = ORM_Impl::read(isQObject, value, property);
                        meta_insert(*metaObject, gadget, tableName, QString(property.name()), ormRowid);
                        continue;
                    }
                }
                if (ORM_Impl::isPointer(property.userType())) {
                    if (ORM_Impl::isWeakPointer(property.userType())) {
                        continue;
                    }
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(property.userType()));
                    if (metaObject) {
                        QVariant pointer = property.readOnGadget(value.value<void*>());
                        //qDebug() << pointer << pointer.data();
                        if (pointer.value<void*>()) {
                            pointer.convert(qMetaTypeId<void*>());
                            meta_insert(*metaObject, pointer, tableName, QString(property.name()), ormRowid);
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isSequentialContainer(property.userType())) {
                    const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(property.userType()));
                    if (metaObject) {
                        QVariant variantList = ORM_Impl::read(isQObject, value, property);
                        if (variantList.isValid()) {
                            QSequentialIterable sequentialIterable = variantList.value<QSequentialIterable>();
                            for (QVariant listElement : sequentialIterable) {
                                meta_insert(*metaObject, listElement, tableName, QString(property.name()), ormRowid);
                            }
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isAssociativeContainer(property.userType()) || ORM_Impl::isPair(property.userType())) {
                    QVariant variantMap = ORM_Impl::read(isQObject, value, property);
                    if (variantMap.canConvert<orm_containers::ORM_QVariantPair>()) {
                        variantMap.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                        meta_insert_pair(property.userType(), variantMap, tableName, QString(property.name()), ormRowid);
                        continue;
                    }
                    if (variantMap.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                        QList<orm_containers::ORM_QVariantPair> pairList = variantMap.value<QList<orm_containers::ORM_QVariantPair>>();
                        for (orm_containers::ORM_QVariantPair & ormPairValue : pairList) {
                            QVariant variantPair = QVariant::fromValue(ormPairValue);
                            meta_insert_pair(property.userType(), variantPair, tableName, QString(property.name()), ormRowid);
                        }
                        continue;
                    }
                }
            }
        }
    }
    query.finish();
}
void     ORM::meta_insert_pair  (int meta_type_id, QVariant &v, QString const& parent_name,
                                 QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(meta_type_id);
    QString tableName = generate_table_name(parent_name, property_name, className, QueryType::Insert);
    orm_containers::ORM_QVariantPair ormPair = v.value<orm_containers::ORM_QVariantPair>();
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(meta_type_id);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(meta_type_id);
    if (!insertQueries.contains(tableName)) {
        QStringList queryColumns;
        QList<int> queryTypes;
        for (int column = 0; column < 2; ++column) {
            int type = column == 0 ? keyType : valueType;
            QString name = column == 0 ? "key" : "value";
            if (ORM_Impl::isIgnored(type)) {
                continue;
            }
            if (ORM_Impl::isTypeForTable(type)) {
                continue;
            }
            queryColumns << name;
            if (ORM_Impl::isPrimitiveString(type)) {
                queryTypes << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(type)) {
                queryTypes << qMetaTypeId<QByteArray>();
                continue;
            }
            queryTypes << type;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_insert_query(parent_name, property_name, className,
                                            queryColumns, queryTypes, parent_orm_rowid != 0));
        insertQueries[tableName] = query;
    }
    QSqlQuery & query = insertQueries[tableName];
    query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    for (int column = 0; column < 2; ++column) {
        int type = column == 0 ? keyType : valueType;
        QString name = column == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(type)) {
            continue;
        }
        if (ORM_Impl::isTypeForTable(type)) {
            continue;
        }
        QVariant variant = ormPair[column];
        if (ORM_Impl::isPrimitiveString(type)) {
            variant.convert(qMetaTypeId<QString>());
        }
        if (ORM_Impl::isPrimitiveRaw(type)) {
            QByteArray array;
            QDataStream stream(&array,QIODevice::ReadWrite);
            stream << variant;
            variant = array;
        }
        query.bindValue(QString(":") + name, variant);
    }
    query.exec();
    if (ORM_Impl::checkQueryErrors(query, className, tableName, property_name)) {
        return;
    }

    long long pairOrmRowid = get_last_inserted_rowid(query);
    if (!pairOrmRowid) {
        return;
    }
    for (int column = 0; column < 2; ++column) {
        int type = column == 0 ? keyType : valueType;
        QString name = column == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(type)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(type)) {
            continue;
        }
        if (ORM_Impl::isGadget(type)) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(type);
            if (metaObject) {
                meta_insert(*metaObject, ormPair[column], tableName, name, pairOrmRowid);
                continue;
            }
        }
        if (ORM_Impl::isPointer(type)) {
            if (ORM_Impl::isWeakPointer(type)) {
                continue;
            }
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(type));
            if (metaObject) {
                QVariant pointer = ormPair[column];
                if (pointer.value<void*>()) {
                    pointer.convert(qMetaTypeId<void*>());
                    meta_insert(*metaObject, pointer, tableName, name, pairOrmRowid);
                }
                continue;
            }
        }
        if (ORM_Impl::isSequentialContainer(type)) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(type));
            if (metaObject) {
                QVariant variantList = ormPair[column];
                if (variantList.isValid()) {
                    QSequentialIterable sequentialIterable = variantList.value<QSequentialIterable>();
                    for (QVariant listElement : sequentialIterable) {
                        meta_insert(*metaObject, listElement, tableName, name, pairOrmRowid);
                    }
                }
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(type) || ORM_Impl::isPair(type)) {
            QVariant variantMap = ormPair[column];
            if (variantMap.canConvert<orm_containers::ORM_QVariantPair>()) {
                variantMap.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                meta_insert_pair(type, variantMap, tableName, name, pairOrmRowid);
                continue;
            }
            if (variantMap.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                QList<orm_containers::ORM_QVariantPair> pairList = variantMap.value<QList<orm_containers::ORM_QVariantPair>>();
                for (orm_containers::ORM_QVariantPair & ormPairValue : pairList) {
                    QVariant variantPair = QVariant::fromValue(ormPairValue);
                    meta_insert_pair(type, variantPair, tableName, name, pairOrmRowid);
                }
                continue;
            }
        }
    }
}

void     ORM::meta_update(const QMetaObject &meta, QVariant &value, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString tableName = generate_table_name(parent_name, property_name, QString(meta.className()), QueryType::Update);
    bool isQObject = ORM_Impl::isQObject(meta);
    if (!ORM_Impl::withRowid(meta)) {
        qWarning() << "update: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    long long rowid = 0;
    if (!updateQueries.contains(tableName)) {
        QStringList queryColumns;
        QList<int> queryTypes;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (ORM_Impl::isIgnored(property.userType())) {
                continue;
            }
            if (!property.isStored() || !property.isReadable() || ORM_Impl::isTypeForTable(property.userType())) {
                continue;
            }
            if (ORM_Impl::isRowID(property.name())) {
                rowid = ORM_Impl::read(isQObject, value, property).toLongLong();
                continue;
            }
            queryColumns << property.name();
            if (ORM_Impl::isPrimitiveString(property.userType())) {
                queryTypes << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(property.userType())) {
                queryTypes << qMetaTypeId<QByteArray>();
                continue;
            }
            queryTypes << property.userType();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_update_query(parent_name, property_name, meta.className(),
                                            queryColumns, queryTypes, parent_orm_rowid != 0));
        updateQueries[tableName] = query;
    }
    QSqlQuery & query = updateQueries[tableName];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty property = meta.property(i);
        if (ORM_Impl::isIgnored(property.userType())) {
            continue;
        }
        if (!property.isStored() || !property.isReadable() || ORM_Impl::isTypeForTable(property.userType())) {
            continue;
        }
        QVariant temp = ORM_Impl::read(isQObject, value, property);
        if (ORM_Impl::isPrimitiveString(property.userType())) {
            temp.convert(qMetaTypeId<QString>());
        }
        if (ORM_Impl::isPrimitiveRaw(property.userType())) {
            QByteArray array;
            QDataStream stream(&array,QIODevice::ReadWrite);
            stream << temp;
            temp = array;
        }
        query.bindValue(QString(":") + property.name(), temp);
    }
    query.exec();
    if (ORM_Impl::checkQueryErrors(query, meta.className(), tableName, property_name)) {
        return;
    }

    //long long orm_rowid = get_changes(query);
    if (get_changes(query) == 0) {
        //meta_insert(meta, value, parent_name, property_name, parent_orm_rowid);
        //return;
    }

    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty property = meta.property(i);
        if (ORM_Impl::isIgnored(property.userType())) {
            continue;
        }
        if (!property.isStored() || !property.isReadable() || ORM_Impl::isPrimitive(property.userType())) {
            continue;
        }
        if (ORM_Impl::isGadget(property.userType())) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(property.userType());
            if (metaObject) {
                QVariant gadget = ORM_Impl::read(isQObject, value, property);
                meta_update(*metaObject, gadget, tableName, QString(property.name()), rowid);
                continue;
            }
        }
        if (ORM_Impl::isPointer(property.userType())) {
            if (ORM_Impl::isWeakPointer(property.userType())) {
                continue;
            }
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(property.userType()));
            if (metaObject) {
                QVariant pointer = ORM_Impl::read(isQObject, value, property);
                if (pointer.value<void*>()) {
                    pointer.convert(qMetaTypeId<void*>());
                    meta_update(*metaObject, pointer, tableName, QString(property.name()), rowid);
                }
                continue;
            }
        }
        if (ORM_Impl::isSequentialContainer(property.userType())) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(property.userType()));
            if (metaObject) {
                QVariant variantList = ORM_Impl::read(isQObject, value, property);
                if (variantList.isValid()) {
                    QSequentialIterable sequentialIterable = variantList.value<QSequentialIterable>();
                    for (QVariant listElement : sequentialIterable) {
                        meta_update(*metaObject, listElement, tableName, QString(property.name()), rowid);
                    }
                }
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(property.userType()) || ORM_Impl::isPair(property.userType())) {
            QVariant variantMap = ORM_Impl::read(isQObject, value, property);
            if (variantMap.canConvert<orm_containers::ORM_QVariantPair>()) {
                variantMap.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                //meta_delete_pair(property.userType(), variantMap, tableName, QString(property.name()), rowid);
                meta_update_pair(property.userType(), variantMap, tableName, QString(property.name()), rowid);
                continue;
            }
            if (variantMap.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                QList<orm_containers::ORM_QVariantPair> pairList = variantMap.value<QList<orm_containers::ORM_QVariantPair>>();
                for (orm_containers::ORM_QVariantPair & ormPairValue : pairList) {
                    QVariant variantPair = QVariant::fromValue(ormPairValue);
                    meta_update_pair(property.userType(), variantPair, tableName, QString(property.name()), rowid);
                }
                continue;
            }
        }
    }
    query.finish();
}
void     ORM::meta_update_pair  (int meta_type_id, QVariant &value, QString const& parent_name,
                                 QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(meta_type_id);
    QString tableName = generate_table_name(parent_name, property_name, className, QueryType::Update);
    orm_containers::ORM_QVariantPair ormPair = value.value<orm_containers::ORM_QVariantPair>();
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(meta_type_id);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(meta_type_id);
    if (!updateQueries.contains(tableName)) {
        QStringList queryColumns;
        QList<int> queryTypes;
        for (int column = 0; column < 2; ++column) {
            int type = column == 0 ? keyType : valueType;
            QString name = column == 0 ? "key" : "value";
            if (ORM_Impl::isIgnored(type)) {
                continue;
            }
            if (ORM_Impl::isTypeForTable(type)) {
                continue;
            }
            queryColumns << name;
            if (ORM_Impl::isPrimitiveString(type)) {
                queryTypes << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(type)) {
                queryTypes << qMetaTypeId<QByteArray>();
                continue;
            }
            queryTypes << type;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_update_query(parent_name, property_name, className,
                                            queryColumns, queryTypes, parent_orm_rowid != 0));
        updateQueries[tableName] = query;
    }
    QSqlQuery & query = updateQueries[tableName];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    for (int column = 0; column < 2; ++column) {
        int type = column == 0 ? keyType : valueType;
        QString name = column == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(type)) {
            continue;
        }
        if (ORM_Impl::isTypeForTable(type)) {
            continue;
        }
        QVariant temp = ormPair[column];
        if (ORM_Impl::isPrimitiveString(type)) {
            temp.convert(qMetaTypeId<QString>());
        }
        if (ORM_Impl::isPrimitiveRaw(type)) {
            QByteArray array;
            QDataStream stream(&array,QIODevice::ReadWrite);
            stream << temp;
            temp = array;
        }
        query.bindValue(QString(":") + name, temp);
    }
    query.exec();
    if (ORM_Impl::checkQueryErrors(query, className, tableName, property_name)) {
        return;
    }

    query.finish();
    if (get_changes(query) == 0) {
        meta_insert_pair(meta_type_id, value, parent_name, property_name, parent_orm_rowid);
        return;
    }

    for (int column = 0; column < 2; ++column) {
        int type = column == 0 ? keyType : valueType;
        QString name = column == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(type)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(type)) {
            continue;
        }
        if (ORM_Impl::isGadget(type)) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(type);
            if (metaObject) {
                QVariant gadget = ormPair[column];
                meta_update(*metaObject, gadget, tableName, name, 0);
                continue;
            }
        }
        if (ORM_Impl::isPointer(type)) {
            if (ORM_Impl::isWeakPointer(type)) {
                continue;
            }
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(type));
            if (metaObject) {
                QVariant pointer = ormPair[column];
                if (pointer.value<void*>()) {
                    pointer.convert(qMetaTypeId<void*>());
                    meta_update(*metaObject, pointer, tableName, name, 0);
                }
                continue;
            }
        }
        if (ORM_Impl::isSequentialContainer(type)) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(type));
            if (metaObject) {
                QVariant variantList = ormPair[column];
                if (variantList.isValid()) {
                    QSequentialIterable sequentialIterable = variantList.value<QSequentialIterable>();
                    for (QVariant listElement : sequentialIterable) {
                        meta_update(*metaObject, listElement, tableName, name, 0);
                    }
                }
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(type) || ORM_Impl::isPair(type)) {
            QVariant variantMap = ormPair[column];
            if (variantMap.canConvert<orm_containers::ORM_QVariantPair>()) {
                variantMap.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                meta_update_pair(type, variantMap, tableName, name, 0);
                continue;
            }
            if (variantMap.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                QList<orm_containers::ORM_QVariantPair> pairList = variantMap.value<QList<orm_containers::ORM_QVariantPair>>();
                for (orm_containers::ORM_QVariantPair & ormPairValue : pairList) {
                    QVariant variantPair = QVariant::fromValue(ormPairValue);
                    meta_update_pair(type, variantPair, tableName, name, 0);
                }
                continue;
            }
        }
    }
}

void     ORM::meta_delete(const QMetaObject &meta, QVariant &value, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString tableName = generate_table_name(parent_name, property_name, QString(meta.className()), QueryType::Delete);
    bool isQObject = ORM_Impl::isQObject(meta);
    long long ormRowid = 0;
    if (!ORM_Impl::withRowid(meta)) {
        qWarning() << "delete: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty property = meta.property(i);
        if (ORM_Impl::isIgnored(property.userType())) {
            continue;
        }
        if (ORM_Impl::isTypeForTable(property.userType())) {
            continue;
        }
        if (ORM_Impl::isRowID(property)) {
            ormRowid = ORM_Impl::read(isQObject, value, property).toLongLong();
        }
    }
    if (!ormRowid && !parent_orm_rowid) {
        return;
    }
    if (ormRowid) {
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty property = meta.property(i);
            if (ORM_Impl::isIgnored(property.userType())) {
                continue;
            }
            if (!property.isStored() || !property.isReadable() || ORM_Impl::isPrimitive(i)) {
                continue;
            }
            if (ORM_Impl::isGadget(property.userType())) {
                const QMetaObject * metaObject = QMetaType::metaObjectForType(property.userType());
                if (metaObject) {
                    QVariant gadget = ORM_Impl::read(isQObject, value, property);
                    meta_delete(*metaObject, gadget, tableName, QString(property.name()), ormRowid);
                    continue;
                }
            }
            if (ORM_Impl::isPointer(property.userType())) {
                if (ORM_Impl::isWeakPointer(property.userType())) {
                    continue;
                }
                const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(property.userType()));
                if (metaObject) {
                    QVariant pointer = ORM_Impl::read(isQObject, value, property);
                    meta_delete(*metaObject, pointer, tableName, QString(property.name()), ormRowid);
                    continue;
                }
            }
            if (ORM_Impl::isSequentialContainer(property.userType())) {
                const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(property.userType()));
                if (metaObject) {
                    QSequentialIterable sequentialIterable = ORM_Impl::read(isQObject, value, property).value<QSequentialIterable>();
                    for (QVariant listElement : sequentialIterable) {
                        meta_delete(*metaObject, listElement, tableName, QString(property.name()), ormRowid);
                    }
                    continue;
                }
            }
            if (ORM_Impl::isAssociativeContainer(property.userType()) || ORM_Impl::isPair(property.userType())) {
                QVariant variantMap = ORM_Impl::read(isQObject, value, property);
                if (variantMap.canConvert<orm_containers::ORM_QVariantPair>()) {
                    variantMap.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                    meta_delete_pair(property.userType(), variantMap, tableName, QString(property.name()), ormRowid);
                    continue;
                }
                if (variantMap.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                    QList<orm_containers::ORM_QVariantPair> pairList = variantMap.value<QList<orm_containers::ORM_QVariantPair>>();
                    for (orm_containers::ORM_QVariantPair & ormPairValue : pairList) {
                        QVariant variantPair = QVariant::fromValue(ormPairValue);
                        meta_delete_pair(property.userType(), variantPair, tableName, QString(property.name()), ormRowid);
                    }
                    continue;
                }
            }
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(generate_delete_query(parent_name, property_name, QString(meta.className()), parent_orm_rowid));
    if (!parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_rowidName, ormRowid);
    }
    else {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, meta.className(), tableName, property_name);
}
void     ORM::meta_delete_pair  (int meta_type_id, QVariant &value, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(meta_type_id);
    QString tableName = generate_table_name(parent_name, property_name, className, QueryType::Delete);
    orm_containers::ORM_QVariantPair ormPair = value.value<orm_containers::ORM_QVariantPair>();
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(meta_type_id);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(meta_type_id);
    for (int column = 0; column < 2; ++column) {
        int type = column == 0 ? keyType : valueType;
        QString name = column == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(type)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(type)) {
            continue;
        }
        if (ORM_Impl::isGadget(type)) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(type);
            if (metaObject) {
                meta_delete(*metaObject, ormPair[column], tableName, name, 0);
                continue;
            }
        }
        if (ORM_Impl::isPointer(type)) {
            if (ORM_Impl::isWeakPointer(type)) {
                continue;
            }
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(type));
            if (metaObject) {
                meta_delete(*metaObject, ormPair[column], tableName, name, 0);
                continue;
            }
        }
        if (ORM_Impl::isSequentialContainer(type)) {
            const QMetaObject * metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(type));
            if (metaObject) {
                QSequentialIterable sequentialIterable = ormPair[column].value<QSequentialIterable>();
                for (QVariant listElement : sequentialIterable) {
                    meta_delete(*metaObject, listElement, tableName, name, 0);
                }
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(type) || ORM_Impl::isPair(type)) {
            QVariant variantMap = ormPair[column];
            if (variantMap.canConvert<orm_containers::ORM_QVariantPair>()) {
                variantMap.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                meta_delete_pair(type, variantMap, tableName, name, 0);
                continue;
            }
            if (variantMap.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                QList<orm_containers::ORM_QVariantPair> pairList = variantMap.value<QList<orm_containers::ORM_QVariantPair>>();
                for (orm_containers::ORM_QVariantPair & ormPairValue : pairList) {
                    QVariant variantPair = QVariant::fromValue(ormPairValue);
                    meta_delete_pair(type, variantPair, tableName, name, 0);
                }
                continue;
            }
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(generate_delete_query(parent_name, property_name, className, parent_orm_rowid));
    if (!parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_rowidName, 0);
    }
    else {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, className, tableName, property_name);
}

void     ORM::meta_drop  (const QMetaObject &meta, QString const& parent_name, const QString &property_name)
{
    QString tableName = generate_table_name(parent_name, property_name, QString(meta.className()), QueryType::Drop);
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty property = meta.property(i);
        if (ORM_Impl::isIgnored(property.userType())) {
            continue;
        }
        if (!property.isStored() || ORM_Impl::isPrimitive(property.userType())) {
            continue;
        }
        const QMetaObject * metaObject = nullptr;
        if (ORM_Impl::isGadget(property.userType())) {
            metaObject = QMetaType::metaObjectForType(property.userType());
        }
        if (ORM_Impl::isPointer(property.userType())) {
            if (ORM_Impl::isWeakPointer(property.userType())) {
                continue;
            }
           metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(property.userType()));
        }
        if (ORM_Impl::isSequentialContainer(property.userType())) {
            metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(property.userType()));
        }
        if (ORM_Impl::isAssociativeContainer(property.userType()) || ORM_Impl::isPair(property.userType())) {
            meta_drop_pair(property.userType(), tableName, property.name());
            continue;
        }
        if (metaObject) {
            meta_drop(*metaObject, tableName, property.name());
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_drop_query(parent_name, property_name, QString(meta.className())));
    ORM_Impl::checkQueryErrors(query, meta.className(), tableName);
}
void     ORM::meta_drop_pair    (int meta_type_id, QString const& parent_name, QString const& property_name)
{
    QString className = QMetaType::typeName(meta_type_id);
    QString tableName = generate_table_name(parent_name, property_name, className, QueryType::Drop);
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(meta_type_id);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(meta_type_id);
    for (int column = 0; column < 2; ++column) {
        int type = column == 0 ? keyType : valueType;
        QString name = column == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(type)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(type)) {
            continue;
        }
        const QMetaObject * metaObject = nullptr;
        if (ORM_Impl::isGadget(type)) {
            metaObject = QMetaType::metaObjectForType(type);
        }
        if (ORM_Impl::isPointer(type)) {
            if (ORM_Impl::isWeakPointer(type)) {
                continue;
            }
           metaObject = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(type));
        }
        if (ORM_Impl::isSequentialContainer(type)) {
            metaObject = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(type));
        }
        if (ORM_Impl::isAssociativeContainer(type)) {
            meta_drop_pair(meta_type_id, tableName, name);
            continue;
        }
        if (metaObject) {
            meta_drop(*metaObject, tableName, name);
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_drop_query(parent_name, property_name, className));
    ORM_Impl::checkQueryErrors(query, className, tableName);
}










QDebug operator<<(QDebug dbg, const ORM_Config &)
{
    QDebugStateSaver s(dbg);
    dbg.nospace().noquote() << "ORM Config:\n";
    dbg.nospace().noquote() << "General:\n";
    dbg.nospace().noquote() << "  ROW ID field name: \"" << ORM_Impl::orm_rowidName << "\"\n";
    dbg.nospace().noquote() << "  Parent foreing key field name: \"" << ORM_Impl::orm_parentRowidName << "\"\n";
    dbg.nospace().noquote() << "  ORM initiated " << !ORM_Impl::orm_once << "\n";

    dbg.nospace().noquote() << "Pair types:\n";
    for (auto i : ORM_Impl::pairTypes) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    dbg << "Associative container types:\n" ;
    for (auto i = ORM_Impl::assContTypeMap.begin(); i != ORM_Impl::assContTypeMap.end(); ++i) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i.key()) << " (" << i.key() << ") -> <" <<
                                   QMetaType::typeName(i.value().first) << " (" << i.value().first << "), " <<
                                   QMetaType::typeName(i.value().second) << " (" << i.value().second << ")>\n";
    }
    dbg << "QObjects:\n" ;
    for (auto i : ORM_Impl::objectMap.keys()) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << ")\n";
    }
    //    static QMap<int, orm_pointers::ORMPointerStub> pointerMap;
    dbg.nospace().noquote() << "Registred pointers:\n";
    for (auto i = ORM_Impl::pointerMap.begin(); i != ORM_Impl::pointerMap.end(); ++i) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i.key()) << " (" << i.key() << ") : "
                                   "Raw[" << i.value().pT << "], "
                                   "QShared[" << i.value().ST << "], "
                                   "QWeak[" << i.value().WT << "], "
                                   "std::shared[" << i.value().sT << "], "
                                   "std::weak[" << i.value().wT << "]\n";
    }

    dbg.nospace().noquote() << "Ignored types:\n";
    for (auto i : ORM_Impl::ignoredTypes) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    dbg.nospace().noquote() << "Primitive types (auto):\n";
    for (auto i : ORM_Impl::primitiveContainers) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    dbg.nospace().noquote() << "Primitive types (as string):\n";
    for (auto i : ORM_Impl::primitiveStringContainers) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    dbg.nospace().noquote() << "Primitive types (as blob):\n";
    for (auto i : ORM_Impl::primitiveRawContainers) {
        dbg.nospace().noquote() << "  " << QMetaType::typeName(i) << " (" << i << "), \n";
    }
    return dbg;


}
