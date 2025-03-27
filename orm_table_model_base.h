#ifndef ORM_TABLE_MODEL_BASE_H
#define ORM_TABLE_MODEL_BASE_H

#include <QAbstractTableModel>
#include <QObject>
#include <QMetaProperty>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QPointer>

#include "orm_def.h"

#ifdef LOGGER_HELPER


#endif

#include <QtCommonTools/errorhelper.h>
Q_DECLARE_EXPORTED_LOGGING_CATEGORY(c_orm_table, ORM_SHARED_EXPORT)


namespace orm
{
    class table_manager;
    namespace table_command
    {
        class ORM_SHARED_EXPORT base_command
        {
        protected:
            QWeakPointer<QAbstractTableModel> table;
        public:

        };
        class ORM_SHARED_EXPORT table_command_transaction : public base_command
        {
            Q_GADGET
        protected:
            QList<base_command> commands;
        public:

        };

        class ORM_SHARED_EXPORT insert_rows : public base_command
        {
            Q_GADGET
        protected:
            int row;
            int count;
        public:
        };

        class ORM_SHARED_EXPORT insert_columns : public base_command
        {
            Q_GADGET
        protected:
            int column;
            int count;
        public:
        };

        class ORM_SHARED_EXPORT remove_rows : public base_command
        {
            Q_GADGET
        protected:
            int row;
            int count;
        public:
        };

        class ORM_SHARED_EXPORT remove_columns : public base_command
        {
            Q_GADGET
        protected:
            int column;
            int count;
        public:
        };

        class ORM_SHARED_EXPORT move_rows : public base_command
        {
            Q_GADGET
        protected:
            int row_from;
            int row_to;
            int count;
        public:
        };

        class ORM_SHARED_EXPORT move_columns : public base_command
        {
            Q_GADGET
        protected:
            int column_from;
            int column_to;
            int count;
        public:
        };

        class ORM_SHARED_EXPORT set_data : public base_command
        {
            Q_GADGET
        protected:
            int row;
            int column;
            int role;
            QVariant data;
        public:
        };
    }

    class ORM_SHARED_EXPORT table_model_base : public QAbstractItemModel
    {
        Q_OBJECT
    public:
        static constexpr const char* orm_primary_key  = "orm_primary_key"     ;
        // Q_CLASSINFO(orm_primary_key, "pk_column")
        static constexpr const char* orm_hidden       = "orm_hidden"          ;
        // Q_CLASSINFO(orm_hidden, "hidden_1,hidden_2")
        // Q_CLASSINFO(orm_hidden, "hidden_3")
        static constexpr const char* orm_not_null     = "orm_not_null"        ;
        // Q_CLASSINFO(orm_not_null, "not_null_1,not_null_2")
        // Q_CLASSINFO(orm_not_null, "not_null_3")
        static constexpr const char* orm_unique       = "orm_unique"          ;
        // Q_CLASSINFO(orm_unique, "unique_1_1,unique_1_2;unique_2")
        // Q_CLASSINFO(orm_unique, "unique_3")
        static constexpr const char* orm_foreign_keys = "orm_foreign_keys"    ;
        // Q_CLASSINFO(orm_foreign_keys, "ref_1_1,ref_1_2=foreign_table_1.fk_1_1,fk_1_2;ref_2=foreign_table_2.fk_2")
        // Q_CLASSINFO(orm_foreign_keys, "ref_3=foreign_table_3.fk_3")

    protected:
        enum TransactionState {
            Begin,
            Continue,
            Pending,
            Success,
            Fail,
            NotImplementedYet
        };
        enum LockOperation {
            LockNone,
            LockPreview,
            LockEdit,
            LockEditSub,
            LockRemove,
            LockNotImplementedYet
        };
        enum OnDelete{
            NO_ACTION,
            CASCADE,
            RESTRICT,
            SET_NULL,
            SET_DEFAULT,
        };

        struct sForeignKey {
            //QByteArrayList m_reference_properties;
            QString m_foreign_table;
            QByteArrayList m_foreign_columns;
            OnDelete m_on_delete;
        };
        struct sForeignKeyI {
            //QList<int> m_reference_properties;
            QPointer<QAbstractItemModel> m_foreign_table;
            QList<int> m_foreign_columns;
            OnDelete m_on_delete;
        };

        QVector<QMap<int, QVariant>> m_headerData;       // column <role,value>
        QVector<Qt::ItemFlags>       m_defaultFlags;     // column <flag>
        QList<int>                   m_headerToProp;     // column <property_index>
        QList<int>                   m_property_signals; // property_index <signal_index>
        QSet<int>                    m_changed;          // set of <row_index>

        QByteArray                                           m_primary_key; //  property_name
        QByteArrayList                                       m_hidden;      // list <property_name>
        QByteArrayList                                       m_not_null;    // list <property_name>
        QList<QByteArrayList>                                m_unique;      // list of unique_and_condition <property_name>
        QHash<QByteArrayList, sForeignKey> m_foreign_key; // <local_property_names,<reference_model_name, reference_property_names>>

        int               m_primary_key_i;                     //  column_index
        QList<int>        m_hidden_i;                          // <column_index>
        QList<int>        m_not_null_i;                        // <column_index>
        QList<QList<int>> m_unique_i;                          // unique_and_condition<column_index>
        mutable QHash<QList<int>, sForeignKeyI> m_foreign_key_i; // <local_property_indexes, <reference_model_name, reference_model_columns>>

    protected:
        table_model_base(QMetaObject const* obj, QObject *parent);
        void init_row(QObject * obj);
        void purge_row(QObject * obj);

    public:
        explicit table_model_base(QObject *parent = nullptr);
        virtual ~table_model_base();

        void setManager(table_manager * mgr);
        table_manager * manager() const;

        virtual int propertyNameToColumn(QByteArray const& p) const;
        int propertyToColumn(int p) const;
        int columnToProperty(int c) const;

        virtual QModelIndex reference(QModelIndex const& ref) const;
    public:
        Q_INVOKABLE TransactionState beginTransaction([[maybe_unused]] QVariantMap const& securityInfo = QVariantMap());
        Q_INVOKABLE TransactionState endTransaction([[maybe_unused]] QVariantMap const& securityInfo = QVariantMap());
        Q_INVOKABLE TransactionState pushCommand([[maybe_unused]] QVariantMap const& securityInfo = QVariantMap());

        Q_INVOKABLE bool isLocked([[maybe_unused]] QModelIndexList const& indexes, [[maybe_unused]] int operation, [[maybe_unused]] QVariantMap const& securityInfo);
        Q_INVOKABLE bool lock([[maybe_unused]] QModelIndexList const& indexes, [[maybe_unused]] int operation, [[maybe_unused]] QVariantMap const& securityInfo);
        Q_INVOKABLE bool unlock([[maybe_unused]] QModelIndexList const& indexes, [[maybe_unused]] int operation, [[maybe_unused]] QVariantMap const& securityInfo);
        Q_INVOKABLE bool maybeLock([[maybe_unused]] QModelIndexList const& indexes, [[maybe_unused]] int operation, [[maybe_unused]] QVariantMap const& securityInfo);
        Q_INVOKABLE bool maybeUnlock([[maybe_unused]] QModelIndexList const& indexes, [[maybe_unused]] int operation, [[maybe_unused]] QVariantMap const& securityInfo);
    public slots:
        virtual void onPropertyChanged(QVariant const& newValue = QVariant());
        virtual void onForeignDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                   const QList<int> &roles = QList<int>());
        virtual void onForeignRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
        void onManagerDestroyed(QObject *);
    signals:
        void managerChanged();
    };


}

#endif // ORM_TABLE_MODEL_BASE_H
