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
 *      Olivier Tilloy <olivier.tilloy@canonical.com>
 */

#include "qmenumodel.h"

#include <gio/gio.h>

#include <QtTest>


class TestModel : public QMenuModel
{
    Q_OBJECT
public:
    TestModel() : QMenuModel(0)
    {
        GMenu *menu5 = g_menu_new();
        g_menu_append(menu5, "menu6", NULL);
        g_menu_append(menu5, "menu7", NULL);

        GMenu *menu3 = g_menu_new();
        g_menu_append(menu3, "menu4", NULL);
        g_menu_append_section(menu3, "menu5", G_MENU_MODEL(menu5));
        g_menu_append(menu3, "menu8", NULL);

        GMenu *menu = g_menu_new();
        g_menu_append(menu, "menu0", NULL);
        g_menu_append(menu, "menu1", NULL);
        g_menu_append(menu, "menu2", NULL);
        g_menu_append_section(menu, "menu3", G_MENU_MODEL(menu3));

        setMenuModel(G_MENU_MODEL(menu));

        m_menus << menu << menu3 << menu5;
    }

private:
    QList<GMenu*> m_menus;
};

class TreeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMenuBuild()
    {
        TestModel menu;
        QCOMPARE(menu.rowCount(), 4);
        QModelIndex row0 = menu.index(0);
        QVERIFY(row0.isValid());
        QCOMPARE(menu.rowCount(row0), 0);

        QModelIndex row3 = menu.index(3);
        QVERIFY(row3.isValid());
        QCOMPARE(menu.rowCount(row3), 3);
        QCOMPARE(menu.data(row3, QMenuModel::Label).toString(), QString("menu3"));

        QModelIndex row4 = row3.model()->index(0, 0, row3);
        QVERIFY(row4.isValid());
        QCOMPARE(menu.rowCount(row4), 0);
        QCOMPARE(menu.data(row4, QMenuModel::Depth).toInt(), 1);
        QCOMPARE(menu.data(row4, QMenuModel::Label).toString(), QString("menu4"));

        QModelIndex row5 = row3.model()->index(1, 0, row3);
        QVERIFY(row5.isValid());
        QCOMPARE(menu.rowCount(row5), 2);
        QCOMPARE(menu.data(row5, QMenuModel::Depth).toInt(), 1);
        QCOMPARE(menu.data(row5, QMenuModel::Label).toString(), QString("menu5"));

        QModelIndex row6 = row5.model()->index(0, 0, row5);
        QVERIFY(row6.isValid());
        QCOMPARE(menu.rowCount(row6), 0);
        QCOMPARE(menu.data(row6, QMenuModel::Depth).toInt(), 2);
        QCOMPARE(menu.data(row6, QMenuModel::Label).toString(), QString("menu6"));

        QModelIndex row7 = row5.model()->index(1, 0, row5);
        QVERIFY(row7.isValid());
        QCOMPARE(menu.rowCount(row7), 0);
        QCOMPARE(menu.data(row7, QMenuModel::Depth).toInt(), 2);
        QCOMPARE(menu.data(row7, QMenuModel::Label).toString(), QString("menu7"));

        QModelIndex parent_6 = menu.parent(row6);
        QVERIFY(parent_6.isValid());
        QCOMPARE(menu.rowCount(parent_6), 2);
        QCOMPARE(menu.data(parent_6, QMenuModel::Depth).toInt(), 1);
        QCOMPARE(menu.data(parent_6, QMenuModel::Label).toString(), QString("menu5"));
    }
};

QTEST_MAIN(TreeTest)

#include "treetest.moc"

