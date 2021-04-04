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

#include <QObject>
#include <QtTest>
#include <QDebug>

class QGVariantType
{
public:
    QGVariantType() : type(NULL) {}
    QGVariantType(const GVariantType *gvtype) : type(gvtype) {}
    const GVariantType *getType() const { return type; }
    operator const GVariantType*() const { return type; }

private:
    const GVariantType *type;
};
Q_DECLARE_METATYPE(QGVariantType);

class QGVariant
{
public:
    QGVariant() : variant(NULL) {}
    ~QGVariant() { if (variant) g_variant_unref(variant); }
    QGVariant(GVariant *gv) : variant(g_variant_ref_sink(gv)) {}
    QGVariant(const QGVariant &other) : QGVariant(other.variant) {}
    GVariant *gvariant() const { return variant; }
    operator GVariant*() const { return variant; }

private:
    GVariant *variant;
};
Q_DECLARE_METATYPE(QGVariant);

class ConverterTest : public QObject
{
    Q_OBJECT

private:
    bool compare(const QVariant &qv, const GVariantType *type)
    {
        bool result;
        GVariant *gv = Converter::toGVariant(qv);
        result = g_variant_type_equal(g_variant_get_type(gv), type);
        if (!result) {
            qWarning() << "types are different: QVariant:" << qv.typeName()
                       << "Result:" << g_variant_type_peek_string(g_variant_get_type(gv))
                       << "Expected:"<< g_variant_type_peek_string(type);
        }
        g_variant_unref(gv);
        return result;
    }
    bool compare(GVariant *gv, const QVariant::Type type)
    {
        g_variant_ref_sink(gv);
        const QVariant& qv = Converter::toQVariant(gv);
        bool result = (qv.type() == type);
        if (!result) {
            qWarning() << "types are different: GVariant:" << g_variant_type_peek_string(g_variant_get_type(gv))
                       << "Result:" << qv.type()
                       << "Expected:"<< type;
        }
        g_variant_unref(gv);
        return result;
    }
    bool compareWithSchema(const QVariant &qv, const QString strType)
    {
        GVariantType* expected_type;
        expected_type = g_variant_type_new(strType.toUtf8().data());

        bool result;
        GVariant *gv = Converter::toGVariantWithSchema(qv, strType.toUtf8().data());
        result = g_variant_type_equal(g_variant_get_type(gv), expected_type);
        if (!result) {
            qWarning() << "types are different: QVariant:" << qv.typeName()
                       << "Result:" << g_variant_type_peek_string(g_variant_get_type(gv))
                       << "Expected:"<< g_variant_type_peek_string(expected_type);
        }
        g_variant_unref(gv);
        g_variant_type_free(expected_type);
        return result;
    }

private Q_SLOTS:

    /*
     * Test converter QVariant to GVariant
     */

    void testConvertToGVariant_data()
    {
        QTest::addColumn<QVariant>("value");
        QTest::addColumn<QGVariantType>("expectedType");

        QTest::newRow("Boolean") << QVariant(true) << QGVariantType(G_VARIANT_TYPE_BOOLEAN);
        QTest::newRow("Byte") << QVariant::fromValue<uchar>(42) << QGVariantType(G_VARIANT_TYPE_BYTE);
        QTest::newRow("Int16") << QVariant::fromValue<short>(-42) << QGVariantType(G_VARIANT_TYPE_INT16);
        QTest::newRow("UInt16") << QVariant::fromValue<ushort>(-42) << QGVariantType(G_VARIANT_TYPE_UINT16);
        QTest::newRow("Int32") << QVariant(-42) << QGVariantType(G_VARIANT_TYPE_INT32);
        QTest::newRow("UInt32") << QVariant((uint)42) << QGVariantType(G_VARIANT_TYPE_UINT32);
        QTest::newRow("Int64") << QVariant::fromValue<long>(-42) << QGVariantType(G_VARIANT_TYPE_INT64);
        QTest::newRow("UInt64") << QVariant::fromValue<ulong>(42) << QGVariantType(G_VARIANT_TYPE_UINT64);
        QTest::newRow("Int64") << QVariant::fromValue<qlonglong>(-42) << QGVariantType(G_VARIANT_TYPE_INT64);
        QTest::newRow("UInt64") << QVariant::fromValue<qulonglong>(42) << QGVariantType(G_VARIANT_TYPE_UINT64);
        QTest::newRow("Double") << QVariant((double)42.42) << QGVariantType(G_VARIANT_TYPE_DOUBLE);
        QTest::newRow("String") << QVariant(QString("42")) << QGVariantType(G_VARIANT_TYPE_STRING);
        QTest::newRow("String List") << QVariant(QStringList({"42", "42"})) << QGVariantType(G_VARIANT_TYPE_STRING_ARRAY);
        QTest::newRow("ByteArray") << QVariant(QByteArray("42")) << QGVariantType(G_VARIANT_TYPE_BYTESTRING);
        QTest::newRow("Map") << QVariant(QVariantMap()) << QGVariantType(G_VARIANT_TYPE_VARDICT);
        QTest::newRow("Map Filled") << QVariant(QVariantMap({{"fooBar", 0xdeadbeef}})) << QGVariantType(G_VARIANT_TYPE_VARDICT);
        QTest::newRow("List") << QVariant(QVariantList()) << QGVariantType(G_VARIANT_TYPE_UNIT);
        QTest::newRow("List Filled") << QVariant(QVariantList({"fooBar", 0xdeadbeef})) << QGVariantType(G_VARIANT_TYPE("(su)"));

        QVariant result;
        result.setValue(QByteArrayList({"42", "53"}));
        QTest::newRow("ByteArrayList") << result << QGVariantType(G_VARIANT_TYPE_BYTESTRING_ARRAY);
    }

    void testConvertToGVariant()
    {
        QFETCH(QVariant, value);
        QFETCH(QGVariantType, expectedType);

        QVERIFY(compare(value, expectedType));
    }

    void testConvertToGVariantAndBack_data()
    {
        testConvertToGVariant_data();
    }

    void testConvertToGVariantAndBack()
    {
        QFETCH(QVariant, value);

        GVariant *gv = Converter::toGVariant(value);
        QVERIFY(gv != NULL);

        QCOMPARE(Converter::toQVariant(gv), value);

        g_variant_unref(gv);
    }

    void testTupleConversion()
    {
        QVariantList qTuple;
        qTuple << 1 << "2" << 3.3;

        GVariant *gTuple = Converter::toGVariant(qTuple);
        QVERIFY(g_variant_type_is_tuple(g_variant_get_type(gTuple)));
        QCOMPARE(g_variant_n_children(gTuple), (gsize)3);

        GVariant *v = g_variant_get_child_value(gTuple, 0);
        int v0 = g_variant_get_int32(v);
        QCOMPARE(v0, 1);
        g_variant_unref(v);

        v = g_variant_get_child_value(gTuple, 1);
        const gchar *v1 = g_variant_get_string(v, NULL);
        QCOMPARE(QString(v1), QString("2"));
        g_variant_unref(v);

        v = g_variant_get_child_value(gTuple, 2);
        gdouble v2 = g_variant_get_double(v);
        QCOMPARE(v2, 3.3);
        g_variant_unref(v);

        g_variant_unref(gTuple);
    }

    void testConvertToGVariantWithSchema_data()
    {
        QTest::addColumn<QVariant>("value");
        QTest::addColumn<QString>("schema");

        // convert to byte
        QTest::newRow("byte") << QVariant::fromValue<int>(1) << "y";

        // convert to integer
        QTest::newRow("integer") << QVariant::fromValue<int>(1) << "i";
        QTest::newRow("integer from double") << QVariant::fromValue<double>(1.1) << "i";
        QTest::newRow("int16") << QVariant::fromValue<int>(-1) << "n";
        QTest::newRow("uint16") << QVariant::fromValue<int>(1) << "q";
        QTest::newRow("uint32") << QVariant::fromValue<int>(1) << "u";
        QTest::newRow("int64") << QVariant::fromValue<int>(1) << "x";
        QTest::newRow("uint64") << QVariant::fromValue<int>(1) << "t";

        // convert to variant
        QTest::newRow("variant from int") << QVariant::fromValue<int>(1) << "v";
        QTest::newRow("variant from double") << QVariant::fromValue<double>(1.1) << "v";
        QTest::newRow("variant from string") << QVariant::fromValue<QString>("string") << "v";

        // convert to bool
        QTest::newRow("bool") << QVariant::fromValue<bool>(true) << "b";
        QTest::newRow("bool from int") << QVariant::fromValue<int>(1) << "b";

        // convert to double
        QTest::newRow("double") << QVariant::fromValue<double>(1.0) << "d";
        QTest::newRow("double from int") << QVariant::fromValue<int>(1) << "d";

        // convert to string
        QTest::newRow("string") << QVariant::fromValue<QString>("FoooBar") << "s";
        QTest::newRow("string from int") << QVariant::fromValue<int>(1) << "s";
        QTest::newRow("string from double") << QVariant::fromValue<double>(1.1) << "s";

        // convert to tuple
        QVariantList list = QVariantList() << QVariant::fromValue<bool>(true) << QVariant::fromValue<int>(1) << QVariant::fromValue<int>(1) << QVariant::fromValue<QString>("test1");
        QTest::newRow("tuple") << QVariant(list) << "(bdis)";

        // convert to array
        list = QVariantList() << QVariant::fromValue<int>(1) << QVariant::fromValue<int>(1);
        QTest::newRow("int list") << QVariant(list) << "ad";
        list = QVariantList() << QVariant::fromValue<QString>("test1") << QVariant::fromValue<QString>("test2");
        QTest::newRow("string list") << QVariant(list) << "as";

        // convert to array of tuple
        QVariantList si1(QVariantList() << QVariant::fromValue<QString>("test1") << QVariant::fromValue<int>(1));
        QVariantList si2(QVariantList() << QVariant::fromValue<QString>("test1") << QVariant::fromValue<int>(1));
        list = QVariantList() << QVariant::fromValue(si1) << QVariant::fromValue(si2);
        QTest::newRow("array of tuple") << QVariant(list) << "a(sd)";

        // convert to vardict
        QVariantMap map;
        map["test1"] = QVariant::fromValue<int>(1);
        map["test2"] = QVariant::fromValue<double>(1);
        QTest::newRow("map") << QVariant(map) << "a{sv}";
    }

    void testConvertToGVariantWithSchema()
    {
        QFETCH(QVariant, value);
        QFETCH(QString, schema);

        QVERIFY(compareWithSchema(value, schema));
    }

    /*
     * Test converter GVariant to QVariant
     */

    void testConvertToQVariant_data()
    {
        QTest::addColumn<QGVariant>("value");
        QTest::addColumn<unsigned>("expectedType");

        QTest::newRow("Boolean") << QGVariant(g_variant_new_boolean(TRUE)) << (unsigned) QVariant::Bool;
        QTest::newRow("Byte") << QGVariant(g_variant_new_byte(53)) << (unsigned) QMetaType::UChar;
        QTest::newRow("Int16") << QGVariant(g_variant_new_int16(-53)) << (unsigned) QMetaType::Short;
        QTest::newRow("UInt16") << QGVariant(g_variant_new_uint16(53)) << (unsigned) QMetaType::UShort;
        QTest::newRow("Int32") << QGVariant(g_variant_new_int32(-53)) << (unsigned) QVariant::Int;
        QTest::newRow("UInt32") << QGVariant(g_variant_new_uint32(53)) << (unsigned) QVariant::UInt;
        QTest::newRow("Int64") << QGVariant(g_variant_new_int64(-53)) << (unsigned) QVariant::LongLong;
        QTest::newRow("UInt64") << QGVariant(g_variant_new_uint64(53)) << (unsigned) QVariant::ULongLong;
        QTest::newRow("Double") << QGVariant(g_variant_new_double(53.3)) << (unsigned) QVariant::Double;
        QTest::newRow("String") << QGVariant(g_variant_new_string("53")) << (unsigned) QVariant::String;
        QTest::newRow("Byte string") << QGVariant(g_variant_new_bytestring("53")) << (unsigned) QVariant::ByteArray;
        QTest::newRow("Tuple") << QGVariant(g_variant_new("(si)", "foo", 53)) << (unsigned) QVariant::List;

        GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add(builder, "{sv}", "fooo", g_variant_new_int64(53));
        QTest::newRow("Map") << QGVariant(g_variant_builder_end(builder)) << (unsigned) QVariant::Map;
        g_variant_builder_unref(builder);

        builder = g_variant_builder_new(G_VARIANT_TYPE("ai"));
        g_variant_builder_add(builder, "i", g_variant_new_int32(53));
        QTest::newRow("List") << QGVariant(g_variant_new("ai", builder)) << (unsigned) QVariant::List;
        g_variant_builder_unref(builder);

        QTest::newRow("Tuple") << QGVariant(g_variant_new("(i)", 53)) << (unsigned) QVariant::List;

        const gchar *byteArray[] = {"42", "53", NULL};
        QTest::newRow("ByteArrayList") << QGVariant(g_variant_new_bytestring_array(byteArray, -1)) << (unsigned) QMetaType::QByteArrayList;
        QTest::newRow("String List") << QGVariant(g_variant_new_strv(byteArray, -1)) << (unsigned) QVariant::StringList;
    }

    void testConvertToQVariant()
    {
        QFETCH(QGVariant, value);
        QFETCH(unsigned, expectedType);

        QVERIFY(compare(value, (QVariant::Type) expectedType));
    }

    void testConvertToQVariantAndBack_data()
    {
        testConvertToQVariant_data();
    }

    void testConvertToQVariantAndBack()
    {
        QFETCH(QGVariant, value);

        QVariant qv = Converter::toQVariant(value);
        QVERIFY(qv.isValid());

        GVariant *gv = Converter::toGVariant(qv);
        gboolean equals = g_variant_equal(value, gv);

        if (!equals && qv.type() == QVariant::List) {
            QVERIFY(g_variant_type_is_array(g_variant_get_type(value)));
            QVERIFY(g_variant_type_is_tuple(g_variant_get_type(gv)));

            gsize vsize = g_variant_n_children(value);
            QCOMPARE(vsize, g_variant_n_children(gv));

            for (gsize i = 0; i < vsize; ++i) {
                equals = g_variant_equal(g_variant_get_child_value(value, i), g_variant_get_child_value(gv, i));
                if (!equals)
                    break;
            }
        }

        if (!equals) {
            gchar *vs = g_variant_print(value, TRUE);
            gchar *gvs = g_variant_print(gv, TRUE);
            qWarning() << "Values do not match. Old" << vs << "converted" << gvs;
            g_free(vs);
            g_free(gvs);
        }

        g_variant_unref(gv);
        QVERIFY(equals != FALSE);
    }

    void testConvertToQVariantFromString_data()
    {
        QTest::addColumn<QString>("value");
        QTest::addColumn<unsigned>("expectedType");

        QTest::newRow("Boolean") << "true" << (unsigned) QVariant::Bool;
        QTest::newRow("Byte") << "byte 0xFF" << (unsigned) QMetaType::UChar;
        QTest::newRow("Int16") << "int16 65" << (unsigned) QMetaType::Short;
        QTest::newRow("UInt16") << "uint16 65" << (unsigned) QMetaType::UShort;
        QTest::newRow("Int32") << "int32 65" << (unsigned) QVariant::Int;
        QTest::newRow("UInt32") << "uint32 65" << (unsigned) QVariant::UInt;
        QTest::newRow("Int64") << "int64 65" << (unsigned) QVariant::LongLong;
        QTest::newRow("UInt64") << "uint64 65" << (unsigned) QVariant::ULongLong;
        QTest::newRow("Double") << "double 65" << (unsigned) QVariant::Double;
        QTest::newRow("String") << "string '65'" << (unsigned) QVariant::String;
        QTest::newRow("String simple") << "\"65\"" << (unsigned) QVariant::String;
        QTest::newRow("String List") << "['foo', 'bar']" << (unsigned) QVariant::StringList;
        QTest::newRow("Byte string") << "b'fooo'" << (unsigned) QVariant::ByteArray;
        QTest::newRow("Map") << "{'foo': <65>}" << (unsigned) QVariant::Map;
        QTest::newRow("List") << "[65, 66]" << (unsigned) QVariant::List;
        QTest::newRow("Tuple") << "('foo', 65)" << (unsigned) QVariant::List;
        QTest::newRow("ByteArrayList") << "[b'foo', b'bar']" << (unsigned) QMetaType::QByteArrayList;
    }

    void testConvertToQVariantFromString()
    {
        QFETCH(QString, value);
        QFETCH(unsigned, expectedType);

        QCOMPARE(Converter::toQVariantFromVariantString(value).type(), (QVariant::Type) expectedType);
    }

};

QTEST_MAIN(ConverterTest)

#include "convertertest.moc"
