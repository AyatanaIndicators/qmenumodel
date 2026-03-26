/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Renato Araujo Oliveira Filho <renato@canonical.com>
 */

#ifndef CONVERTER_H
#define CONVERTER_H

#include <QString>
#include <QVariant>

typedef struct _GVariant GVariant;

class Converter
{
public:
    static QVariant toQVariant(GVariant *value);
    static QVariant toQVariantFromVariantString(const QString &variantString);
    static GVariant* toGVariant(const QVariant &value);

    // This converts a QVariant to a GVariant using a provided gvariant schema as
    // a conversion base (it will attempt to convert to this format).
    static GVariant* toGVariantWithSchema(const QVariant &value, const char* schema);
};

#endif // CONVERTER_H

