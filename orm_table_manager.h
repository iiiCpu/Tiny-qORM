#ifndef ORM_TABLE_MANAGER_H
#define ORM_TABLE_MANAGER_H


#include <QAbstractTableModel>
#include <QObject>
#include <QMetaProperty>
#include <QLoggingCategory>
#include <QRegularExpression>
#include "orm_def.h"

#include "orm_table_model.h"

#ifdef LOGGER_HELPER


#endif

#include <QtCommonTools/errorhelper.h>
Q_DECLARE_EXPORTED_LOGGING_CATEGORY(c_orm_table_manager, ORM_SHARED_EXPORT)


namespace orm
{
class ORM_SHARED_EXPORT table_manager : public QObject
{
    Q_OBJECT
protected:
    QMap<QString, QSharedPointer<QAbstractItemModel>> m_tables;
public:
    explicit table_manager(QObject* parent = nullptr);
    virtual ~table_manager();

    template <typename T>
    QSharedPointer<table_model<T>> model(QString name = QString())
    {
        auto ret = NewTraceC(c_orm_table_manager())
                       .object(this)
                       .thread()
                       .template metaType<T>()
                       .function("template <typename T> QSharedPointer<table_model<T>> const& orm::table_manager::manager(QString const& name)")
                       .functionArgument("name", name)
                       .post();
        if (name.isEmpty()) {
            name = QMetaType(qMetaTypeId<T>()).name();
        }
        if (!m_tables.contains(name)){
            QSharedPointer<QAbstractItemModel> model;
            model.reset(new table_model<T>(this));
            model->setObjectName(name);
            m_tables[name] = model;
        }
        //model.reset(m_tables[name].dynamicCast<table_model<T>>());
        return qSharedPointerCast<table_model<T>>(m_tables[name]); // QSharedPointer<table_model<T>>(m_tables[name].objectCast<table_model<T>>());
    }

    QSharedPointer<table_model_base> model(QString name);
    QSharedPointer<QAbstractItemModel> anymodel(QString name);
    bool addManager(QAbstractItemModel * model);
};
}

#endif // ORM_TABLE_MANAGER_H
