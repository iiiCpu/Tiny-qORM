#ifndef ORM_TABLE_MODEL_HPP
#define ORM_TABLE_MODEL_HPP

#include <qORM/orm_table_model.h>

#include <QMetaObject>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaClassInfo>

template<typename T> template <typename U>
typename std::enable_if< (std::is_same<T, typename std::remove_cv<U>::type>::value && std::is_copy_constructible<U>::value), bool>::type
orm::table_model<T>::insert(U const& t, int row_before, const QModelIndex &parent)
{
    return insert(orm::table_model<T>::Tsp(new T(t)), row_before, parent);
}


template <typename T> template <typename U>
typename std::enable_if< (!std::is_same<T, U>::value) && std::is_base_of<T, U>::value && std::is_base_of<QObject,U>::value, bool>::type
orm::table_model<T>::insert(QSharedPointer<U> u, int row_before, const QModelIndex & parent)
{
    return insert(u.template dynamicCast<T>(), row_before, parent);
}

template<typename T>
orm::table_model<T>::table_model(QObject * parent)
    : orm::table_model_base(QMetaType(qMetaTypeId<T>()).metaObject(), parent)
{
    auto ret = NewTraceC(c_orm_table())
                   .object(this)
                   .thread()
                   .template metaType<T>()
                   .function("explicit orm::table_model<T>::table_model(QObject *parent = nullptr)")
                   .functionArgument("parent", parent)
                   .post();

}

template<typename T>
orm::table_model<T>::~table_model()
{
    m_data.clear();
}

template<typename T>
orm::table_model<T> * orm::table_model<T>::model(QObject * el)
{
    if (nullptr == el) {
        return nullptr;
    }
    orm::table_model_base * pret = qobject_cast<orm::table_model_base*>(el->parent());
    if (nullptr == pret) {
        return nullptr;
    }
    orm::table_model<T> * ret = dynamic_cast<orm::table_model<T>*>(pret);
    return ret;
}

template<typename T>
int orm::table_model<T>::propertyNameToColumn(const QByteArray & p) const
{
    return m_headerToProp.indexOf(QMetaType(qMetaTypeId<T>()).metaObject()->indexOfProperty(p));
}

template<typename T>
bool orm::table_model<T>::insert(orm::table_model<T>::Tp t, int row_before, const QModelIndex & parent)
{
    return insert(orm::table_model<T>::Tsp(t), row_before, parent);
}

template<typename T>
bool orm::table_model<T>::insert(orm::table_model<T>::Tsp t, int row_before, const QModelIndex & parent)
{
    auto ret = NewTraceC(c_orm_table())
                   .object(this)
                   .thread()
                   .template metaType<T>()
                   .function("virtual bool orm::table_model<T>::insert(Tp t, int row_before = -1, const QModelIndex &parent = QModelIndex())")
                   //.functionArgument("t", t)
                   .functionArgument("row_before", row_before)
                   .functionArgument("parent", parent)
                   .post()
                   .template ret<bool>(false);
    if (t.isNull()) {
        return ret = false;
    }
    if (parent.isValid()) {
        return ret = false;
    }
    if (0 > row_before || m_data.size() < row_before) {
        row_before = m_data.size();
    }
    QObject * old_parent = nullptr;
    if constexpr (std::is_base_of<QObject,T>()) {
        old_parent = t->parent();
        t->setParent(this);
        if (!check_all(t.data())) {
            t->setParent(old_parent);
            return ret = false;
        }
    }
    else {
        if (!check_all(t.data())) {
            return ret = false;
        }
    }
    if (insertRows(row_before, 1, parent)) {
        if constexpr (std::is_base_of<QObject,T>()) {
            purge_row(m_data[row_before].data());
            init_row(t.data());
        }
        m_data[row_before] = t;
        return ret = true;
    }
    if constexpr (std::is_base_of<QObject,T>()) {
        t->setParent(old_parent);
    }
    return ret = false;
}

template<typename T>
const typename orm::table_model<T>::Tsp & orm::table_model<T>::at(int row, const QModelIndex & parent) const
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

template<typename T>
typename orm::table_model<T>::Tsp orm::table_model<T>::at(const QModelIndex & index)
{
    if (!index.isValid() || index.model() != this || index.parent().isValid()) {
        return Tsp{};
    }
    if (0 > index.row() || m_data.size() <= index.row()) {
        return Tsp{};
    }
    return m_data[index.row()];
}

template<typename T>
typename orm::table_model<T>::Tsp orm::table_model<T>::at(int row, const QModelIndex & parent)
{
    if (parent.isValid()) {
        return Tsp{};
    }
    if (0 > row || m_data.size() <= row) {
        return Tsp{};
    }
    return m_data[row];
}

template<typename T>
typename orm::table_model<T>::Tsp && orm::table_model<T>::take(int row, const QModelIndex & parent)
{
    static Tsp r;
    if (parent.isValid()) {
        return std::move(r = Tsp{});
    }
    if (0 > row || m_data.size() <= row) {
        return std::move(r = Tsp{});
    }
    Tsp tsp(m_data[row]);
    if constexpr (std::is_base_of<QObject, T>()) {
        purge_row(tsp.data());
    }
    m_data[row].reset();
    removeRow(row, parent);
    return std::move(r = tsp);
}

template<typename T>
bool orm::table_model<T>::any(const std::function<bool (const Tsp &)> & f) const
{
    return std::any_of(m_data.cbegin(), m_data.cend(), f);
}

template<typename T>
bool orm::table_model<T>::all(const std::function<bool (const Tsp &)> & f) const
{
    return std::all_of(m_data.cbegin(), m_data.cend(), f);
}


template<typename T>
void orm::table_model<T>::onPropertyChanged(QVariant const& newValue)
{

}
template<typename T>
void orm::table_model<T>::onForeignDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                  const QList<int> &roles)
{

}

template<typename T>
void orm::table_model<T>::onForeignRowsAboutToBeRemoved(const QModelIndex & parent, int first, int last)
{
    if (parent.isValid()) {
        return;
    }
    QSet<int> to_remove;
    for (auto i = m_foreign_key_i.cbegin(); i != m_foreign_key_i.cend(); ++i) {
        QPointer<QAbstractItemModel> foreign_model = i.value().m_foreign_table;
        if (!foreign_model) {
            qCDebug(c_orm_table()) << "Foreign key error: no table " << i.key();
            continue;
        }
        for (int r = first; r <= last; ++r) {
            QVariantList vs;
            for (auto j = i.value().m_foreign_columns.cbegin(); j != i.value().m_foreign_columns.cend(); ++j) {
                vs << foreign_model->index(r, *j).data();
            }
            for (int v = 0; v < m_data.size(); ++v) {
                bool skip = false;
                for (int j = 0; j < i.key().size(); ++j) {
                    QVariant dat = orm::_data(m_data[v], i.key()[j]);
                    if (dat.isNull() || dat != vs[j]) {
                        skip = true;
                        break;
                    }
                }
                if (!skip) {
                    to_remove << v;
                }
            }
        }
    }
    QList<int> tor(to_remove.begin(), to_remove.end());
    for (auto i = tor.rbegin(); i != tor.rend(); ++i) {
        removeRow(*i);
    }
}

template<typename T>
QVariant orm::table_model<T>::data(const QModelIndex & index, int role) const
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
            return orm::_data(m_data[index.row()], columnToProperty(index.column()));
        }
    }
    return QVariant();
}

template<typename T>
Qt::ItemFlags orm::table_model<T>::flags(const QModelIndex & index) const
{
    if (!index.isValid()) {
        return Qt::ItemFlag::NoItemFlags;
    }

    if (0 > index.row() || 0 > index.column() || m_data.size() <= index.row() || m_headerData.size() <= index.column()) {
        return Qt::ItemFlag::NoItemFlags;
    }
    return m_defaultFlags[index.column()];
}

template<typename T>
QVariant orm::table_model<T>::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Qt::Orientation::Vertical == orientation) {
        return QVariant();
    }
    if (m_headerData.size() <= section || 0 > section) {
        return QVariant();
    }
    return m_headerData[section].value(role, QVariant());
}

template<typename T>
QModelIndex orm::table_model<T>::index(int row, int column, const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    }
    if (0 > row || 0 > column || m_data.size() <= row || m_headerData.size() <= column) {
        return QModelIndex();
    }
    return createIndex(row, column, m_data[row].data());
}

template<typename T>
bool orm::table_model<T>::insertRows(int row, int count, const QModelIndex & parent)
{
    auto ret = NewTraceC(c_orm_table())
                   .object(this)
                   .thread()
                   .template metaType<T>()
                   .function("virtual bool orm::table_model<T>::insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override")
                   .functionArgument("row", row)
                   .functionArgument("count", count)
                   .functionArgument("parent", parent)
                   .post()
                   .template ret<bool>();
    if (parent.isValid()) {
        return ret() = false;
    }
    if (0 > row || m_data.size() < row) {
        return ret() = false;
    }
    auto transaction = beginTransaction();
    if (orm::table_model_base::TransactionState::Fail == transaction) {

    }
    bool rrem = true;

    beginInsertRows(parent, row, row+count-1);
    for (int i = row; i < row+count; ++i) {
        orm::table_model<T>::Tsp tsp;
        tsp.reset(new T);
        if constexpr (std::is_base_of<QObject, T>()) {
            init_row(tsp.data());
        }
        m_data.insert(m_data.begin() + row, 1, tsp);
    }
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
    return ret() = rrem;
}

template<typename T>
bool orm::table_model<T>::moveRows(const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationChild)
{
    auto ret = NewTraceC(c_orm_table())
                   .object(this)
                   .thread()
                   .template metaType<T>()
                   .function("virtual bool orm::table_model<T>::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override")
                   .functionArgument("sourceParent", sourceParent)
                   .functionArgument("sourceRow", sourceRow)
                   .functionArgument("count", count)
                   .functionArgument("destinationParent", destinationParent)
                   .functionArgument("destinationChild", destinationChild)
                   .post()
                   .template ret<bool>();
    if (sourceParent.isValid()) {
        return ret() = false;
    }
    if (destinationParent.isValid()) {
        return ret() = false;
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
    return ret() = true;
}

template<typename T>
QModelIndex orm::table_model<T>::parent([[maybe_unused]] const QModelIndex & index) const
{
    return QModelIndex();
}

template<typename T>
bool orm::table_model<T>::removeRows(int row, int count, const QModelIndex & parent)
{
    auto ret = NewTraceC(c_orm_table())
                   .object(this)
                   .thread()
                   .template metaType<T>()
                   .function("virtual bool orm::table_model<T>::removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override")
                   .functionArgument("row", row)
                   .functionArgument("count", count)
                   .functionArgument("parent", parent)
                   .post()
                   .template ret<bool>();
    if (parent.isValid()) {
        return ret() = false;
    }
    if (0 > row || m_data.size() < row + count) {
        return ret() = false;
    }
    auto transaction = beginTransaction();
    if (orm::table_model_base::TransactionState::Fail == transaction) {

    }
    bool rrem = true;

    beginRemoveRows(parent, row, row+count-1);
    for (int i = row; i < row+count; ++i) {
        if constexpr (std::is_base_of<QObject, T>()) {
            purge_row(m_data[i].data());
        }
    }
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
    return ret() = rrem;
}

template<typename T>
int orm::table_model<T>::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_data.size();
}



// true = OK
// if [column] == -1, [t] is new row, all checks visit all columns
// if [column] >= 0, [t] is existing column, checks skip unrelated columns and use [value] instead of property value
template <typename T>
bool orm::table_model<T>::check_all(table_model<T>::Tp t, int column, QVariant const& value) const
{
    if (!check_custom_light(t, column, value)) {
        return false;
    }
    if (!check_not_null(t, column, value)) {
        return false;
    }
    if (!check_unique(t, column, value)) {
        return false;
    }
    //if (!check_foreign_key(t, column, value)) {
    //    return false;
    //}
    if (!check_custom_heavy(t, column, value)) {
        return false;
    }
    return true;
}

template <typename T>
bool orm::table_model<T>::check_not_null(table_model<T>::Tp t, int column, QVariant const& value) const
{
    if (column >= 0) {
        if (m_not_null_i.contains(column) && value.isNull()) {
            qCDebug(c_orm_table()) << "check_not_null failed: " << t << " c:" << column << "=" << value;
            return false;
        }
    } else {
        for (int i : m_not_null_i) {
            if (orm::_data(t, columnToProperty(i)).isNull()) {
                qCDebug(c_orm_table()) << "check_not_null failed: " << t << " c:" << i << "=" << orm::_data(t, columnToProperty(i)).isNull();
                return false;
            }
        }
    }
    return true;
}

template <typename T>
bool orm::table_model<T>::check_unique(table_model<T>::Tp t, int column, QVariant const& value) const
{
    QMap<int, QVariant> ts;
    for (auto i = m_unique_i.cbegin(); i != m_unique_i.cend(); ++i) {
        for (auto p = i->cbegin(); p != i->cend(); ++p) {
            if (!ts.contains(*p)) {
                if (*p == column) {
                    ts[*p] = value;
                } else {
                    ts[*p] = orm::_data(t, columnToProperty(*p));
                }
            }
        }
    }
    if (any([this, t, ts](Tsp const& tp)->bool {
            if (t == tp) {
                return false;
            }
            for (auto i = m_unique_i.cbegin(); i != m_unique_i.cend(); ++i) {
                bool eq = true;
                for (auto p = i->cbegin(); p != i->cend(); ++p) {
                    eq = true;
                    if (ts[*p] != orm::_data(tp, columnToProperty(*p))) {
                        eq = false;
                        break;
                    }
                }
                if (eq) {
                    qCDebug(c_orm_table()) << "check_unique failed:" << tp << " cond:" << m_unique[i-m_unique_i.cbegin()] << " values:" << ts;
                    return true;
                }
            }
            return false;
        })) {
        return false;
    }
    return true;
}

/* template <typename T>
bool orm::table_model<T>::check_foreign_key(table_model<T>::Tp t, int column, QVariant const& value) const
{
    if (m_foreign_key.size() != m_foreign_key_i.size()) {
        QSet<QString> to_connect;
        for (auto fk = m_foreign_key.cbegin(); fk != m_foreign_key.cend(); ++fk) {
            bool good = false;
            QList<int> refs;
            for (QByteArray const& ref : fk.key()) {
                int i = QMetaType(qMetaTypeId<T>()).metaObject()->indexOfProperty(ref);
                refs << i;
            }
            if (m_foreign_key_i.contains(refs)) {
                continue;
            }
            good = true;
            if (refs.size()) {
                QPair<QString, QList<int>> v;
                QPointer<table_model_base> foreign_model = manager()->model(fk->first);
                if (foreign_model) {
                    v.first = fk.value().first;
                    for (QByteArray const& fkey : fk->second) {
                        int idx = foreign_model->propertyNameToColumn(fkey);
                        if (idx < 0) {
                            NewCriticalC(c_orm_table())
                                .object(this)
                                .thread()
                                .template metaType<T>()
                                .description(QString("Foreign key error on field %1! Property %2 not found for type %3")
                                                 .arg(fk.key().join(','))
                                                 .arg(fkey)
                                                 .arg(fk.value().first))
                                .post();
                            good = false;
                            break;
                        }
                        v.second << idx;
                    }
                    if (good) {
                        m_foreign_key_i[refs] = v;
                        to_connect << fk->first;
                    }
                }
            }
            if (!good) {
                NewCriticalC(c_orm_table())
                    .object(this)
                    .thread()
                    .template metaType<T>()
                    .description(QString("Foreign key error on field %1! Property %2 not found for type %3")
                                     .arg(fk.key().join(','))
                                     .arg(fk.value().second.join(','))
                                     .arg(fk.value().first))
                    .post();
            }
        }
        for (auto i = to_connect.cbegin(); i != to_connect.cend(); ++i) {
            QSharedPointer<table_model_base> foreign_model = manager()->model(*i);
            if (foreign_model) {
                connect(foreign_model.data(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &table_model_base::onForeignRowsAboutToBeRemoved);
                connect(foreign_model.data(), &QAbstractItemModel::dataChanged, this, &table_model_base::onForeignDataChanged);
            }
        }
    }
    if (column >= 0 && !value.isNull()) {
        int prop = columnToProperty(column);
        for (auto fkey = m_foreign_key_i.cbegin(); fkey != m_foreign_key_i.cend(); ++fkey) {
            int findex = fkey.key().indexOf(prop);
            if (findex < 0) {
                continue;
            }
            QSharedPointer<table_model_base> foreign_model = manager()->model(fkey->first);
                if (!foreign_model) {
                    qCDebug(c_orm_table()) << "Foreign key failed:" << t << " table:" << fkey->first;
                    return false;
                }
                bool good = false;
                for (int j = 0; j < foreign_model->rowCount(); ++j) {
                    if (foreign_model->index(j, fkey->second[findex]).data() == value) {
                        good = true;
                    }
                }
                if (!good) {
                    qCDebug(c_orm_table()) << "Foreign key failed: " << t <<
                        " ref:" << fkey->first <<
                        "." << foreign_model->headerData(fkey->second[findex], Qt::Horizontal) <<
                        " c:" << column << "=" << value;
                    return false;
                }

        }
    } else {
        for (auto i = m_foreign_key_i.cbegin(); i != m_foreign_key_i.cend(); ++i) {
            QSharedPointer<table_model_base> foreign_model = manager()->model(i->first);
            if (!foreign_model) {
                qCDebug(c_orm_table()) << "Foreign key failed: " << t << " table:" << i->first;
                return false;
            }
            QVariantList vs;
            bool good = false;
            for (auto j = i.key().cbegin(); j != i.key().cend(); ++j) {
                QVariant v = orm::_data(t, *j);
                if (v.isNull()) {
                    good = true;
                    break;
                }
                vs << v;
            }
            if (good) {
                continue;
            }
            for (int j = 0; j < foreign_model->rowCount(); ++j) {
                good = true;
                for (int k = 0; k < i->second.size(); ++k) {
                    if (foreign_model->index(j, i->second[k]).data() != vs[k]) {
                        good = false;
                    }
                }
                if (good) {
                    break;
                }
            }
            if (!good) {
                qCDebug(c_orm_table()) << "Foreign key failed: no values " << vs <<
                    " ref:" << i->first <<
                    " c:" << i.key();
                return false;
            }
        }
    }
    return true;
}
 */
template <typename T>
bool orm::table_model<T>::check_custom_light([[maybe_unused]] table_model<T>::Tp t, [[maybe_unused]] int column, [[maybe_unused]] QVariant const& value) const
{
    return true;
}

template <typename T>
bool orm::table_model<T>::check_custom_heavy([[maybe_unused]] table_model<T>::Tp t, [[maybe_unused]] int column, [[maybe_unused]] QVariant const& value) const
{
    return true;
}

template<typename T>
int orm::table_model<T>::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_headerData.size();
}

template<typename T>
bool orm::table_model<T>::setData(const QModelIndex & index, const QVariant & value, int role)
{
    auto ret = NewTraceC(c_orm_table())
                   .object(this)
                   .thread()
                   .template metaType<T>()
                   .function("virtual bool orm::table_model<T>::setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override")
                   .functionArgument("index", index)
                   .functionArgument("value", value)
                   .functionArgument("role", role)
                   .post()
                   .template ret<bool>();
    if (!index.isValid()) {
        return ret() = false;
    }
    if (index.model() != this) {
        return ret() = false;
    }
    if (index.parent().isValid()) {
        return ret() = false;
    }

    auto transaction = beginTransaction();
    if (orm::table_model_base::TransactionState::Fail == transaction) {

    }
    bool sset = false;

    switch (role) {
        case Qt::ItemDataRole::DisplayRole:
        case Qt::ItemDataRole::EditRole:
            if (check_all(m_data[index.row()].data(), index.column(), value)) {
                sset = orm::_setdata(m_data[index.row()], columnToProperty(index.column()), value);
            }
            else {
                sset = false;
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
    return ret() = sset;
}

template<typename T>
bool orm::table_model<T>::setHeaderData(int section, Qt::Orientation orientation, const QVariant & value, int role)
{
    auto ret = NewTraceC(c_orm_table())
                   .object(this)
                   .thread()
                   .template metaType<T>()
                   .function("virtual bool orm::table_model<T>::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override")
                   .functionArgument("section", section)
                   .functionArgument("orientation", orientation)
                   .functionArgument("value", value)
                   .functionArgument("role", role)
                   .post()
                   .template ret<bool>();
    if (Qt::Orientation::Vertical == orientation) {
        return ret() = false;
    }
    if (m_headerData.size() <= section || 0 > section) {
        return ret() = false;
    }
    if (headerData(section, orientation, role) == value) {
        return ret() = false;
    }
    if (!value.isValid()) {
        m_headerData[section].remove(role);
    }
    else {
        m_headerData[section][role] = value;
    }
    emit headerDataChanged(orientation, section, section);
    return ret() = true;
}

#endif // ORM_TABLE_MODEL_HPP
