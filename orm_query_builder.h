#ifndef ORM_QUERY_BUILDER_H
#define ORM_QUERY_BUILDER_H

#include "orm_def.h"
#include <QVariant>
#include <QMetaProperty>

namespace orm
{

class ORM_SHARED_EXPORT query_builder
{
public:
    query_builder(){}
};

class ORM_SHARED_EXPORT clause_where
{
public:

};

class ORM_SHARED_EXPORT clause_expr_base
{
public:

};

class ORM_SHARED_EXPORT clause_constant : public clause_expr_base
{
public:
    class clause_constant_p {};
    QVariant value;
};

class ORM_SHARED_EXPORT clause_bind : public clause_expr_base
{
public:
    QString binding;
};

class ORM_SHARED_EXPORT clause_column : public clause_expr_base
{
public:
    QMetaProperty column;
};

class ORM_SHARED_EXPORT clause_list : public clause_expr_base
{
public:
    QList<clause_expr_base*> list;
};

class ORM_SHARED_EXPORT clause_unary_operator : public clause_expr_base
{
public:
    enum class op {

    };
    clause_expr_base* arg;
};

class ORM_SHARED_EXPORT clause_binary_operator : public clause_expr_base
{
public:
    enum class op {

    };
    clause_expr_base* arg1;
    clause_expr_base* arg2;
};

class ORM_SHARED_EXPORT clause_function_call : public clause_expr_base
{
public:
    enum class op {
        func

    };

    QString function_name;
    clause_list * args;
};

class ORM_SHARED_EXPORT clause_collate : public clause_expr_base
{
public:
    clause_expr_base arg;
    QString collate;
};

class ORM_SHARED_EXPORT clause_cast : public clause_expr_base
{
public:
    clause_expr_base arg;
    QString type;
};

class ORM_SHARED_EXPORT clause_raw : public clause_expr_base
{
public:
    QString query;
};

}

#endif // ORM_QUERY_BUILDER_H
