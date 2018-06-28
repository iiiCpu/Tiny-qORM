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

bool isAssociativeContainer(int metatypeid)
{
    return QMetaType::hasRegisteredConverterFunction(metatypeid, qMetaTypeId<QtMetaTypePrivate::QAssociativeIterableImpl>());
}

int getSequentialContainerStoredType(int metatypeid)
{
    return (*(QVariant((QVariant::Type)metatypeid).value<QSequentialIterable>()).end()).userType();
}

int getAssociativeContainerStoredKeyType(int metatypeid)
{
    return ((QVariant((QVariant::Type)metatypeid).value<QAssociativeIterable>()).end().key()).userType();
}

int getAssociativeContainerStoredValueType(int metatypeid)
{
    return ((QVariant((QVariant::Type)metatypeid).value<QAssociativeIterable>()).end().value()).userType();
}










QString ORM::databaseName() const
{
    return m_databaseName;
}

void ORM::setDatabaseName(const QString & databaseName)
{
    m_databaseName = databaseName;
}


bool withRowid(const QMetaObject &meta)
{
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!p.isStored()) {
            continue;
        }
        if (p.name() == QString("orm_rowid")) {
            return true;
        }
    }
    return false;
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
    }
}

ORM::~ORM()
{

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
    return "_" + fromCamelCase(str);
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

void ORM::meta_create(const QMetaObject &meta, QString const& parent_name)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    QString query_text = QString("CREATE TABLE IF NOT EXISTS %1 (").arg(myName);
    QStringList query_columns;
    bool with_orm_rowid = withRowid(meta);
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!p.isStored() || !p.isWritable()) {
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
            if (isWeakPointer(p.userType())) {
                continue;
            }
            if (!with_orm_rowid) {
                qWarning() << "create: Using structure fields without using orm_rowid is not supported. Type " << meta.className() << " property " << p.name();
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
            if (o) {
                meta_create(*o, myName);
                continue;
            }
        }
        if (isSequentialContainer(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using structure fields without using orm_rowid is not supported. Type " << meta.className() << " property " << p.name();
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
            if (o) {
                meta_create(*o, myName);
                continue;
            }
        }
        if (isAssociativeContainer(p.userType())) {
            if (!with_orm_rowid) {
                qWarning() << "create: Using structure fields without using orm_rowid is not supported. Type " << meta.className() << " property " << p.name();
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(getAssociativeContainerStoredValueType(p.userType()));
            if (o) {
                meta_create(*o, myName);
                continue;
            }
        }
        query_columns << normalize(p.name()) + " " + TYPE(p.type());
    }
    if (with_orm_rowid) {
        query_text += "orm_rowid INTEGER PRIMARY KEY, ";
    }
    query_text += " property_name TEXT, parent_orm_rowid INTEGER, " + query_columns.join(", ") + ");";
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(query_text);
    if (query.lastError().isValid()) {
        qDebug() << meta.className() << myName;
        qDebug() << query.lastError().databaseText() << query.lastError().driverText();
        qDebug() << query.lastQuery();
        qDebug() << query.boundValues();
    }
}

QVariant ORM::meta_select(const QMetaObject &meta, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    QStringList query_columns;
    bool with_orm_rowid = withRowid(meta);
    QString query_text = "SELECT property_name, parent_orm_rowid, ";
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!isPrimitive(p.userType()) && (!p.isStored() || isGadget(p.userType()) || isPointer(p.userType()) ||
               isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
            continue;
        }
        if (QString(p.name()) == "orm_rowid") {
            continue;
        }
        query_columns << normalize(p.name());
    }
    if (with_orm_rowid) {
        query_text += " orm_rowid, ";
    }
    query_text += query_columns.join(", ") + " FROM " + myName;
    if (parent_orm_rowid) {
        query_text += " WHERE parent_orm_rowid = :parent_orm_rowid AND property_name = :property_name ";
    }
    query_text += ";";
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(query_text);
    if (with_orm_rowid && parent_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    query.exec();
    if (query.lastError().isValid()) {
        qDebug() << meta.className() << myName << property_name;
        qDebug() << query.lastError().databaseText() << query.lastError().driverText();
        qDebug() << query.lastQuery();
        qDebug() << query.boundValues();
    }
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
            orm_rowid = query.value("orm_rowid").toLongLong();
        }
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (!p.isStored() || !p.isWritable()) {
                continue;
            }
            if (with_orm_rowid) {
                if (!isPrimitive(p.userType())) {
                    if (isGadget(p.userType())) {
                        const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
                        if (o) {
                            QVariantList vl = meta_select(*o, myName, QString(p.name()), orm_rowid).toList();
                            if (vl.size()) {
                                if (isQObject) {
                                    meta.property(i).write(v.value<QObject*>(), vl.first());
                                }
                                else {
                                    meta.property(i).writeOnGadget(v.value<void*>(), vl.first());
                                }
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
                                    if (isQObject) {
                                        meta.property(i).write(v.value<QObject*>(), vl.first());
                                    }
                                    else {
                                        meta.property(i).writeOnGadget(v.value<void*>(), vl.first());
                                    }
                                }
                            }
                            continue;
                        }
                    }
                    if (isSequentialContainer(p.userType())) {
                        const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
                        if (o) {
                            QVariant qv = meta_select(*o, myName, QString(p.name()), orm_rowid);
                            if (isQObject) {
                                meta.property(i).write(v.value<QObject*>(), qv.toList());
                            }
                            else {
                                meta.property(i).writeOnGadget(v.value<void*>(), qv.toList());
                            }
                            continue;
                        }
                    }
                    if (isAssociativeContainer(p.userType())) {
                        const QMetaObject * o = QMetaType::metaObjectForType(getAssociativeContainerStoredValueType(p.userType()));
                        if (o) {
                            QVariant qv = meta_select(*o, myName, QString(p.name()), orm_rowid);
                            if (isQObject) {
                                meta.property(i).write(v.value<QObject*>(), qv);
                            }
                            else {
                                meta.property(i).writeOnGadget(v.value<void*>(), qv);
                            }
                            continue;
                        }
                    }
                }

                if (QString(p.name()) == "orm_rowid") {
                    if (isQObject) {
                        meta.property(i).write(v.value<QObject*>(), query.value(p.name()));
                    }
                    else {
                        meta.property(i).writeOnGadget(v.value<void*>(), query.value(p.name()));
                    }
                    continue;
                }
                if (isQObject) {
                    meta.property(i).write(v.value<QObject*>(), query.value(normalize(p.name())));
                }
                else {
                    meta.property(i).writeOnGadget(v.value<void*>(), query.value(normalize(p.name())));
                }
            }
            else {
                if (isQObject) {
                    meta.property(i).write(v.value<QObject*>(), query.value(normalize(p.name())));
                }
                else {
                    meta.property(i).writeOnGadget(v.data(), query.value(normalize(p.name())));
                }
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
    QStringList names, values;
    bool with_orm_rowid = withRowid(meta);
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!isPrimitive(p.userType()) && (!p.isStored() || !p.isReadable()  || isGadget(p.userType()) || isPointer(p.userType()) ||
               isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
            continue;
        }
        if (p.name() == QString("orm_rowid")) {
            bool b;
            if (isQObject) {
                b = p.read(v.value<QObject*>()).toLongLong() == 0;
            }
            else {
                b = p.readOnGadget(v.value<void*>()).toLongLong() == 0;
            }
            if (b) {
                names << "parent_orm_rowid" << "property_name";
                values << ":parent_orm_rowid" << ":property_name";
            }
            else {
                names << "orm_rowid" << "parent_orm_rowid" << "property_name";
                values << ":orm_rowid" << ":parent_orm_rowid" << ":property_name";
            }
            continue;
        }
        names << normalize(p.name());
        values << ":" + normalize(p.name());
    }
    QString query_text = QString("INSERT OR IGNORE INTO %1 (%2) values(%3);").arg(myName).arg(names.join(", ")).arg(values.join(", "));
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(query_text);
    if (with_orm_rowid) {
        query.bindValue(":parent_orm_rowid", parent_orm_rowid);
        query.bindValue(":property_name", property_name);
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!p.isStored()  || !p.isReadable()  || isGadget(p.userType()) || isPointer(p.userType()) ||
                isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType())) {
            continue;
        }
        if (isQObject) {
            query.bindValue(QString(":") + normalize(p.name()), p.read(v.value<QObject*>()));
        }
        else {
            query.bindValue(QString(":") + normalize(p.name()), p.readOnGadget(v.value<void*>()));
        }
    }
    query.exec();
    if (query.lastError().isValid()) {
        qDebug() << meta.className() << myName << property_name;
        qDebug() << query.lastError().databaseText() << query.lastError().driverText();
        qDebug() << query.lastQuery();
        qDebug() << query.boundValues();
        return;
    }

    QSqlQuery query2(QSqlDatabase::database(m_databaseName, true));
    query2.exec("select last_insert_rowid() as orm_rowid;");
    if (query2.lastError().isValid()) {
        qDebug() << meta.className() << myName << property_name;
        qDebug() << query2.lastError().databaseText() << query2.lastError().driverText();
        qDebug() << query2.lastQuery();
        qDebug() << query2.boundValues();
        return;
    }

    long long orm_rowid = 0;
    if (query2.next()) {
        orm_rowid = query2.value("orm_rowid").toLongLong();
    }
    if (with_orm_rowid && orm_rowid) {
        for (int i = 0; i < meta.propertyCount(); ++i) {
            QMetaProperty p = meta.property(i);
            if (!p.isStored() || !p.isReadable() || isPrimitive(p.userType())) {
                continue;
            }
            if (p.name() == QString("orm_rowid")) {
                if (isQObject) {
                    p.write(v.value<QObject*>(), orm_rowid);
                }
                else {
                    p.writeOnGadget(v.value<void*>(), orm_rowid);
                }
                continue;
            }
            if (isGadget(p.userType())) {
                const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
                if (o) {
                    QVariant vvv;
                    if (isQObject) {
                        vvv= p.read(v.value<QObject*>());
                    }
                    else {
                        vvv= p.readOnGadget(v.value<void*>());
                    }
                    meta_insert(*o, vvv, myName, QString(p.name()), orm_rowid);
                    continue;
                }
            }
            if (isPointer(p.userType())) {
                if (isWeakPointer(p.userType())) {
                    continue;
                }
                const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
                if (o) {
                    QVariant vvv;
                    if (isQObject) {
                        vvv = p.read(v.value<QObject*>());
                    }
                    else {
                        vvv = p.readOnGadget(v.value<void*>());
                    }
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
                    QVariant qv;
                    if (isQObject) {
                        qv = p.read(v.value<QObject*>());
                    }
                    else {
                        qv = p.readOnGadget(v.value<void*>());
                    }
                    if (qv.isValid()) {
                        QSequentialIterable si = qv.value<QSequentialIterable>();
                        for (QVariant v : si) {
                            meta_insert(*o, v, myName, QString(p.name()), orm_rowid);
                        }
                    }
                    continue;
                }
            }
            //if (isAssociativeContainer(p.userType())) {
            //    const QMetaObject * o = QMetaType::metaObjectForType(getAssociativeContainerStoredValueType(p.userType()));
            //    if (o) {
            //        QVariant qv = meta_select(*o, myName, QString(p.name()), orm_rowid);
            //        qDebug() << meta.property(i).name() << qv << meta.property(i).writeOnGadget(variant, qv);
            //        continue;
            //    }
            //}
        }
    }
}

void ORM::meta_update(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    QString query_text = QString("UPDATE %1 SET ").arg(myName);
    QStringList sets;
    if (!withRowid(meta)) {
        qWarning() << "update: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!isPrimitive(p.userType()) && (!p.isStored() || isGadget(p.userType()) || isPointer(p.userType()) ||
               isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
            continue;
        }
        if (p.name() == QString("orm_rowid")) {
            continue;
        }
        sets << normalize(p.name()) + " = :" + p.name();
    }
    query_text += sets.join(',') + " WHERE orm_rowid = :orm_rowid AND parent_orm_rowid = :parent_orm_rowid AND property_name = :property_name;";
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.prepare(query_text);
    query.bindValue(":parent_orm_rowid", parent_orm_rowid);
    query.bindValue(":property_name", property_name);
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!isPrimitive(p.userType()) && (!p.isStored() || !p.isReadable() || isGadget(p.userType()) || isPointer(p.userType()) ||
               isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
            continue;
        }
        if (isQObject) {
            query.bindValue(QString(":") + p.name(), p.read(v.value<QObject*>()));
        }
        else {
            query.bindValue(QString(":") + p.name(), p.readOnGadget(v.value<void*>()));
        }
    }
    query.exec();
    if (query.lastError().isValid()) {
        qDebug() << meta.className() << myName << property_name;
        qDebug() << query.lastError().databaseText() << query.lastError().driverText();
        qDebug() << query.lastQuery();
        qDebug() << query.boundValues();
        return;
    }

    QSqlQuery query2(QSqlDatabase::database(m_databaseName, true));
    query2.exec("select changes() as ch;");
    if (query2.lastError().isValid()) {
        qDebug() << query2.lastError().databaseText() << query2.lastError().driverText();
        qDebug() << query2.lastQuery();
        qDebug() << query2.boundValues();
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
                QVariant vvv;
                if (isQObject) {
                    vvv = p.read(v.value<QObject*>());
                }
                else {
                    vvv = p.readOnGadget(v.value<void*>());
                }
                meta_update(*o, vvv, myName, QString(p.name()), orm_rowid);
                continue;
            }
        }
        if (isPointer(p.userType())) {
            if (isWeakPointer(p.userType())) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
            if (o) {
                QVariant vvv;
                if (isQObject) {
                    vvv = p.read(v.value<QObject*>());
                }
                else {
                    vvv = p.readOnGadget(v.value<void*>());
                }
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
                QVariant qv;
                if (isQObject) {
                    qv = p.read(v.value<QObject*>());
                }
                else {
                    qv = p.readOnGadget(v.value<void*>());
                }
                if (qv.isValid()) {
                    QSequentialIterable si = qv.value<QSequentialIterable>();
                    for (QVariant v : si) {
                        meta_update(*o, v, myName, QString(p.name()), orm_rowid);
                    }
                }
                continue;
            }
        }
        //if (isAssociativeContainer(p.userType())) {
        //    const QMetaObject * o = QMetaType::metaObjectForType(getAssociativeContainerStoredValueType(p.userType()));
        //    if (o) {
        //        QVariant qv = meta_select(*o, myName, QString(p.name()), orm_rowid);
        //        qDebug() << meta.property(i).name() << qv << meta.property(i).writeOnGadget(variant, qv);
        //        continue;
        //    }
        //}
    }
}

void ORM::meta_delete(const QMetaObject &meta, QVariant &v, QString const& parent_name, QString const& property_name, long long parent_orm_rowid)
{
    QString myName = parent_name + normalize(QString(meta.className()));
    int classtype = QMetaType::type(meta.className());
    bool isQObject = meta.inherits(QMetaType::metaObjectForType(QMetaType::QObjectStar));
    long long orm_rowid = 0;
    if (!withRowid(meta)) {
        qWarning() << "delete: Opperation is not supported without orm_rowids. Type" << meta.className();
        return;
    }
    for (int i = 0; i < meta.propertyCount(); ++i) {
        QMetaProperty p = meta.property(i);
        if (!isPrimitive(p.userType()) && (!p.isStored() || !p.isReadable() || isGadget(p.userType()) || isPointer(p.userType()) ||
               isSequentialContainer(p.userType()) || isAssociativeContainer(p.userType()))) {
            continue;
        }
        if (p.name() == QString("orm_rowid")) {
            if (isQObject) {
                orm_rowid = p.read(v.value<QObject*>()).toLongLong();
            }
            else {
                orm_rowid = p.readOnGadget(v.value<void*>()).toLongLong();
            }
            break;;
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
                    QVariant vvv;
                    if (isQObject) {
                        vvv = p.read(v.value<QObject*>());
                    }
                    else {
                        vvv = p.readOnGadget(v.value<void*>());
                    }
                    meta_delete(*o, vvv, myName, QString(p.name()), orm_rowid);
                    continue;
                }
            }
            if (isPointer(p.userType())) {
                if (isWeakPointer(p.userType())) {
                    continue;
                }
                const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
                if (o) {
                    QVariant vvv;
                    if (isQObject) {
                        vvv = p.read((v.value<QObject*>()));
                    }
                    else {
                        vvv = p.readOnGadget((v.value<void*>()));
                    }
                    meta_delete(*o, vvv, myName, QString(p.name()), orm_rowid);
                    continue;
                }
            }
            if (isSequentialContainer(p.userType())) {
                const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
                if (o) {
                    QVariant qv;
                    if (isQObject) {
                        qv = p.read(v.value<QObject*>());
                    }
                    else {
                        qv = p.readOnGadget(v.value<void*>());
                    }
                    QSequentialIterable si = qv.value<QSequentialIterable>();
                    for (QVariant v : si) {
                        meta_delete(*o, v, myName, QString(p.name()), orm_rowid);
                    }
                    continue;
                }
            }
            //if (isAssociativeContainer(p.userType())) {
            //    const QMetaObject * o = QMetaType::metaObjectForType(getAssociativeContainerStoredValueType(p.userType()));
            //    if (o) {
            //        QVariant qv = meta_select(*o, myName, QString(p.name()), orm_rowid);
            //        qDebug() << meta.property(i).name() << qv << meta.property(i).writeOnGadget(variant, qv);
            //        continue;
            //    }
            //}
        }
    }

    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    if (!parent_orm_rowid) {
        QString query_text = QString("DELETE FROM %1 WHERE orm_rowid = :orm_rowid;").arg(myName);
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
    if (query.lastError().isValid()) {
        qDebug() << meta.className() << myName << property_name;
        qDebug() << query.lastError().databaseText() << query.lastError().driverText();
        qDebug() << query.lastQuery();
        qDebug() << query.boundValues();
    }
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
        if (isGadget(p.userType())) {
            const QMetaObject * o = QMetaType::metaObjectForType(p.userType());
            if (o) {
                meta_drop(*o, myName);
                continue;
            }
        }
        if (isPointer(p.userType())) {
            if (isWeakPointer(p.userType())) {
                continue;
            }
            const QMetaObject * o = QMetaType::metaObjectForType(typeToValue(p.userType()));
            if (o) {
                meta_drop(*o, myName);
                continue;
            }
        }
        if (isSequentialContainer(p.userType())) {
            const QMetaObject * o = QMetaType::metaObjectForType(getSequentialContainerStoredType(p.userType()));
            if (o) {
                meta_drop(*o, myName);
                continue;
            }
        }
        if (isAssociativeContainer(p.userType())) {
            const QMetaObject * o = QMetaType::metaObjectForType(getAssociativeContainerStoredValueType(p.userType()));
            if (o) {
                meta_drop(*o, myName);
                continue;
            }
        }
    }
    QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
    query.exec(query_text);
    if (query.lastError().isValid()) {
        qDebug() << meta.className() << myName;
        qDebug() << query.lastError().databaseText() << query.lastError().driverText();
        qDebug() << query.lastQuery();
        qDebug() << query.boundValues();
    }
}
