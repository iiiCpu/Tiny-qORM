#ifndef ORM_TABLE_MODEL_H
#define ORM_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QMetaProperty>
#include <orm_def.h>

namespace orm
{

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

    class table_manager;

    class ORM_SHARED_EXPORT table_model_base : public QAbstractTableModel
    {
        Q_OBJECT
    protected:
        enum TransactionState {
            Begin,
            Continue,
            Pending,
            Success,
            Fail
        };

        QVector<QMap<int, QVariant>> m_headerData;
        QVector<Qt::ItemFlags> m_defaultFlags;

    public:
        explicit table_model_base(QObject *parent = nullptr)
            : QAbstractTableModel(parent)
        {

        }
        virtual ~table_model_base() = default;

    public:
        Q_INVOKABLE TransactionState beginTransaction() { return TransactionState::Success; };
        Q_INVOKABLE TransactionState endTransaction() { return TransactionState::Success; };
        Q_INVOKABLE TransactionState pushCommand() { return TransactionState::Success; };

    public slots:
        void onPropertyChanged(QVariant const& newValue = QVariant())
        {

        }
    };

    template <typename T>
    class ORM_SHARED_EXPORT table_model : public table_model_base
    {
    protected:
        using Tp = typename std::conditional<std::is_base_of<QObject,T>::value, T*, T>::type;
        using Tsp = typename std::conditional<std::is_base_of<QObject,T>::value, QSharedPointer<T>, T>::type;
        QVector<Tsp> m_data;
    public:
        explicit table_model(QObject *parent = nullptr)
            : table_model_base(parent)
        {
            QMetaObject const* obj = QMetaType(qMetaTypeId<T>()).metaObject();
            for (int i = 0; i < obj->propertyCount(); ++i) {
                QMetaProperty prop = obj->property(i);
                if (!prop.isReadable()) {
                    continue;
                }
                Qt::ItemFlags flags;
                flags.setFlag(Qt::ItemIsSelectable       , true);
                flags.setFlag(Qt::ItemIsEnabled          , true);
                flags.setFlag(Qt::ItemNeverHasChildren   , true);
                if (prop.isRequired()) {

                }
                if (prop.isWritable()) {
                    flags.setFlag(Qt::ItemIsEditable     , true);
                }
                m_defaultFlags.append(flags);
                QMap<int, QVariant> roles;
                roles[Qt::DisplayRole] = QString(prop.name());
                m_headerData.append(roles);
            }
        }
        virtual ~table_model() = default;

    public:
        virtual bool insert(Tp t, int row_before = -1, const QModelIndex &parent = QModelIndex())
        {
            if (parent.isValid()) {
                return false;
            }
            if (0 > row_before || m_data.size() < row_before) {
                row_before = m_data.size();
            }
            if (insertRows(row_before, 1, parent)) {
                m_data[row_before] = Tsp(t);
                return true;
            }
            return false;
        }
        virtual bool insert(Tsp t, int row_before = -1, const QModelIndex &parent = QModelIndex())
        {
            if (parent.isValid()) {
                return false;
            }
            if (0 > row_before || m_data.size() < row_before) {
                row_before = m_data.size();
            }
            if (insertRows(row_before, 1, parent)) {
                m_data[row_before] = t;
                return true;
            }
            return false;
        }
        virtual Tsp const& at(int row, const QModelIndex &parent = QModelIndex()) const
        {
            static Tsp r;
            if (parent.isValid()) {
                return r = Tsp{};
            }
            if (0 > row || m_data.size() <= row) {
                return r = Tsp{};
            }
            return m_data[row];
        }
        virtual Tsp & at(int row, const QModelIndex &parent = QModelIndex())
        {
            static Tsp r;
            if (parent.isValid()) {
                return r = Tsp{};
            }
            if (0 > row || m_data.size() <= row) {
                return r = Tsp{};
            }
            return m_data[row];
        }
        virtual Tsp && take(int row, const QModelIndex &parent = QModelIndex())
        {
            if (parent.isValid()) {
                return Tsp{};
            }
            if (0 > row || m_data.size() <= row) {
                return Tsp{};
            }
            return m_data.takeAt(row);
        }

    public:
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override
        {
            if (parent.isValid()) {
                return 0;
            }
            return m_headerData.size();
        }

        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
        {
            if (!index.isValid()) {
                return QVariant();
            }
            if (index.parent().isValid()) {
                return QVariant();
            }

            switch (role) {
                case Qt::ItemDataRole::DisplayRole:
                case Qt::ItemDataRole::EditRole:
                {
                    return orm::_data(m_data[index.row()], index.column());
                }
            }
            return QVariant();
        }

        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
        {
            if (Qt::Orientation::Vertical == orientation) {
                return QVariant();
            }
            if (m_headerData.size() <= section || 0 > section) {
                return QVariant();
            }
            return m_headerData[section].value(role, QVariant());
        }

        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
        {
            if (parent.isValid()) {
                return QModelIndex();
            }
            if (0 > row || 0 > column || m_data.size() <= row || m_headerData.size() <= column) {
                return QModelIndex();
            }
            return createIndex(row, column);
        }

        virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
        {
            if (parent.isValid()) {
                return false;
            }
            if (0 > row || m_data.size() < row) {
                return false;
            }
            auto transaction = beginTransaction();
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            bool rrem = true;

            beginInsertRows(parent, row, row+count-1);
            m_data.insert(m_data.begin() + row, count, Tsp{});
            endInsertRows();

            if (rrem) {
                transaction = pushCommand();
            }
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            if (rrem) {
                transaction = endTransaction();
            }
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            return rrem;
        }

        virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override
        {
            if (sourceParent.isValid()) {
                return false;
            }
            if (destinationParent.isValid()) {
                return false;
            }
            auto transaction = beginTransaction();
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            bool rrem = true;
            beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild);
            std::swap_ranges(m_data.begin() + sourceRow, m_data.begin()+sourceRow+count, m_data.begin()+destinationChild);
            endMoveRows();

            if (rrem) {
                transaction = pushCommand();
            }
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            if (rrem) {
                transaction = endTransaction();
            }
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            return true;
        }

        virtual QModelIndex parent(const QModelIndex &index) const override
        {
            return QModelIndex();
        }

        virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override
        {
            if (parent.isValid()) {
                return false;
            }
            if (0 > row || m_data.size() < row + count) {
                return false;
            }
            auto transaction = beginTransaction();
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            bool rrem = true;

            beginRemoveRows(parent, row, row+count-1);
            m_data.erase(m_data.begin() + row, m_data.begin() + row + count);
            endRemoveRows();

            if (rrem) {
                transaction = pushCommand();
            }
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            if (rrem) {
                transaction = endTransaction();
            }
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            return rrem;
        }

        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override
        {
            if (parent.isValid()) {
                return 0;
            }
            return m_data.size();
        }

        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
        {
            if (!index.isValid()) {
                return false;
            }
            if (index.parent().isValid()) {
                return false;
            }

            auto transaction = beginTransaction();
            if (orm::table_model_base::TransactionState::Fail == transaction) {

            }
            bool sset = false;

            switch (role) {
                case Qt::ItemDataRole::DisplayRole:
                case Qt::ItemDataRole::EditRole:
                {
                    sset = orm::_setdata(m_data[index.row()], index.column(), value);
                }
                break;
                default:
                    sset = orm::table_model_base::setData(index, value, role);
            }
            if (true == sset) {
                emit dataChanged(index, index, {role});
                transaction = pushCommand();
            }
            transaction = endTransaction();
            return sset;
        }

        virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override
        {
            if (Qt::Orientation::Vertical == orientation) {
                return false;
            }
            if (m_headerData.size() <= section || 0 > section) {
                return false;
            }
            if (headerData(section, orientation, role) == value) {
                return false;
            }
            if (!value.isValid()) {
                m_headerData[section].remove(role);
            }
            else {
                m_headerData[section][role] = value;
            }
            emit headerDataChanged(orientation, section, section);
            return true;
        }
    public:

    };

    class ORM_SHARED_EXPORT table_manager : public QObject
    {
        Q_OBJECT
    protected:
        QMap<QString, QSharedPointer<QAbstractItemModel>> m_tables;
    public:
        explicit table_manager(QObject* parent = nullptr)
            : QObject(parent)
            {
            };
        virtual ~table_manager() = default;

        template <typename T>
        QSharedPointer<table_model<T>> const& manager(QString const& name)
        {
            assert(!name.isEmpty());
            if (!m_tables.contains(name)){
                QSharedPointer<table_model<T>> model = QSharedPointer(new table_model<T>(this));
                model->setObjectName(name);
                m_tables[name] = model;
            }
            QSharedPointer<QAbstractItemModel> mdl = m_tables[name];
            QSharedPointer<table_model<T>> model(qobject_cast<table_model<T>>(mdl.data()));
            return model; // QSharedPointer<table_model<T>>(m_tables[name].objectCast<table_model<T>>());
        }
        QSharedPointer<QAbstractItemModel> & manager(QString const& name)
        {
            assert(m_tables.contains(name));
            return m_tables[name];
        }
        bool addManager(QAbstractItemModel * model)
        {
            assert(model);
            assert(!model->objectName().isEmpty());
            if (m_tables.contains(model->objectName())) {
                return false;
            }
            m_tables[model->objectName()] = QSharedPointer<QAbstractItemModel>(model);
            model->setParent(this);
            return true;
        }
    };
}
#endif // ORM_TABLE_MODEL_H
