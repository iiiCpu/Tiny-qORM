#include "sqliteorm.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>

namespace orm
{




    QString SQLiteORM::normalize(const QString & str) const
    {
        QString s = str;
        static QRegularExpression regExp1 {"(.)([A-Z]+)"};
        static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};
        static QRegularExpression regExp3 {"[:;,.<>*]+"};
        return "_" + s.replace(regExp1, "\\1_\\2").replace(regExp2, "\\1_\\2").toLower().replace(regExp3, "_");
    }

    QString SQLiteORM::normalizeVar(const QString &str, int meta_type_id) const
    {
        Q_UNUSED(meta_type_id)
        return str;
    }

    QString SQLiteORM::sqlType(int meta_type_id) const {
        switch(meta_type_id) {
        case QMetaType::Bool:
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
        case QMetaType::Long:
        case QMetaType::Short:
        case QMetaType::Char:
        case QMetaType::ULong:
        case QMetaType::UShort:
        case QMetaType::UChar:
        case QMetaType::QChar:
            return "INTEGER";
        case QMetaType::Double:
        case QMetaType::Float:
            return "FLOAT";
        case QMetaType::QString:
        case QMetaType::QDate:
        case QMetaType::QTime:
        case QMetaType::QDateTime:
        case QMetaType::QUrl:
        case QMetaType::QUuid:
            return "TEXT";
        }
        if (Impl::isPrimitiveType(meta_type_id)) {
            return "TEXT";
        }
        if (Impl::isPrimitiveString(meta_type_id)) {
            return "TEXT";
        }
        if (Impl::isPrimitiveRaw(meta_type_id)) {
            return "BLOB";
        }
        return "BLOB";
    }

    QString SQLiteORM::generate_table_name(const QString &parent_name, const QString &property_name, const QString &class_name) const
    {
        return parent_name + normalize(class_name) + normalize(property_name);
    }

    QString SQLiteORM::generate_create_query(ORM::TableEntry const& entry, ORM::TableEntry * parent) const
    {
        if (!entry.norm_meta_type_id || entry.is_field) {
            qWarning() << "multiplie orm_rowid properties in " << entry.field_name;
            return QString();
        }
        bool inlinePrimaryKey = false; // in column declaration
        bool externPrimaryKey = false; // in
        QString queryText = QString("CREATE TABLE IF NOT EXISTS %1 (").arg(entry.field_name);
        QStringList queryColumns, keys;
        if (parent) {
            if (parent->has_orm_row_id) {
                for (int i = 0; i < parent->properties.count(); ++i) {
                    if (parent->properties[i].is_field && parent->properties[i].is_orm_row_id) {
                        queryColumns << "parent_" + parent->properties[i].field_name + " " + sqlType(parent->properties[i].norm_meta_type_id)
                                        + " REFERENCES " + parent->field_name + " (" + parent->properties[i].field_name + ") NOT NULL ";
                    }
                }
            }
            else {
                if (parent->has_primary_key) {
                    for (int i = 0; i < parent->properties.count(); ++i) {
                        if (parent->properties[i].is_field && parent->properties[i].is_primary_key)  {
                            queryColumns << "parent_" + parent->properties[i].field_name + " " + sqlType(parent->properties[i].norm_meta_type_id)
                                            + " REFERENCES " + parent->field_name + " (" + parent->properties[i].field_name + ") ";
                            if (parent->properties[i].is_not_null) {
                                queryColumns.last() += " NOT NULL ";
                            }
                        }
                    }
                }
            }
        }
        for (int i = 0; i < entry.properties.count(); ++i) {
            if (!entry.properties[i].is_field) {
                continue;
            }
            queryColumns << entry.properties[i].field_name + " " + sqlType(entry.properties[i].norm_meta_type_id);

            if (entry.has_orm_row_id) {
                if (entry.properties[i].is_orm_row_id) {
                    queryColumns.last() += " PRIMARY KEY AUTOINCREMENT ";
                }
                else {
                    keys << entry.properties[i].field_name;
                }
            }
            else {
                if (entry.properties[i].is_primary_key) {
                    if (entry.properties[i].is_autoincrement) {
                        if (!inlinePrimaryKey) {
                            queryColumns.last() += " PRIMARY KEY AUTOINCREMENT ";
                            inlinePrimaryKey = true;
                        }
                        else {
                            qWarning() << "Multiplie primary keys do not work with AUTOINCREMENT clause! Other primary keys will be stored as common columns.";
                        }
                    }
                    else {
                        keys << entry.properties[i].field_name;
                        externPrimaryKey = true;
                    }
                }
            }
            if (entry.properties[i].is_not_null) {
                queryColumns.last() += " NOT NULL ";
            }
            if (entry.properties[i].is_unique) {
                queryColumns.last() += " UNIQUE ";
            }
        }
        if (queryColumns.size()) {
            queryText += queryColumns.join(", ");
        }
        if (inlinePrimaryKey && externPrimaryKey) {
            qWarning() << "Multiplie primary keys do not work with AUTOINCREMENT clause! Extern PRIMARY KEY clause will be skipped.";
        }
        if (!entry.has_orm_row_id && !inlinePrimaryKey && keys.size()) {
            queryText += ", PRIMARY KEY ( " + keys.join(", ") + ") ";
        }
        queryText += ");";
        return queryText;
    }

    QString SQLiteORM::generate_insert_query(ORM::TableEntry const& entry, ORM::TableEntry * parent) const
    {
        if (!entry.norm_meta_type_id || entry.is_field) {
            qWarning() << "multiplie orm_rowid properties in " << entry.field_name;
            return QString();
        }
        QStringList queryColumns, queryBindings;
        if (parent) {
            if (parent->has_orm_row_id) {
                for (int i = 0; i < parent->properties.count(); ++i) {
                    if (parent->properties[i].is_field && parent->properties[i].is_orm_row_id) {
                        queryColumns << "parent_" + parent->properties[i].field_name;
                        queryBindings << normalizeVar(":parent_" + parent->properties[i].field_name, parent->properties[i].orig_meta_type_id);
                    }
                }
            }
            else {
                if (parent->has_primary_key) {
                    for (int i = 0; i < parent->properties.count(); ++i) {
                        if (parent->properties[i].is_field && parent->properties[i].is_primary_key)  {
                            queryColumns << "parent_" + parent->properties[i].field_name;
                            queryBindings << normalizeVar(":parent_" + parent->properties[i].field_name, parent->properties[i].orig_meta_type_id);
                        }
                    }
                }
            }
        }
        for (auto i : entry.properties) {
            if (!i.is_field || i.is_autoincrement) {
                continue;
            }
            queryColumns << i.field_name;
            queryBindings << normalizeVar(":" + i.field_name, i.orig_meta_type_id); // let it be like this
        }
        // INFO: on SQLite UPSERT (since 3.24.0 (2018-06-04))
        //QStringList upsert;
        //for (int i = 0; i < t_names.size(); ++i) {
        //      upsert << t_names[i] + " = " + t_values[i];
        //}
        return QString("INSERT INTO %1 (%2) VALUES (%3);") //ON CONFLICT DO UPDATE SET %4 WHERE _orm_rowid = :_orm_rowid
                .arg(entry.field_name).arg(queryColumns.join(", ")).arg(queryBindings.join(", "));//.arg(upsert.join(", "));
    }

    QString SQLiteORM::generate_select_query(ORM::TableEntry const& entry, ORM::TableEntry * parent) const
    {
        if (!entry.norm_meta_type_id || entry.is_field) {
            qWarning() << "multiplie orm_rowid properties in " << entry.field_name;
            return QString();
        }
        QString queryText = QString("SELECT ");
        QStringList queryColumns;
        for (int i = 0; i < entry.properties.count(); ++i) {
            if (!entry.properties[i].is_field) {
                continue;
            }
            queryColumns << entry.properties[i].field_name;
        }
        if (queryColumns.size()) {
            queryText += queryColumns.join(", ");
        }
        queryText += QString(" FROM %1").arg(entry.field_name);
        if (parent) {
            queryColumns.clear();
            queryText += " WHERE ";
            if (parent->has_orm_row_id) {
                for (int i = 0; i < parent->properties.count(); ++i) {
                    if (parent->properties[i].is_field && parent->properties[i].is_orm_row_id) {
                        queryColumns << "parent_" + parent->properties[i].field_name + " = " + normalizeVar(":parent_" + parent->properties[i].field_name, parent->properties[i].orig_meta_type_id);
                        break;
                    }
                }
            }
            else {
                if (parent->has_primary_key) {
                    for (int i = 0; i < parent->properties.count(); ++i) {
                        if (parent->properties[i].is_field && parent->properties[i].is_primary_key)  {
                            queryColumns << "parent_" + parent->properties[i].field_name + " = " + normalizeVar(":parent_" + parent->properties[i].field_name, parent->properties[i].orig_meta_type_id);
                        }
                    }
                }
            }
            queryText += queryColumns.join(" AND ");
        }
        queryText += ";";
        return queryText;
    }

    QString SQLiteORM::generate_update_query(ORM::TableEntry const& entry, ORM::TableEntry * parent) const
    {
        if (!entry.has_orm_row_id && ! entry.has_primary_key) {
            qWarning() << "Update<type> does not work for types without primary keys.";
            return QString();
        }
        if (!entry.norm_meta_type_id || entry.is_field) {
            qWarning() << "multiplie orm_rowid properties in " << entry.field_name;
            return QString();
        }
        QString queryText = QString("UPDATE OR IGNORE %1 SET ").arg(entry.field_name);

        QStringList queryColumns;
        for (auto i : entry.properties) {
            if (!i.is_field) {
                continue;
            }
            queryColumns << i.field_name + " = " + normalizeVar(":" + i.field_name, i.orig_meta_type_id); // let it be like this
        }
        queryText += queryColumns.join(", ") + " WHERE " ;
        queryColumns.clear();
        if (parent) {
            if (parent->has_orm_row_id) {
                for (int i = 0; i < parent->properties.count(); ++i) {
                    if (parent->properties[i].is_field && parent->properties[i].is_orm_row_id) {
                        queryColumns << "parent_" + parent->properties[i].field_name + " = " + normalizeVar(":parent_" + parent->properties[i].field_name, parent->properties[i].orig_meta_type_id);
                        break;
                    }
                }
            }
            else {
                if (parent->has_primary_key) {
                    for (int i = 0; i < parent->properties.count(); ++i) {
                        if (parent->properties[i].is_field && parent->properties[i].is_primary_key)  {
                            queryColumns << "parent_" + parent->properties[i].field_name + " = " + normalizeVar(":parent_" + parent->properties[i].field_name, parent->properties[i].orig_meta_type_id);
                        }
                    }
                }
            }
        }
        if (entry.has_orm_row_id) {
            for (int i = 0; i < entry.properties.count(); ++i) {
                if (entry.properties[i].is_field && entry.properties[i].is_orm_row_id) {
                    queryColumns << entry.properties[i].field_name + " = " + normalizeVar(entry.properties[i].field_name, entry.properties[i].orig_meta_type_id);
                    break;
                }
            }
        }
        else {
            if (entry.has_primary_key) {
                for (int i = 0; i < entry.properties.count(); ++i) {
                    if (entry.properties[i].is_field && entry.properties[i].is_primary_key)  {
                        queryColumns << entry.properties[i].field_name + " = " + normalizeVar(entry.properties[i].field_name, entry.properties[i].orig_meta_type_id);
                    }
                }
            }
        }
        queryText += queryColumns.join(" AND ") + ";";
        return queryText;
    }

    QString SQLiteORM::generate_delete_query(ORM::TableEntry const& entry, ORM::TableEntry * parent) const
    {
        QString queryText = QString("DELETE FROM %1 WHERE ").arg(entry.field_name);

        QStringList queryColumns;
        if (parent) {
            if (parent->has_orm_row_id) {
                for (int i = 0; i < parent->properties.count(); ++i) {
                    if (parent->properties[i].is_field && parent->properties[i].is_orm_row_id) {
                        queryColumns << "parent_" + parent->properties[i].field_name + " = " + normalizeVar(":parent_" + parent->properties[i].field_name, parent->properties[i].orig_meta_type_id);
                        break;
                    }
                }
            }
            else {
                if (parent->has_primary_key) {
                    for (int i = 0; i < parent->properties.count(); ++i) {
                        if (parent->properties[i].is_field && parent->properties[i].is_primary_key)  {
                            queryColumns << "parent_" + parent->properties[i].field_name + " = " + normalizeVar(":parent_" + parent->properties[i].field_name, parent->properties[i].orig_meta_type_id);
                        }
                    }
                }
            }
        }
        if (entry.has_orm_row_id) {
            for (int i = 0; i < entry.properties.count(); ++i) {
                if (entry.properties[i].is_field && entry.properties[i].is_orm_row_id) {
                    queryColumns << entry.properties[i].field_name + " = " + normalizeVar(entry.properties[i].field_name, entry.properties[i].orig_meta_type_id);
                    break;
                }
            }
        }
        else {
            if (entry.has_primary_key) {
                for (int i = 0; i < entry.properties.count(); ++i) {
                    if (entry.properties[i].is_field && entry.properties[i].is_primary_key)  {
                        queryColumns << entry.properties[i].field_name + " = " + normalizeVar(entry.properties[i].field_name, entry.properties[i].orig_meta_type_id);
                    }
                }
            }
        }
        queryText += queryColumns.join(" AND ") + ";";
        return queryText;
    }

    QString SQLiteORM::generate_drop_query(ORM::TableEntry const& entry, ORM::TableEntry * parent) const
    {
        return QString("DROP TABLE IF EXISTS %1;").arg(entry.field_name);
    }

    long long SQLiteORM::get_last_inserted_rowid(QSqlQuery &query) const
    {
        return query.lastInsertId().toLongLong();
    }
    long long SQLiteORM::get_changes (QSqlQuery &query) const
    {
        return query.numRowsAffected();
    }

    long long SQLiteORM::get_last_rowid(QString table_name) const
    {
        QSqlQuery query(QSqlDatabase::database(m_databaseName, true));
        QString name = normalize(ORM_Impl::rowidName());
        query.exec("SELECT " + name + " FROM " + table_name + " ORDER BY " + name + " DESC LIMIT 1;" );
        if (query.lastError().isValid()) {
            qDebug() << table_name;
            qDebug() << query.lastError().databaseText() << query.lastError().driverText();
            qDebug() << query.executedQuery();
            qDebug() << query.boundValues();
            return 0;
        }
        if (!query.next()) {
            return 0;
        }
        return query.value(0).toLongLong();
    }

}
