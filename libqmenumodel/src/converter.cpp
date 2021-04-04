/*
 * Copyright 2012-2016 Canonical Ltd.
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
 *      Marco Trevisan <marco.trevisan@canonical.com>
 */

#include <glib.h>

#include "converter.h"

#include <QDebug>
#include <QString>
#include <QVariant>

/*! \internal */
QVariant Converter::toQVariant(GVariant *value)
{
    QVariant result;
    if (value == NULL) {
        return result;
    }

    const GVariantType *type = g_variant_get_type(value);
    if (g_variant_type_equal(type, G_VARIANT_TYPE_BOOLEAN)) {
        result.setValue((bool)g_variant_get_boolean(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTE)) {
        result.setValue(g_variant_get_byte(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT16)) {
        result.setValue(qint16(g_variant_get_int16(value)));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT16)) {
        result.setValue(quint16(g_variant_get_uint16(value)));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT32)) {
        result.setValue(qint32(g_variant_get_int32(value)));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT32)) {
        result.setValue(quint32(g_variant_get_uint32(value)));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_INT64)) {
        result.setValue(qint64(g_variant_get_int64(value)));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_UINT64)) {
        result.setValue(quint64(g_variant_get_uint64(value)));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_DOUBLE)) {
        result.setValue(g_variant_get_double(value));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_STRING)) {
        gsize size = 0;
        const gchar *v = g_variant_get_string(value, &size);
        result.setValue(QString::fromUtf8(v, size));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_STRING_ARRAY)) {
        gsize size = 0;
        const gchar **sa = g_variant_get_strv(value, &size);
        QStringList list;
        for (gsize i = 0; i < size; ++i) {
            list << QString::fromUtf8(sa[i]);
        }
        result.setValue(list);
        g_free(sa);
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTESTRING)) {
        result.setValue(QByteArray(g_variant_get_bytestring(value)));
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTESTRING_ARRAY)) {
        gsize size = 0;
        const gchar **bsa = g_variant_get_bytestring_array(value, &size);
        QByteArrayList list;
        for (gsize i = 0; i < size; ++i) {
            list << bsa[i];
        }
        result.setValue(list);
        g_free(bsa);
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_VARIANT)) {
        GVariant *var = g_variant_get_variant(value);
        result = toQVariant(var);
        g_variant_unref(var);
    } else if (g_variant_type_equal(type, G_VARIANT_TYPE_VARDICT)) {
        GVariantIter iter;
        GVariant *vvalue;
        gchar *key;
        QVariantMap qmap;

        g_variant_iter_init (&iter, value);
        while (g_variant_iter_loop (&iter, "{sv}", &key, &vvalue))
        {
            qmap.insert(QString::fromUtf8(key), toQVariant(vvalue));
        }

        result.setValue(qmap);
    } else if (g_variant_type_is_array(type)) {
        QVariantList lst;
        for (int i = 0, iMax = g_variant_n_children(value); i < iMax; i++) {
            GVariant *child = g_variant_get_child_value(value, i);
            lst << toQVariant(child);
            g_variant_unref(child);
        }
        result.setValue(lst);
    } else if (g_variant_type_is_tuple(type)) {
        gsize size = g_variant_n_children(value);
        QVariantList vlist;

        for (gsize i=0; i < size; i++) {
            GVariant *v = g_variant_get_child_value(value, i);
            if (v) {
                vlist << toQVariant(v);
                g_variant_unref(v);
            }
        }

        result.setValue(vlist);
    } else {
        qWarning() << "Unsupported GVariant value" << (char*) type;
    }

    /* TODO: implement convertions to others types
     * G_VARIANT_TYPE_HANDLE
     * G_VARIANT_TYPE_OBJECT_PATH
     * G_VARIANT_TYPE_SIGNATURE
     * G_VARIANT_TYPE_ANY
     * G_VARIANT_TYPE_BASIC
     * G_VARIANT_TYPE_MAYBE
     * G_VARIANT_TYPE_UNIT
     * G_VARIANT_TYPE_DICT_ENTRY
     * G_VARIANT_TYPE_DICTIONARY
     * G_VARIANT_TYPE_OBJECT_PATH_ARRAY
     */

    return result;
}

QVariant Converter::toQVariantFromVariantString(const QString &variantString)
{
    GVariant *gvariant;
    GError *error = NULL;

    if (variantString.isEmpty()) {
        return QVariant();
    }

    gvariant = g_variant_parse (NULL, qUtf8Printable(variantString), NULL, NULL, &error);

    if (error) {
        qWarning() << "Impossible to parse" << variantString << "as variant string:"<< error->message;
        g_error_free (error);
        return QVariant();
    }

    const QVariant& qvariant = Converter::toQVariant(gvariant);
    g_variant_unref (gvariant);

    return qvariant;
}

GVariant* Converter::toGVariant(const QVariant &value)
{
    GVariant *result = NULL;
    if (value.isNull() || !value.isValid())
        return result;

    switch(value.type()) {
    case QVariant::Bool:
        result = g_variant_new_boolean(value.toBool());
        break;
    case QVariant::ByteArray:
        result = g_variant_new_bytestring(value.toByteArray());
        break;
    case QVariant::Double:
        result = g_variant_new_double(value.toDouble());
        break;
    case QVariant::Int:
        result = g_variant_new_int32(value.toInt());
        break;
    case QVariant::LongLong:
        result = g_variant_new_int64(value.toLongLong());
        break;
    case QVariant::String:
        result = g_variant_new_string(qUtf8Printable(value.toString()));
        break;
    case QVariant::UInt:
        result = g_variant_new_uint32(value.toUInt());
        break;
    case QVariant::ULongLong:
        result = g_variant_new_uint64(value.toULongLong());
        break;
    case QMetaType::UChar:
        result = g_variant_new_byte(value.value<uchar>());
        break;
    case QMetaType::Short:
        result = g_variant_new_int16(value.value<short>());
        break;
    case QMetaType::UShort:
        result = g_variant_new_uint16(value.value<ushort>());
        break;
    case QMetaType::Long:
        result = g_variant_new_int64(value.value<long>());
        break;
    case QMetaType::ULong:
        result = g_variant_new_uint64(value.value<ulong>());
        break;
    case QVariant::Map:
    {
        GVariantBuilder *b;
        GVariant *dict;

        b = g_variant_builder_new(G_VARIANT_TYPE_VARDICT);
        QMapIterator<QString, QVariant> i(value.toMap());
        while (i.hasNext()) {
            i.next();
            g_variant_builder_add(b, "{sv}", qUtf8Printable(i.key()), toGVariant(i.value()));
        }
        result = g_variant_builder_end(b);
        g_variant_builder_unref(b);
        break;
    }
    case QMetaType::QByteArrayList:
    {
        const QByteArrayList &list = qvariant_cast<QByteArrayList>(value);
        GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE_BYTESTRING_ARRAY);

        for (const QByteArray &ba : list) {
            g_variant_builder_add_value(b, g_variant_new_bytestring(ba));
        }
        result = g_variant_builder_end(b);
        g_variant_builder_unref(b);
        break;
    }
    case QVariant::List:
    {
        GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE_TUPLE);

        for (const QVariant &v : value.toList()) {
            g_variant_builder_add_value(b, toGVariant(v));
        }
        result = g_variant_builder_end(b);
        g_variant_builder_unref(b);
        break;
    }
    case QVariant::StringList:
    {
        GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE_STRING_ARRAY);

        for (const QString &s : value.toStringList()) {
            g_variant_builder_add(b, "s", qUtf8Printable(s));
        }
        result = g_variant_builder_end(b);
        g_variant_builder_unref(b);
        break;
    }
    default:
        qWarning() << "QVariant type not supported:" << value.type();
    }

    return result;
}

GVariant* Converter::toGVariantWithSchema(const QVariant &value, const char* schema)
{
    if (!g_variant_type_string_is_valid(schema)) {
        return Converter::toGVariant(value);
    }

    GVariant* result = NULL;
    GVariantType* schema_type = g_variant_type_new(schema);

    if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_BOOLEAN)) {
        if (value.canConvert<bool>()) {
            result = g_variant_new_boolean (value.value<bool>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_BYTE)) {
        if (value.canConvert<uchar>()) {
            result = g_variant_new_byte (value.value<uchar>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_INT16)) {
        if (value.canConvert<qint16>()) {
            result = g_variant_new_int16 (value.value<qint16>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_UINT16)) {
        if (value.canConvert<quint16>()) {
            result = g_variant_new_uint16 (value.value<quint16>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_INT32)) {
        if (value.canConvert<qint32>()) {
            result = g_variant_new_int32 (value.value<qint32>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_UINT32)) {
        if (value.canConvert<quint32>()) {
            result = g_variant_new_uint32 (value.value<quint32>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_INT64)) {
        if (value.canConvert<qint64>()) {
            result = g_variant_new_int64 (value.value<qint64>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_UINT64)) {
        if (value.canConvert<quint64>()) {
            result = g_variant_new_uint64 (value.value<quint64>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_DOUBLE)) {
        if (value.canConvert<double>()) {
            result = g_variant_new_double (value.value<double>());
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_STRING)) {
        if (value.canConvert<QString>()) {
            result = g_variant_new_string(qUtf8Printable(value.toString()));
        }
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_VARIANT)) {
        result = g_variant_new_variant(Converter::toGVariant(value));
    } else if (g_variant_type_equal(schema_type, G_VARIANT_TYPE_VARDICT)) {
        if (value.canConvert(QVariant::Map)) {
            result = Converter::toGVariant(value.toMap());
        }
    } else if (g_variant_type_is_array(schema_type)) {
        if (value.canConvert(QVariant::List)) {
            const GVariantType* entryType = g_variant_type_element(schema_type);
            const gchar* entryTypeString = g_variant_type_peek_string(entryType);

            GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);
            bool ok = true;

            for (const QVariant &v : value.toList()) {
                GVariant *data = toGVariantWithSchema(v, entryTypeString);

                if (data) {
                    g_variant_builder_add_value(b, data);
                } else {
                    ok = false;
                    qWarning() << "Failed to convert list to array with schema:" << schema;
                    break;
                }
            }
            if (ok) {
                result = g_variant_builder_end(b);
            }
            g_variant_builder_unref(b);
        }
    } else if (g_variant_type_is_tuple(schema_type)) {
        if (value.canConvert(QVariant::List)) {
            const GVariantType* entryType = g_variant_type_first(schema_type);

            GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE_TUPLE);
            bool ok = true;

            for (const QVariant &v : value.toList()) {
                gchar* entryTypeString = g_variant_type_dup_string(entryType);
                GVariant *data = toGVariantWithSchema(v, entryTypeString);
                g_free(entryTypeString);

                if (data) {
                    g_variant_builder_add_value(b, data);
                    entryType = g_variant_type_next(entryType);
                    if (!entryType)
                        break;
                } else {
                    ok = false;
                    qWarning() << "Failed to convert list to array with schema:" << schema;
                    break;
                }
            }
            if (ok) {
                result = g_variant_builder_end(b);
            }
            g_variant_builder_unref(b);
        }
    }

    // fallback to straight convert.
    if (!result) {
        result = Converter::toGVariant(value);
    }

    g_free(schema_type);

    return result;
}

