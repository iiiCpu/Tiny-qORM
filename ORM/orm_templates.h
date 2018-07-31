#ifndef ORM_TEMPLATES_H
#define ORM_TEMPLATES_H

#include "orm_def.h"
#include <QMetaProperty>

template <typename T>
QDebug orm_toDebugObj(QDebug dbg, T const& tt)
{
    T const* t = &tt;
    QDebugStateSaver saver(dbg);
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        dbg.nospace() << o->className() << "(";
        for (int i = 0; i < o->propertyCount(); ++i) {
            if (i) dbg.nospace() << ", ";
            QMetaProperty p = o->property(i);
            dbg.nospace() << p.name() << "=";
            orm_toDebugObj(dbg, p.read(t));
        }
        dbg.nospace() << ")";
    }
    else {
        dbg.nospace() << QMetaType::typeName(qMetaTypeId<T>()) << "(" << tt << ")";
    }
    return dbg;
}

template <typename T>
QString orm_toString(T const& tt)
{
    T const* t = &tt;
    QString str;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        str += o->className();
        str += "(";
        for (int i = 0; i < o->propertyCount(); ++i) {
            if (i) str += ", ";
            QMetaProperty p = o->property(i);
            str += p.name();
            str += "=";
            str += p.readOnGadget(t).toString();
        }
        str += ")";
    }
    else {
        str += QMetaType::typeName(qMetaTypeId<T>());
        str += "(" + QVariant::fromValue(tt).toString() + ")";
    }
    return str;
}
template <typename T>
QString orm_toStringP(T const* t)
{
    if (!t) {
        return "{null}";
    }
    QString str;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        str += o->className();
        str += "(";
        for (int i = 0; i < o->propertyCount(); ++i) {
            if (i) str += ", ";
            QMetaProperty p = o->property(i);
            str += p.name();
            str += "=";
            str += p.readOnGadget(t).toString();
        }
        str += ")";
    }
    else {
        str += QMetaType::typeName(qMetaTypeId<T>());
        str += "(" + QVariant::fromValue(t).toString() + ")";
    }
    return str;
}
template <typename T>
QDebug orm_toDebug(QDebug dbg, T const& tt)
{
    dbg.nospace() << orm_toString(tt);
    return dbg;
}

template <typename T>
bool orm_equalObj(T const& ll, T const& rr)
{
    T const* l = &ll;
    T const* r = &rr;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (int i = 0; i < o->propertyCount(); ++i) {
            QMetaProperty p = o->property(i);
            if (p.name() == QString("orm_rowid")) continue;
            if (p.read(l) != p.read(r)) {
                return false;
            }
        }
        return true;
    }
    else {
        return l == r;
    }
}
template <typename T>
bool orm_equal(T const& ll, T const& rr)
{
    T const* l = &ll;
    T const* r = &rr;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (int i = 0; i < o->propertyCount(); ++i) {
            QMetaProperty p = o->property(i);
            if (p.name() == QString("orm_rowid")) continue;
            if (p.readOnGadget(l) != p.readOnGadget(r)) {
                return false;
            }
        }
        return true;
    }
    else {
        return l == r;
    }
}

template <typename T>
bool orm_lessObj(T const& ll, T const& rr)
{
    T const* l = &ll;
    T const* r = &rr;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        bool ret = true;
        for (int i = 0; i < o->propertyCount(); ++i) {
            QMetaProperty p = o->property(i);
            if (p.name() == QString("orm_rowid")) continue;
            ret &= p.read(l) < p.read(r);
            if (!ret) {
                return false;
            }
        }
        return ret;
    }
    else {
        return ll < rr;
    }
}
template <typename T>
bool orm_less(T const& ll, T const& rr)
{
    T const* l = &ll;
    T const* r = &rr;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (int i = 0; i < o->propertyCount(); ++i) {
            QMetaProperty p = o->property(i);
            if (p.name() == QString("orm_rowid")) continue;
            if (p.readOnGadget(l) >= p.readOnGadget(r)) {
                return false;
            }
        }
        return true;
    }
    else {
        return ll < rr;
    }
}

template <typename T>
bool orm_nequalObj(T const& ll, T const& rr)
{
    T const* l = &ll;
    T const* r = &rr;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (int i = 0; i < o->propertyCount(); ++i) {
            QMetaProperty p = o->property(i);
            if (p.name() == QString("orm_rowid")) continue;
            if (p.read(l) != p.read(r)) {
                return true;
            }
        }
        return false;
    }
    else {
        return ll != rr;
    }
}
template <typename T>
bool orm_nequal(T const& ll, T const& rr)
{
    T const* l = &ll;
    T const* r = &rr;
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (int i = 0; i < o->propertyCount(); ++i) {
            QMetaProperty p = o->property(i);
            if (p.name() == QString("orm_rowid")) continue;
            if (p.readOnGadget(l) != p.readOnGadget(r)) {
                qDebug() << o->className() << p.name() << ll << rr;
                return true;
            }
        }
        return false;
    }
    else {
        return ll != rr;
    }
}

template <typename T>
T& orm_setObj(T & l, T const& r)
{
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (int i = 0; i < o->propertyCount(); ++i) {
            QMetaProperty p = o->property(i);
            p.write(l, p.read(r));
        }
    }
    else {
        l = r;
    }
    return l;
}
template <typename T>
T& orm_set(T & l, T const& r)
{
    const QMetaObject * o = QMetaType::metaObjectForType(qMetaTypeId<T>());
    if (o) {
        for (int i = 0; i < o->propertyCount(); ++i) {
            QMetaProperty p = o->property(i);
            p.writeOnGadget(l, p.readOnGadget(r));
        }
    }
    else {
        l = r;
    }
    return l;
}

#endif // ORM_TEMPLATES_H
