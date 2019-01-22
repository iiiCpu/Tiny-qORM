#ifndef SQLITEORM_H
#define SQLITEORM_H

#include "orm.h"

namespace orm
{

    class SQLiteORM : public ORM
    {
    public:
        SQLiteORM(QString const& dbname = QString()) : ORM(dbname) {}
    protected:
        virtual long long get_last_inserted_rowid (QSqlQuery & query)  const;
        virtual long long get_changes             (QSqlQuery & query)  const;
        virtual long long get_last_rowid          (QString table_name) const;

        virtual QString normalize                 (QString const& str)                   const;
        virtual QString normalizeVar              (QString const& str, int meta_type_id) const;
        virtual QString sqlType                   (int type_id)                          const;
        virtual QString generate_table_name        (QString const& parent_name, QString const& property_name, QString const& class_name) const;

        virtual QString generate_create_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const;
        virtual QString generate_insert_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const;
        virtual QString generate_select_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const;
        virtual QString generate_update_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const;
        virtual QString generate_delete_query      (ORM::TableEntry const& entry, ORM::TableEntry * parent) const;
        virtual QString generate_drop_query        (ORM::TableEntry const& entry, ORM::TableEntry * parent) const;
    };

}

#endif // SQLITEORM_H
