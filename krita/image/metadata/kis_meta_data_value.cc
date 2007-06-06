/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_value.h"

#include <QVariant>

#include <kdebug.h>

using namespace KisMetaData;

struct Value::Private {
    Private() : propertyQualifier(0) {}
    union {
        QVariant* variant;
        QList<Value>* array;
        QMap<QString, Value>* structure;
    } value;
    ValueType type;
    Value* propertyQualifier;
};

Value::Value() : d(new Private)
{
    d->type = Invalid;
}


Value::Value(const QVariant& variant) : d(new Private)
{
    d->type = Value::Variant;
    d->value.variant = new QVariant(variant);
}

Value::Value(const QList<Value>& array, ValueType type) : d(new Private)
{
    Q_ASSERT(type == OrderedArray or type == UnorderedArray or type == AlternativeArray or type == LangArray);
    d->value.array = new QList<Value>( array );
    d->type = type; // TODO: I am hesitating about LangArray to keep them as array or convert them to maps
}

Value::Value(const QMap<QString, Value>& structure) : d(new Private)
{
    d->type = Structure;
    d->value.structure = new QMap<QString, Value>( structure );
}

Value::Value(const Value& v) : d(new Private)
{
    *this = v;
}

Value& Value::operator=(const Value& v)
{
    d->type = v.d->type;
    switch(d->type)
    {
        case Invalid:
            break;
        case Variant:
            d->value.variant = new QVariant(*v.d->value.variant);
            break;
        case OrderedArray:
        case UnorderedArray:
        case AlternativeArray:
        case LangArray:
            d->value.array = new QList<Value>(*v.d->value.array);
            break;
        case Structure:
            d->value.structure = new QMap<QString, Value>(*v.d->value.structure);
            break;
    }
    delete d->propertyQualifier;
    if(v.d->propertyQualifier)
    {
        d->propertyQualifier = new Value();
        *d->propertyQualifier = *v.d->propertyQualifier;
    } else {
        d->propertyQualifier = 0;
    }
    return *this;
}


Value::~Value()
{
    delete d;
}

Value::ValueType Value::type() const
{
    return d->type;
}

QVariant Value::asVariant() const
{
    if(d->type == Variant)
    {
        return *d->value.variant;
    }
    return QVariant();
}

void Value::setVariant(const QVariant& variant)
{
    if(d->type == Variant and d->value.variant->type() == variant.type())
    {
        *d->value.variant = variant;
    }
}


QDebug operator<<(QDebug dbg, const Value &v)
{
    switch(v.type())
    {
        case Value::Invalid:
            dbg.nospace() << "invalid value";
            break;
        case Value::Variant:
            dbg.nospace() << v.asVariant();
            break;
        case Value::OrderedArray:
        case Value::UnorderedArray:
        case Value::AlternativeArray:
        case Value::LangArray:
//             d->value.array = new QList<Value>(*v.d->value.array);
            break;
        case Value::Structure:
//             d->value.structure = new QMap<QString, Value>(*v.d->value.structure);
            break;
    }
    return dbg.space();
}
