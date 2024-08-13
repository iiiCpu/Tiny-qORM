#ifndef ORM_QUERY_BUILDER_H
#define ORM_QUERY_BUILDER_H

#include "orm.h"

namespace orm
{

class query_builder
{
public:
    query_builder(){}
};

class clause_where
{
public:

};

class clause_expr_base
{
public:

};

class clause_constant : public clause_expr_base
{
public:
    class clause_constant_p {};
    QVariant value;
};

class clause_bind : public clause_expr_base
{
public:
    QString binding;
};

class clause_column : public clause_expr_base
{
public:
    QMetaProperty column;
};

class clause_list : public clause_expr_base
{
public:
    QList<clause_expr_base*> list;
};

class clause_unary_operator : public clause_expr_base
{
public:
    enum class op {

    };
    clause_expr_base* arg;
};

class clause_binary_operator : public clause_expr_base
{
public:
    enum class op {

    };
    clause_expr_base* arg1;
    clause_expr_base* arg2;
};

class clause_function_call : public clause_expr_base
{
public:
    enum class op {
        func

    };

    QString function_name;
    clause_list * args;
};

class clause_collate : public clause_expr_base
{
public:
    clause_expr_base arg;
    QString collate;
};

class clause_cast : public clause_expr_base
{
public:
    clause_expr_base arg;
    QString type;
};

class clause_raw : public clause_expr_base
{
public:
    QString query;
};

}

#endif // ORM_QUERY_BUILDER_H
