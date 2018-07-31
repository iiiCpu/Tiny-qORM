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
    static QSet<int> pairTypes;
    static QMap<int, QPair<int, int>> assContTypeMap;
    static QMap<int, orm_qobjects::creators> objectMap;
    static QMap<int, orm_pointers::ORMPointerStub> pointerMap;
    static QSet<int> ignoredTypes;
    static QSet<int> primitiveContainers;
    static QSet<int> primitiveStringContainers;
    static QSet<int> primitiveRawContainers;

    static QString orm_rowidName = QString("orm_rowid");
    static QString orm_parentRowidName = QString("parent_orm_rowid");
    static bool orm_once = true;


    inline bool isEnumeration(int metaTypeID)
    {
        QMetaType metatype(metaTypeID);
        if (metatype.flags() & QMetaType::IsEnumeration) {
            return true;
        }
        return false;
    }

    inline bool isGadget(int metaTypeID)
    {
        QMetaType metatype(metaTypeID);
        if (metatype.flags() & QMetaType::IsGadget) {
            return true;
        }
        return false;
    }

    inline bool isPointer(int metaTypeID)
    {
        QMetaType metatype(metaTypeID);
        if (metatype.flags() & QMetaType::PointerToGadget || metatype.flags() & QMetaType::PointerToQObject) {
            return true;
        }
        if (ORM_Impl::pointerMap.contains(metaTypeID) && ORM_Impl::pointerMap[metaTypeID].T != metaTypeID) {
            return true;
        }
        return false;
    }
    inline bool isWeakPointer(int metaTypeID)
    {
        if (ORM_Impl::pointerMap.contains(metaTypeID) &&
                (ORM_Impl::pointerMap[metaTypeID].WT == metaTypeID ||
                 ORM_Impl::pointerMap[metaTypeID].wT == metaTypeID)) {
            return true;
        }
        return false;
    }

    inline int pointerToValue(int metaTypeID)
    {
        if (ORM_Impl::pointerMap.contains(metaTypeID)) {
            return ORM_Impl::pointerMap[metaTypeID].T;
        }
        return 0;
    }

    inline bool shouldSkip(int metaTypeID)
    {
        return ORM_Impl::ignoredTypes.contains(metaTypeID);
    }
    inline bool isPrimitiveType(int metaTypeID)
    {
        return ORM_Impl::primitiveContainers.contains(metaTypeID);
    }
    inline bool isPrimitiveString(int metaTypeID)
    {
        return ORM_Impl::primitiveStringContainers.contains(metaTypeID);
    }
    inline bool isPrimitiveRaw(int metaTypeID)
    {
        return ORM_Impl::primitiveRawContainers.contains(metaTypeID);
    }

    inline bool isSequentialContainer(int metaTypeID)
    {
        return QMetaType::hasRegisteredConverterFunction(metaTypeID, qMetaTypeId<QtMetaTypePrivate::QSequentialIterableImpl>());
    }

    inline bool isPair(int metaTypeID)
    {
        return ORM_Impl::pairTypes.contains(metaTypeID);
    }
    inline bool isAssociativeContainer(int metaTypeID)
    {
        return ORM_Impl::assContTypeMap.contains(metaTypeID);
    }

    inline int getSequentialContainerStoredType(int metaTypeID)
    {
        return (*(QVariant(static_cast<QVariant::Type>(metaTypeID)).value<QSequentialIterable>()).end()).userType();
    }

    inline int getAssociativeContainerStoredKeyType(int metaTypeID)
    {
        if (!ORM_Impl::assContTypeMap.contains(metaTypeID)) return 0;
        return ORM_Impl::assContTypeMap[metaTypeID].first;
    }
    inline int getAssociativeContainerStoredValueType(int metaTypeID)
    {
        if (!ORM_Impl::assContTypeMap.contains(metaTypeID)) return 0;
        return ORM_Impl::assContTypeMap[metaTypeID].second;
    }


    inline bool isQObject(int metaTypeID)
    {
        const QMetaObject * meta = QMetaType::metaObjectForType(metaTypeID);
        if (meta) {
            return meta->inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
        }
        return false;
    }
    inline bool isQObject(QMetaObject const& meta)
    {
        return meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    }

    inline bool isIgnored(int metaTypeID)
    {
        return ignoredTypes.contains(metaTypeID);
    }

    inline bool isPrimitive(int metaTypeID)
    {
        return  isPrimitiveType(metaTypeID) ||
                isPrimitiveString(metaTypeID) ||
                isPrimitiveRaw(metaTypeID);
    }
    inline bool isTypeForTable(int metaTypeID)
    {
        if (isPrimitive(metaTypeID)) {
            return false;
        }
        return  isPointer(metaTypeID) ||
                isGadget(metaTypeID) ||
                isSequentialContainer(metaTypeID) ||
                isAssociativeContainer(metaTypeID) ||
                isPair(metaTypeID);
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
        return QString(ORM_Impl::orm_rowidName) == QString(prop.name());
    }
    inline bool isRowID(const QString & prop)
    {
        return QString(ORM_Impl::orm_rowidName) == prop;
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
            if (ORM_Impl::shouldSkipWriteMeta(property, o)) {
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
            if (ORM_Impl::shouldSkipReadMeta(property, o)) {
                return QVariant();
            }
            return property.read(o);
        }
        else {
            return property.readOnGadget(readFrom.value<void*>());
        }
    }



    void registerPrimitiveTypeContainers()
    {
        orm_containers::registerPrimitiveTypeContainer<QList>();
        orm_containers::registerPrimitiveTypeContainer<QVector>();
    }

}



void orm_qobjects::addQObjectStub(int metaTypeID, orm_qobjects::creators stub)
{
    if (!metaTypeID) qWarning() << "registerQObject: Could not register type 0";
    if (metaTypeID)  ORM_Impl::objectMap[metaTypeID] = stub;
}
bool orm_qobjects::hasQObjectStub(int metaTypeID)
{
    return ORM_Impl::objectMap.contains(metaTypeID);
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

void ORM_Config::addIgnoredType(int metaTypeID)
{
    ORM_Impl::ignoredTypes << metaTypeID;
}

bool ORM_Config::isIgnoredType(int metaTypeID)
{
    return ORM_Impl::ignoredTypes.contains(metaTypeID);
}

void ORM_Config::removeIgnoredType(int metaTypeID)
{
    ORM_Impl::primitiveContainers.remove(metaTypeID);
}

void ORM_Config::addPrimitiveType(int metaTypeID)
{
    ORM_Impl::primitiveContainers << metaTypeID;
}

bool ORM_Config::isPrimitiveType(int metaTypeID)
{
    return ORM_Impl::isPrimitiveType(metaTypeID);
}

void ORM_Config::removePrimitiveType(int metaTypeID)
{
    ORM_Impl::primitiveContainers.remove(metaTypeID);
}

void ORM_Config::addPrimitiveStringType(int metaTypeID)
{
    Q_ASSERT_X( QMetaType::hasRegisteredConverterFunction(metaTypeID, qMetaTypeId<QString>()) &&
                QMetaType::hasRegisteredConverterFunction(qMetaTypeId<QString>(), metaTypeID),
                "addPrimitiveStringType", "Needs registred converters T->QString->T");
    ORM_Impl::primitiveStringContainers << metaTypeID;
}

bool ORM_Config::isPrimitiveStringType(int metaTypeID)
{
    return ORM_Impl::isPrimitiveString(metaTypeID);
}

void ORM_Config::removePrimitiveStringType(int metaTypeID)
{
    ORM_Impl::primitiveStringContainers.remove(metaTypeID);
}

void ORM_Config::addPrimitiveRawType(int metaTypeID)
{
    ORM_Impl::primitiveRawContainers << metaTypeID;
}

bool ORM_Config::isPrimitiveRawType(int metaTypeID)
{
    return ORM_Impl::isPrimitiveRaw(metaTypeID);
}

void ORM_Config::removePrimitiveRawType(int metaTypeID)
{
    ORM_Impl::primitiveRawContainers.remove(metaTypeID);
}



uint qHash(const orm_containers::ORM_QVariantPair &v) noexcept
{
    return qHash(v.key.data());
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




QString ORM::normalize(const QString & str) const
{
    QString s = str;
    static QRegularExpression regExp1 {"(.)([A-Z]+)"};
    static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};
    static QRegularExpression regExp3 {"[:;,.<>]+"};
    return "_" + s.replace(regExp1, "\\1_\\2").replace(regExp2, "\\1_\\2").toLower().replace(regExp3, "_");
}

QString ORM::normalizeVar(const QString &str, int metaType) const
{
    Q_UNUSED(metaType)
    return str;
}

QString ORM::sqlType(int metaType) const {
    if (ORM_Impl::isPrimitiveType(metaType)) {
        return "TEXT";
    }
    if (ORM_Impl::isPrimitiveString(metaType)) {
        return "TEXT";
    }
    if (ORM_Impl::isPrimitiveRaw(metaType)) {
        return "BLOB";
    }
    switch(metaType) {
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
    case QMetaType::QChar:
        return "NUMERIC";
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
    QSqlQuery last_inserted_query(QSqlDatabase::database(m_databaseName, true));
    last_inserted_query.exec("select last_insert_rowid() as rowid;");
    if (ORM_Impl::checkQueryErrors(last_inserted_query, "", "", "")) {
        return 0;
    }
    if (last_inserted_query.next()) {
        return last_inserted_query.value("rowid").toLongLong();
    }
    return 0;
}
long long ORM::get_last_updated_rowid (QSqlQuery &query) const
{
    Q_UNUSED(query)
    QSqlQuery last_updated_query(QSqlDatabase::database(m_databaseName, true));
    last_updated_query.exec("select changes() as ch;");
    if (ORM_Impl::checkQueryErrors(last_updated_query, "", "", "")) {
        return 0;
    }
    if (last_updated_query.next()) {
        return last_updated_query.value("ch").toLongLong();
    }
    return 0;
}

QString ORM::generate_create_query(QString const& parent_name, QString const& property_name, const QString &class_name, const QStringList &names, const QList<int> &types) const
{
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    bool with_orm_rowid = false;
    for (int i = 0; i < names.count(); ++i) {
        if (ORM_Impl::isRowID(names[i])) {
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
        if (ORM_Impl::isRowID(names[i])) {
            query_columns << normalize(names[i]) + " INTEGER PRIMARY KEY ";
            continue;
        }
        query_columns << normalize(names[i]) + " " + sqlType(types[i]);
    }
    if (parent_name != "t") {
        query_text += " " + ORM_Impl::orm_parentRowidName + " INTEGER REFERENCES "
                + parent_name + " (" + normalize(ORM_Impl::orm_rowidName) + ") ";
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
        if (ORM_Impl::isRowID(names[i])) {
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
        query_text += " " + ORM_Impl::orm_parentRowidName + " ";
        if (query_columns.size()) {
            query_text += " , ";
        }
    }
    if (query_columns.size()) {
        query_text += query_columns.join(", ");
    }
    query_text += " FROM " + table_name;
    if (parent_orm_rowid) {
        query_text += " WHERE " + ORM_Impl::orm_parentRowidName + " = :" + ORM_Impl::orm_parentRowidName + " ";
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
        t_names << ORM_Impl::orm_parentRowidName;
        t_values << ":" + ORM_Impl::orm_parentRowidName;
    }
    for (int i = 0; i < names.size(); ++i) {
        t_names << normalize(names[i]);
        t_values << normalizeVar(":" + names[i], types[i]); // let it be like this
    }
    // INFO: on SQLite UPSERT (since 3.24.0 (2018-06-04))
  //QStringList upsert;
  //for (int i = 0; i < t_names.size(); ++i) {
  //      upsert << t_names[i] + " = " + t_values[i];
  //}
    return QString("INSERT INTO %1 (%2) VALUES (%3);") //ON CONFLICT DO UPDATE SET %4 WHERE _orm_rowid = :_orm_rowid
            .arg(table_name).arg(t_names.join(", ")).arg(t_values.join(", "));//.arg(upsert.join(", "));
}
QString ORM::generate_update_query(QString const& parent_name, QString const& property_name, const QString &class_name, const QStringList &names, const QList<int> &types, bool parent_orm_rowid) const
{
    Q_UNUSED(types)
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    QString query_text = QString("UPDATE OR IGNORE %1 SET ").arg(table_name);
    QStringList t_set;
    for (int i = 0; i < names.size(); ++i) {
        t_set << normalize(names[i]) + " = " + normalizeVar(":" + names[i], types[i]);
    }
    query_text += t_set.join(',') + " WHERE " + normalize(ORM_Impl::orm_rowidName) + " = :" + ORM_Impl::orm_rowidName + " ";
    if (parent_orm_rowid) {
        query_text += " AND " + ORM_Impl::orm_parentRowidName + " = :" + ORM_Impl::orm_parentRowidName + " ";
    }
    query_text += ";";
    return query_text;
}
QString ORM::generate_delete_query(QString const& parent_name, QString const& property_name, const QString &class_name, bool parent_orm_rowid) const
{
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    if (!parent_orm_rowid) {
        return QString("DELETE FROM %1 WHERE " + normalize(ORM_Impl::orm_rowidName) + " = :" + ORM_Impl::orm_rowidName + " ").arg(table_name);
    }
    else {
        return QString("DELETE FROM %1 WHERE " + ORM_Impl::orm_parentRowidName + " = :" + ORM_Impl::orm_parentRowidName + ";").arg(table_name);
    }
}
QString ORM::generate_drop_query  (QString const& parent_name, QString const& property_name, const QString &class_name) const
{
    QString table_name = generate_table_name(parent_name, property_name, class_name);
    return QString("DROP TABLE IF EXISTS %1;").arg(table_name);
}


void     ORM::meta_create(const QMetaObject &meta, QString const& parent_name, QString const& property_name)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    bool with_orm_rowid = false;
    QStringList query_columns;
    QList<int> query_columns_types;
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (ORM_Impl::isIgnored(p.userType())) {
            continue;
        }
        if (!p.isStored() || !p.isWritable() || !p.isReadable()) {
            continue;
        }
        if (ORM_Impl::isRowID(p.name())) {
            if (with_orm_rowid) {
                qWarning() << "multiplie orm_rowid properties in " << meta.className();
            }
            with_orm_rowid = true;
        }
        if (ORM_Impl::isPrimitiveType(p.userType())) {
            query_columns << p.name();
            query_columns_types << p.userType();
            continue;
        }
        if (ORM_Impl::isPrimitiveString(p.userType())) {
            query_columns << p.name();
            query_columns_types << qMetaTypeId<QString>();
            continue;
        }
        if (ORM_Impl::isPrimitiveRaw(p.userType())) {
            query_columns << p.name();
            query_columns_types << qMetaTypeId<QByteArray>();
            continue;
        }
        if (ORM_Impl::isTypeForTable(p.userType())) {
            continue;
        }
        query_columns << p.name();
        query_columns_types << p.userType();
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_create_query(parent_name, property_name, meta.className(), query_columns, query_columns_types));
    ORM_Impl::checkQueryErrors(query, meta.className(), table_name);

    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (ORM_Impl::isIgnored(p.userType())) {
            continue;
        }
        if (ORM_Impl::isPrimitive(p.userType())) {
            continue;
        }
        if (ORM_Impl::isGadget(p.userType())) {
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
        if (ORM_Impl::isPointer(p.userType())) {
            if (!ORM_Impl::isWeakPointer(p.userType())) {
                if (!with_orm_rowid) {
                    qWarning() << "create: Using pointer fields without using orm_rowid is not supported. "
                                  "Type " << table_name << " property " << p.name();
                    continue;
                }
                const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(p.userType()));
                if (o) {
                    meta_create(*o, table_name, p.name());
                    continue;
                }
            }
            continue;
        }
        if (ORM_Impl::isSequentialContainer(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using containers without using orm_rowid is not supported. "
                              "Type " << table_name << " property " << p.name();
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(p.userType()));
            if (o) {
                meta_create(*o, table_name, p.name());
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(p.userType()) || ORM_Impl::isPair(p.userType())) {
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
void     ORM::meta_create_pair  (int metaTypeID, QString const& parent_name, QString const& property_name)
{
    QString className = QMetaType::typeName(metaTypeID);
    QString table_name = generate_table_name(parent_name, property_name, className);
    QStringList query_columns;
    QList<int> query_types;
    query_columns << ORM_Impl::orm_rowidName;
    query_types << qMetaTypeId<long long>();
    for (int j = 0; j < 2; ++j) {
        int userType = j == 0 ? ORM_Impl::getAssociativeContainerStoredKeyType(metaTypeID)
                              : ORM_Impl::getAssociativeContainerStoredValueType(metaTypeID);
        QString name = j == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(userType)) {
            continue;
        }
        if (ORM_Impl::isPrimitiveType(userType)) {
            query_columns << name;
            query_types << userType;
            continue;
        }
        if (ORM_Impl::isPrimitiveString(userType)) {
            query_columns << name;
            query_types << qMetaTypeId<QString>();
            continue;
        }
        if (ORM_Impl::isPrimitiveRaw(userType)) {
            query_columns << name;
            query_types << qMetaTypeId<QByteArray>();
            continue;
        }
        if (ORM_Impl::isTypeForTable(userType)) {
            continue;
        }
        query_columns << name;
        query_types << userType;
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_create_query(parent_name, property_name, className, query_columns, query_types));
    ORM_Impl::checkQueryErrors(query, "", table_name);
    for (int j = 0; j < 2; ++j) {
        int userType = j == 0 ? ORM_Impl::getAssociativeContainerStoredKeyType(metaTypeID)
                              : ORM_Impl::getAssociativeContainerStoredValueType(metaTypeID);
        QString name = j == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(userType)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(userType)) {
            continue;
        }
        if (ORM_Impl::isGadget(userType)) {
            const QMetaObject * o = QMetaType::metaObjectForType(userType);
            if (o) {
                meta_create(*o, table_name, name);
                continue;
            }
        }
        if (ORM_Impl::isPointer(userType)) {
            if (!ORM_Impl::isWeakPointer(userType)) {
                const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(userType));
                if (o) {
                    meta_create(*o, table_name, name);
                    continue;
                }
            }
            continue;
        }
        if (ORM_Impl::isSequentialContainer(userType)) {
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(userType));
            if (o) {
                meta_create(*o, table_name, name);
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(userType) || ORM_Impl::isPair(userType)) {
            meta_create_pair(userType, table_name, name);
            continue;
        }
    }
}

QVariant ORM::meta_select(const QMetaObject &meta, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = ORM_Impl::isQObject(meta);
    bool with_orm_rowid = ORM_Impl::withRowid(meta);
    if (!selectQueries.contains(table_name)) {
        QStringList query_columns;
        QList<int> query_types;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (ORM_Impl::isIgnored(p.userType())) {
                continue;
            }
            if (!p.isStored() || ORM_Impl::isTypeForTable(p.userType())) {
                continue;
            }
            if (ORM_Impl::isPrimitiveString(p.userType())) {
                query_columns << p.name();
                query_types << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(p.userType())) {
                query_columns << p.name();
                query_types << qMetaTypeId<QByteArray>();
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
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, meta.className(), table_name, property_name);
    QVariantList list;
    while (query.next()) {
        QVariant v;
        if (isQObject) {
            if (!ORM_Impl::objectMap.contains(classtype)) {
                qWarning() << "select: " << meta.className() << " is not registered. "
                              "Call registerQObjectORM<" << meta.className() << ">(); first";
                break;
            }
            v = QVariant::fromValue(ORM_Impl::objectMap[classtype]());
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
            orm_rowid = query.value(normalize(ORM_Impl::orm_rowidName)).toLongLong();
        }
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (ORM_Impl::isIgnored(p.userType())) {
                continue;
            }
            if (ORM_Impl::shouldSkipWriteMeta(p)) {
                continue;
            }
            if (with_orm_rowid) {
                if (!ORM_Impl::isPrimitive(p.userType())) {
                    if (ORM_Impl::isGadget(p.userType())) {
                        const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
                        if (o) {
                            QVariantList vl = meta_select(*o, table_name, QString(p.name()), orm_rowid).toList();
                            if (vl.size()) {
                                ORM_Impl::write(isQObject, v, p, vl.first());
                            }
                            continue;
                        }
                    }
                    if (ORM_Impl::isPointer(p.userType())) {
                        if (ORM_Impl::isWeakPointer(p.userType())) {
                            continue;
                        }
                        const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(p.userType()));
                        if (o) {
                            QVariantList vl = meta_select(*o, table_name, QString(p.name()), orm_rowid).toList();
                            if (vl.size()) {
                                if (vl.first().value<void*>()) {
                                    ORM_Impl::write(isQObject, v, p, vl.first());
                                }
                            }
                            continue;
                        }
                    }
                    if (ORM_Impl::isSequentialContainer(p.userType())) {
                        const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(p.userType()));
                        if (o) {
                            ORM_Impl::write(isQObject, v, p, meta_select(*o, table_name, QString(p.name()), orm_rowid).toList());
                            continue;
                        }
                    }
                    if (ORM_Impl::isAssociativeContainer(p.userType()) || ORM_Impl::isPair(p.userType())) {
                        QVariant qv = meta_select_pair(p.userType(), table_name, QString(p.name()), orm_rowid);
                        if (qv.canConvert(p.userType())) {
                            qv.convert(p.userType());
                            ORM_Impl::write(isQObject, v, p, qv);
                        }
                        continue;
                    }
                }
            }
            QVariant vv = query.value(normalize(p.name()));
            if (p.isEnumType()) {
                vv.convert(p.userType());
            }
            if (ORM_Impl::isPrimitiveString(p.userType())) {
                vv.convert(p.userType());
            }
            if (ORM_Impl::isPrimitiveRaw(p.userType())) {
                QVariant w;
                QByteArray array = vv.toByteArray();
                QDataStream stream(&array,QIODevice::ReadWrite);
                stream >> w;
                vv = w;
            }
            ORM_Impl::write(isQObject, v, p, vv);
        }
        list << v;
    }
    return QVariant(list);
}
QVariant ORM::meta_select_pair  (int metaTypeID, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(metaTypeID);
    QString table_name = generate_table_name(parent_name, property_name, className);
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(metaTypeID);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(metaTypeID);
    if (!selectQueries.contains(table_name)) {
        QStringList query_columns;
        QList<int> query_types;
        query_columns << ORM_Impl::orm_rowidName;
        query_types << qMetaTypeId<long long>();
        for (int j = 0; j < 2; ++j) {
            int userType = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (ORM_Impl::isIgnored(userType)) {
                continue;
            }
            if (ORM_Impl::isTypeForTable(userType)) {
                continue;
            }
            query_columns << name;
            if (ORM_Impl::isPrimitiveString(userType)) {
                query_types << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(userType)) {
                query_types << qMetaTypeId<QByteArray>();
                continue;
            }
            query_types << userType;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_select_query(parent_name, property_name, className,
                                            query_columns, query_types, parent_orm_rowid != 0));
        selectQueries[table_name] = query;
    }
    QSqlQuery & query = selectQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, className, table_name, property_name);
    QVariantList list;
    while (query.next()) {
        orm_containers::ORM_QVariantPair pair;
        pair.m_orm_rowid = query.value("_orm_rowid").toLongLong();
        for (int j = 0; j < 2; ++j) {
            int userType = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (ORM_Impl::isIgnored(userType)) {
                continue;
            }
            if (!ORM_Impl::isPrimitive(userType)) {
                if (ORM_Impl::isGadget(userType)) {
                    const QMetaObject * o = QMetaType::metaObjectForType(userType);
                    if (o) {
                        QVariantList vl = meta_select(*o, table_name, name, pair.m_orm_rowid).toList();
                        if (vl.size()) {
                            pair[j] = vl.first();
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isPointer(userType)) {
                    if (ORM_Impl::isWeakPointer(userType)) {
                        continue;
                    }
                    const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(userType));
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
                if (ORM_Impl::isSequentialContainer(userType)) {
                    const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(userType));
                    if (o) {
                        pair[j] = meta_select(*o, table_name, name, pair.m_orm_rowid).toList();
                        continue;
                    }
                }
                if (ORM_Impl::isAssociativeContainer(userType) || ORM_Impl::isPair(userType)) {
                    QVariant qv = meta_select_pair(userType, table_name, name, pair.m_orm_rowid);
                    if (qv.canConvert(userType)) {
                        qv.convert(userType);
                        pair[j] = qv;
                    }
                    continue;
                }
            }
            QVariant vv = query.value(normalize(name));
            if (QMetaType::typeFlags(userType).testFlag(QMetaType::IsEnumeration)) {
                vv.convert(userType);
            }
            if (ORM_Impl::isPrimitiveString(userType)) {
                vv.convert(userType);
            }
            if (ORM_Impl::isPrimitiveRaw(userType)) {
                QVariant w;
                QByteArray array = vv.toByteArray();
                QDataStream stream(&array,QIODevice::ReadWrite);
                stream >> w;
                vv = w;
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
    bool with_orm_rowid = ORM_Impl::withRowid(meta);
    if (!insertQueries.contains(table_name)) {
        QStringList names;
        QList<int> types;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (ORM_Impl::isIgnored(p.userType())) {
                continue;
            }
            if (ORM_Impl::isTypeForTable(p.userType()) || !p.isStored() || !p.isReadable()) {
                continue;
            }
            if (ORM_Impl::isRowID(p)) {
                orm_rowid = ORM_Impl::read(isQObject, v, p).toLongLong();
                if (orm_rowid == 0 || orm_rowid == -1) {
                    continue;
                }
            }
            names << p.name();
            if (ORM_Impl::isPrimitiveString(p.userType())) {
                types << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(p.userType())) {
                types << qMetaTypeId<QByteArray>();
                continue;
            }
            types << p.userType();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_insert_query(parent_name, property_name, meta.className(),
                                            names, types, parent_orm_rowid != 0));
        insertQueries[table_name] = query;
    }
    QSqlQuery & query = insertQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (ORM_Impl::isIgnored(p.userType())) {
            continue;
        }
        if (!p.isStored() || !p.isReadable() || ORM_Impl::isTypeForTable(p.userType())) {
            continue;
        }
        if (ORM_Impl::isRowID(p) && (orm_rowid == 0 || orm_rowid == -1)) {
            continue;
        }
        QVariant w = ORM_Impl::read(isQObject, v, p);
        if (ORM_Impl::isPrimitiveString(p.userType())) {
            w.convert(qMetaTypeId<QString>());
        }
        if (ORM_Impl::isPrimitiveRaw(p.userType())) {
            QByteArray array;
            QDataStream stream(&array,QIODevice::ReadWrite);
            stream << w;
            w = array;
            //w.convert(qMetaTypeId<QByteArray>());
        }
        query.bindValue(QString(":") + p.name(), w);
    }
    query.exec();
    if (ORM_Impl::checkQueryErrors(query, meta.className(), table_name, property_name)) {
        return;
    }

    if (with_orm_rowid) {
        orm_rowid = get_last_inserted_rowid(query);
        if (orm_rowid) {
            for (int i = 0; i < meta.propertyCount(); ++i) {
                QMetaProperty p = meta.property(i);
                if (ORM_Impl::isIgnored(p.userType())) {
                    continue;
                }
                if (!p.isStored() || !p.isReadable() || ORM_Impl::isPrimitive(p.userType())) {
                    continue;
                }
                if (ORM_Impl::isGadget(p.userType())) {
                    const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
                    if (o) {
                        QVariant vv = ORM_Impl::read(isQObject, v, p);
                        meta_insert(*o, vv, table_name, QString(p.name()), orm_rowid);
                        continue;
                    }
                }
                if (ORM_Impl::isPointer(p.userType())) {
                    if (ORM_Impl::isWeakPointer(p.userType())) {
                        continue;
                    }
                    const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(p.userType()));
                    if (o) {
                        QVariant vvv = p.readOnGadget(v.value<void*>());
                        if (vvv.value<void*>()) {
                            vvv.convert(qMetaTypeId<void*>());
                            meta_insert(*o, vvv, table_name, QString(p.name()), orm_rowid);
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isSequentialContainer(p.userType())) {
                    const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(p.userType()));
                    if (o) {
                        QVariant qv = ORM_Impl::read(isQObject, v, p);
                        if (qv.isValid()) {
                            QSequentialIterable si = qv.value<QSequentialIterable>();
                            for (QVariant vv : si) {
                                meta_insert(*o, vv, table_name, QString(p.name()), orm_rowid);
                            }
                        }
                        continue;
                    }
                }
                if (ORM_Impl::isAssociativeContainer(p.userType()) || ORM_Impl::isPair(p.userType())) {
                    QVariant qv = ORM_Impl::read(isQObject, v, p);
                    if (qv.canConvert<orm_containers::ORM_QVariantPair>()) {
                        qv.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                        meta_insert_pair(p.userType(), qv, table_name, QString(p.name()), orm_rowid);
                        continue;
                    }
                    if (qv.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                        QList<orm_containers::ORM_QVariantPair> list = qv.value<QList<orm_containers::ORM_QVariantPair>>();
                        for (orm_containers::ORM_QVariantPair & v : list) {
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
void     ORM::meta_insert_pair  (int metaTypeID, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(metaTypeID);
    QString table_name = generate_table_name(parent_name, property_name, className);
    orm_containers::ORM_QVariantPair ops = v.value<orm_containers::ORM_QVariantPair>();
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(metaTypeID);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(metaTypeID);
    if (!insertQueries.contains(table_name)) {
        QStringList names;
        QList<int> types;
        for (int j = 0; j < 2; ++j) {
            int userType = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (ORM_Impl::isIgnored(userType)) {
                continue;
            }
            if (ORM_Impl::isTypeForTable(userType)) {
                continue;
            }
            names << name;
            if (ORM_Impl::isPrimitiveString(userType)) {
                types << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(userType)) {
                types << qMetaTypeId<QByteArray>();
                continue;
            }
            types << userType;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_insert_query(parent_name, property_name, className,
                                            names, types, parent_orm_rowid != 0));
        insertQueries[table_name] = query;
    }
    QSqlQuery & query = insertQueries[table_name];
    query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    for (int j = 0; j < 2; ++j) {
        int userType = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(userType)) {
            continue;
        }
        if (ORM_Impl::isTypeForTable(userType)) {
            continue;
        }
        QVariant w = ops[j];
        if (ORM_Impl::isPrimitiveString(userType)) {
            w.convert(qMetaTypeId<QString>());
        }
        if (ORM_Impl::isPrimitiveRaw(userType)) {
            QByteArray array;
            QDataStream stream(&array,QIODevice::ReadWrite);
            stream << w;
            w = array;
        }
        query.bindValue(QString(":") + name, w);
    }
    query.exec();
    if (ORM_Impl::checkQueryErrors(query, className, table_name, property_name)) {
        return;
    }

    ops.m_orm_rowid = get_last_inserted_rowid(query);
    if (!ops.m_orm_rowid) {
        return;
    }
    for (int j = 0; j < 2; ++j) {
        int userType = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(userType)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(userType)) {
            continue;
        }
        if (ORM_Impl::isGadget(userType)) {
            const QMetaObject * o = QMetaType::metaObjectForType(userType);
            if (o) {
                meta_insert(*o, ops[j], table_name, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (ORM_Impl::isPointer(userType)) {
            if (ORM_Impl::isWeakPointer(userType)) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(userType));
            if (o) {
                QVariant vvv = ops[j];
                if (vvv.value<void*>()) {
                    vvv.convert(qMetaTypeId<void*>());
                    meta_insert(*o, vvv, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
        if (ORM_Impl::isSequentialContainer(userType)) {
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(userType));
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
        if (ORM_Impl::isAssociativeContainer(userType) || ORM_Impl::isPair(userType)) {
            QVariant qv = ops[j];
            if (qv.canConvert<orm_containers::ORM_QVariantPair>()) {
                qv.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                meta_insert_pair(userType, qv, table_name, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                QList<orm_containers::ORM_QVariantPair> list = qv.value<QList<orm_containers::ORM_QVariantPair>>();
                for (orm_containers::ORM_QVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_insert_pair(userType, vv, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
}

void     ORM::meta_update(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    bool isQObject = ORM_Impl::isQObject(meta);
    if (!ORM_Impl::withRowid(meta)) {
        qWarning() << "update: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    if (!updateQueries.contains(table_name)) {
        QStringList sets;
        QStringList names;
        QList<int> types;
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (ORM_Impl::isIgnored(p.userType())) {
                continue;
            }
            if (!p.isStored() || !p.isReadable() || ORM_Impl::isTypeForTable(p.userType())) {
                continue;
            }
            if (ORM_Impl::isRowID(p.name())) {
                continue;
            }
            names << p.name();
            if (ORM_Impl::isPrimitiveString(p.userType())) {
                types << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(p.userType())) {
                types << qMetaTypeId<QByteArray>();
                continue;
            }
            types << p.userType();
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_update_query(parent_name, property_name, meta.className(),
                                            names, types, parent_orm_rowid != 0));
        updateQueries[table_name] = query;
    }
    QSqlQuery & query = updateQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (ORM_Impl::isIgnored(p.userType())) {
            continue;
        }
        if (!p.isStored() || !p.isReadable() || ORM_Impl::isTypeForTable(p.userType())) {
            continue;
        }
        QVariant w = ORM_Impl::read(isQObject, v, p);
        if (ORM_Impl::isPrimitiveString(p.userType())) {
            w.convert(qMetaTypeId<QString>());
        }
        if (ORM_Impl::isPrimitiveRaw(p.userType())) {
            QByteArray array;
            QDataStream stream(&array,QIODevice::ReadWrite);
            stream << w;
            w = array;
        }
        query.bindValue(QString(":") + p.name(), w);
    }
    query.exec();
    if (ORM_Impl::checkQueryErrors(query, meta.className(), table_name, property_name)) {
        return;
    }

    long long orm_rowid = get_last_updated_rowid(query);
    if (orm_rowid == 0 && false) {
        meta_insert(meta, v, parent_name, property_name, parent_orm_rowid);
        return;
    }

    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (ORM_Impl::isIgnored(p.userType())) {
            continue;
        }
        if (!p.isStored() || !p.isReadable() || ORM_Impl::isPrimitive(p.userType())) {
            continue;
        }
        if (ORM_Impl::isGadget(p.userType())) {
            const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
            if (o) {
                QVariant vv = ORM_Impl::read(isQObject, v, p);
                meta_update(*o, vv, table_name, QString(p.name()), orm_rowid);
                continue;
            }
        }
        if (ORM_Impl::isPointer(p.userType())) {
            if (ORM_Impl::isWeakPointer(p.userType())) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(p.userType()));
            if (o) {
                QVariant vvv = ORM_Impl::read(isQObject, v, p);
                if (vvv.value<void*>()) {
                    vvv.convert(qMetaTypeId<void*>());
                    meta_update(*o, vvv, table_name, QString(p.name()), orm_rowid);
                }
                continue;
            }
        }
        if (ORM_Impl::isSequentialContainer(p.userType())) {
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(p.userType()));
            if (o) {
                QVariant qv = ORM_Impl::read(isQObject, v, p);
                if (qv.isValid()) {
                    QSequentialIterable si = qv.value<QSequentialIterable>();
                    for (QVariant v : si) {
                        meta_update(*o, v, table_name, QString(p.name()), orm_rowid);
                    }
                }
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(p.userType()) || ORM_Impl::isPair(p.userType())) {
            QVariant qv = ORM_Impl::read(isQObject, v, p);
            if (qv.canConvert<orm_containers::ORM_QVariantPair>()) {
                qv.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                meta_update_pair(p.userType(), qv, table_name, QString(p.name()), orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                QList<orm_containers::ORM_QVariantPair> list = qv.value<QList<orm_containers::ORM_QVariantPair>>();
                for (orm_containers::ORM_QVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_update_pair(p.userType(), vv, table_name, QString(p.name()), orm_rowid);
                }
                continue;
            }
        }
    }
}
void     ORM::meta_update_pair  (int metaTypeID, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(metaTypeID);
    QString table_name = generate_table_name(parent_name, property_name, className);
    orm_containers::ORM_QVariantPair ops = v.value<orm_containers::ORM_QVariantPair>();
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(metaTypeID);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(metaTypeID);
    if (!updateQueries.contains(table_name)) {
        QStringList names;
        QList<int> types;
        for (int j = 0; j < 2; ++j) {
            int userType = j == 0 ? keyType : valueType;
            QString name = j == 0 ? "key" : "value";
            if (ORM_Impl::isIgnored(userType)) {
                continue;
            }
            if (ORM_Impl::isTypeForTable(userType)) {
                continue;
            }
            names << name;
            if (ORM_Impl::isPrimitiveString(userType)) {
                types << qMetaTypeId<QString>();
                continue;
            }
            if (ORM_Impl::isPrimitiveRaw(userType)) {
                types << qMetaTypeId<QByteArray>();
                continue;
            }
            types << userType;
        }
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        query.prepare(generate_update_query(parent_name, property_name, className,
                                            names, types, parent_orm_rowid != 0));
        updateQueries[table_name] = query;
    }
    QSqlQuery & query = updateQueries[table_name];
    if (parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    for (int j = 0; j < 2; ++j) {
        int userType = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(userType)) {
            continue;
        }
        if (ORM_Impl::isTypeForTable(userType)) {
            continue;
        }
        QVariant w = ops[j];
        if (ORM_Impl::isPrimitiveString(userType)) {
            w.convert(qMetaTypeId<QString>());
        }
        if (ORM_Impl::isPrimitiveRaw(userType)) {
            QByteArray array;
            QDataStream stream(&array,QIODevice::ReadWrite);
            stream << w;
            w = array;
        }
        query.bindValue(QString(":") + name, w);
    }
    query.exec();
    if (ORM_Impl::checkQueryErrors(query, className, table_name, property_name)) {
        return;
    }

    long long orm_rowid = get_last_updated_rowid(query);
    if (orm_rowid == 0 && false) {
        meta_insert_pair(metaTypeID, v, parent_name, property_name, parent_orm_rowid);
        return;
    }

    for (int j = 0; j < 2; ++j) {
        int userType = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(userType)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(userType)) {
            continue;
        }
        if (ORM_Impl::isGadget(userType)) {
            const QMetaObject * o = QMetaType::metaObjectForType(userType);
            if (o) {
                QVariant vv = ops[j];
                meta_update(*o, vv, table_name, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (ORM_Impl::isPointer(userType)) {
            if (ORM_Impl::isWeakPointer(userType)) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(userType));
            if (o) {
                QVariant vvv = ops[j];
                if (vvv.value<void*>()) {
                    vvv.convert(qMetaTypeId<void*>());
                    meta_update(*o, vvv, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
        if (ORM_Impl::isSequentialContainer(userType)) {
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(userType));
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
        if (ORM_Impl::isAssociativeContainer(userType) || ORM_Impl::isPair(userType)) {
            QVariant qv = ops[j];
            if (qv.canConvert<orm_containers::ORM_QVariantPair>()) {
                qv.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                meta_update_pair(userType, qv, table_name, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                QList<orm_containers::ORM_QVariantPair> list = qv.value<QList<orm_containers::ORM_QVariantPair>>();
                for (orm_containers::ORM_QVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_update_pair(userType, vv, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
}

void     ORM::meta_delete(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    bool isQObject = ORM_Impl::isQObject(meta);
    long long orm_rowid = 0;
    if (!ORM_Impl::withRowid(meta)) {
        qWarning() << "delete: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (ORM_Impl::isIgnored(p.userType())) {
            continue;
        }
        if (ORM_Impl::isTypeForTable(p.userType())) {
            continue;
        }
        if (ORM_Impl::isRowID(p)) {
            orm_rowid = ORM_Impl::read(isQObject, v, p).toLongLong();
        }
    }
    if (!orm_rowid && !parent_orm_rowid) {
        return;
    }
    if (orm_rowid) {
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (ORM_Impl::isIgnored(p.userType())) {
                continue;
            }
            if (!p.isStored() || !p.isReadable() || ORM_Impl::isPrimitive(i)) {
                continue;
            }
            if (ORM_Impl::isGadget(p.userType())) {
                const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
                if (o) {
                    QVariant vv = ORM_Impl::read(isQObject, v, p);
                    meta_delete(*o, vv, table_name, QString(p.name()), orm_rowid);
                    continue;
                }
            }
            if (ORM_Impl::isPointer(p.userType())) {
                if (ORM_Impl::isWeakPointer(p.userType())) {
                    continue;
                }
                const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(p.userType()));
                if (o) {
                    QVariant vv = ORM_Impl::read(isQObject, v, p);
                    meta_delete(*o, vv, table_name, QString(p.name()), orm_rowid);
                    continue;
                }
            }
            if (ORM_Impl::isSequentialContainer(p.userType())) {
                const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(p.userType()));
                if (o) {
                    QSequentialIterable si = ORM_Impl::read(isQObject, v, p).value<QSequentialIterable>();
                    for (QVariant v : si) {
                        meta_delete(*o, v, table_name, QString(p.name()), orm_rowid);
                    }
                    continue;
                }
            }
            if (ORM_Impl::isAssociativeContainer(p.userType()) || ORM_Impl::isPair(p.userType())) {
                QVariant qv = ORM_Impl::read(isQObject, v, p);
                if (qv.canConvert<orm_containers::ORM_QVariantPair>()) {
                    qv.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                    meta_delete_pair(p.userType(), qv, table_name, QString(p.name()), orm_rowid);
                    continue;
                }
                if (qv.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                    QList<orm_containers::ORM_QVariantPair> list = qv.value<QList<orm_containers::ORM_QVariantPair>>();
                    for (orm_containers::ORM_QVariantPair & v : list) {
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
        query.bindValue(":" + ORM_Impl::orm_rowidName, orm_rowid);
    }
    else {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, meta.className(), table_name, property_name);
}
void     ORM::meta_delete_pair  (int metaTypeID, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString className = QMetaType::typeName(metaTypeID);
    QString table_name = generate_table_name(parent_name, property_name, className);
    orm_containers::ORM_QVariantPair ops = v.value<orm_containers::ORM_QVariantPair>();
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(metaTypeID);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(metaTypeID);
    for (int j = 0; j < 2; ++j) {
        int userType = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(userType)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(userType)) {
            continue;
        }
        if (ORM_Impl::isGadget(userType)) {
            const QMetaObject * o = QMetaType::metaObjectForType(userType);
            if (o) {
                meta_delete(*o, ops[j], table_name, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (ORM_Impl::isPointer(userType)) {
            if (ORM_Impl::isWeakPointer(userType)) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(userType));
            if (o) {
                meta_delete(*o, ops[j], table_name, name, ops.m_orm_rowid);
                continue;
            }
        }
        if (ORM_Impl::isSequentialContainer(userType)) {
            const QMetaObject * o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(userType));
            if (o) {
                QSequentialIterable si = ops[j].value<QSequentialIterable>();
                for (QVariant v : si) {
                    meta_delete(*o, v, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
        if (ORM_Impl::isAssociativeContainer(userType) || ORM_Impl::isPair(userType)) {
            QVariant qv = ops[j];
            if (qv.canConvert<orm_containers::ORM_QVariantPair>()) {
                qv.convert(qMetaTypeId<orm_containers::ORM_QVariantPair>());
                meta_delete_pair(userType, qv, table_name, name, ops.m_orm_rowid);
                continue;
            }
            if (qv.canConvert<QList<orm_containers::ORM_QVariantPair>>()) {
                QList<orm_containers::ORM_QVariantPair> list = qv.value<QList<orm_containers::ORM_QVariantPair>>();
                for (orm_containers::ORM_QVariantPair & v : list) {
                    QVariant vv = QVariant::fromValue(v);
                    meta_delete_pair(userType, vv, table_name, name, ops.m_orm_rowid);
                }
                continue;
            }
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(generate_delete_query(parent_name, property_name, className, parent_orm_rowid));
    if (!parent_orm_rowid) {
        query.bindValue(":" + ORM_Impl::orm_rowidName, ops.m_orm_rowid);
    }
    else {
        query.bindValue(":" + ORM_Impl::orm_parentRowidName, parent_orm_rowid);
    }
    query.exec();
    ORM_Impl::checkQueryErrors(query, className, table_name, property_name);
}

void     ORM::meta_drop  (const QMetaObject &meta, QString const& parent_name, const QString &property_name)
{
    QString table_name = generate_table_name(parent_name, property_name, QString(meta.className()));
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (ORM_Impl::isIgnored(p.userType())) {
            continue;
        }
        if (!p.isStored() || ORM_Impl::isPrimitive(p.userType())) {
            continue;
        }
        const QMetaObject * o = nullptr;
        if (ORM_Impl::isGadget(p.userType())) {
            o = QMetaType::metaObjectForType(p.userType());
        }
        if (ORM_Impl::isPointer(p.userType())) {
            if (ORM_Impl::isWeakPointer(p.userType())) {
                continue;
            }
           o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(p.userType()));
        }
        if (ORM_Impl::isSequentialContainer(p.userType())) {
            o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(p.userType()));
        }
        if (ORM_Impl::isAssociativeContainer(p.userType()) || ORM_Impl::isPair(p.userType())) {
            meta_drop_pair(p.userType(), table_name, p.name());
            continue;
        }
        if (o) {
            meta_drop(*o, table_name, p.name());
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_drop_query(parent_name, property_name, QString(meta.className())));
    ORM_Impl::checkQueryErrors(query, meta.className(), table_name);
}
void     ORM::meta_drop_pair    (int metaTypeID, QString const& parent_name, QString const& property_name)
{
    QString className = QMetaType::typeName(metaTypeID);
    QString table_name = generate_table_name(parent_name, property_name, className);
    int keyType = ORM_Impl::getAssociativeContainerStoredKeyType(metaTypeID);
    int valueType = ORM_Impl::getAssociativeContainerStoredValueType(metaTypeID);
    for (int j = 0; j < 2; ++j) {
        int userType = j == 0 ? keyType : valueType;
        QString name = j == 0 ? "key" : "value";
        if (ORM_Impl::isIgnored(userType)) {
            continue;
        }
        if (ORM_Impl::isPrimitive(userType)) {
            continue;
        }
        const QMetaObject * o = nullptr;
        if (ORM_Impl::isGadget(userType)) {
            o = QMetaType::metaObjectForType(userType);
        }
        if (ORM_Impl::isPointer(userType)) {
            if (ORM_Impl::isWeakPointer(userType)) {
                continue;
            }
           o = QMetaType::metaObjectForType(ORM_Impl::pointerToValue(userType));
        }
        if (ORM_Impl::isSequentialContainer(userType)) {
            o = QMetaType::metaObjectForType(ORM_Impl::getSequentialContainerStoredType(userType));
        }
        if (ORM_Impl::isAssociativeContainer(userType)) {
            meta_drop_pair(metaTypeID, table_name, name);
            continue;
        }
        if (o) {
            meta_drop(*o, table_name, name);
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(generate_drop_query(parent_name, property_name, className));
    ORM_Impl::checkQueryErrors(query, className, table_name);
}










QDebug &operator<<(QDebug &dbg, const ORM_Config &)
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
