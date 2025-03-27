#ifndef ORM_TABLE_MODEL_H
#define ORM_TABLE_MODEL_H

#include "orm_table_model_base.h"

namespace orm
{
    class table_manager;

    template <typename T>
    class table_model : public table_model_base
    {
        friend T;
    public:
        using Tp = T*;
        using Tsp = QSharedPointer<T>;
    protected:
        mutable QVector<Tsp>    m_data;
    public:
        explicit table_model(QObject *parent = nullptr);
        virtual ~table_model();

        static table_model<T> * model(QObject * el);

        virtual int propertyNameToColumn(QByteArray const& p) const override;

    public:

        template <typename U>
        typename std::enable_if< (!std::is_same<T, U>::value) && std::is_base_of<T, U>::value &&
                                   std::is_base_of<QObject,U>::value, bool>::type
        insert(QSharedPointer<U> u, int row_before = -1, const QModelIndex &parent = QModelIndex());

        bool insert(Tp t, int row_before = -1, const QModelIndex &parent = QModelIndex());

        virtual bool insert(Tsp t, int row_before = -1, const QModelIndex &parent = QModelIndex());

        template <typename U>
        typename std::enable_if< (std::is_same<T, typename std::remove_cv<U>::type>::value &&
                                  std::is_copy_constructible<U>::value), bool>::type
        insert(U const& t, int row_before = -1, const QModelIndex &parent = QModelIndex());

        virtual Tsp const& at(int row, const QModelIndex &parent = QModelIndex()) const;
        virtual Tsp at(const QModelIndex &index);
        virtual Tsp at(int row, const QModelIndex &parent = QModelIndex());
        virtual Tsp && take(int row, const QModelIndex &parent = QModelIndex());

    protected:
        bool any(std::function<bool(Tsp const&)> const& f) const;
        bool all(std::function<bool(Tsp const&)> const& f) const;

    public:
        // check conditions on columns on insert or update
        // checks run in exact order
        // put light checks (like type or value limits) into [check_custom_light]
        // and heavy checks (like circular or foreign table search) into [check_custom_heavy]
        bool check_all(Tp t, int column = -1, QVariant const& value = QVariant()) const;
    protected:
        virtual bool check_custom_light(Tp t, int column = -1, QVariant const& value = QVariant()) const;
        inline  bool check_not_null    (Tp t, int column = -1, QVariant const& value = QVariant()) const;
        inline  bool check_unique      (Tp t, int column = -1, QVariant const& value = QVariant()) const;
        //inline  bool check_foreign_key (Tp t, int column = -1, QVariant const& value = QVariant()) const;
        virtual bool check_custom_heavy(Tp t, int column = -1, QVariant const& value = QVariant()) const;

    public:
        virtual void onPropertyChanged(QVariant const& newValue = QVariant()) override;
        virtual void onForeignDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                   const QList<int> &roles = QList<int>()) override;
        virtual void onForeignRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last) override;

    public:
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
        virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                              const QModelIndex &destinationParent, int destinationChild) override;
        virtual QModelIndex parent([[maybe_unused]] const QModelIndex &index) const override;
        virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        virtual bool setHeaderData(int section, Qt::Orientation orientation,
                                   const QVariant &value, int role = Qt::EditRole) override;
    public:

    };

}
#endif // ORM_TABLE_MODEL_H
