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
        GMenu *menu3 = g_menu_new();
        g_menu_append(menu3, "menu4", NULL);
        g_menu_append(menu3, "menu5", NULL);
        g_menu_append(menu3, "menu6", NULL);

        GMenu *menu = g_menu_new();
        g_menu_append(menu, "menu0", NULL);
        g_menu_append(menu, "menu1", NULL);
        g_menu_append(menu, "menu2", NULL);
        g_menu_append_section(menu, "menu3", G_MENU_MODEL(menu3));

        setMenuModel(G_MENU_MODEL(menu));

        m_menus << menu << menu3;
    }

    void removeItem(int section, int index)
    {
        GMenu *menu = m_menus[section];
        g_menu_remove(menu, index);
    }

    void insertItem(int section, int index, const QString &label)
    {
        GMenu *menu = m_menus[section];
        g_menu_insert(menu, index, label.toUtf8().data(), NULL);
    }

    QList<int> cacheIndexes() const
    {
        QList<int> indexes = cache().keys();
        qSort(indexes);
        return indexes;
    }

private:
    QList<GMenu*> m_menus;
};

class CacheTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    // Verify that normal menu items are not cached (only sub-menus are)
    void testCacheContents()
    {
        TestModel menu;
        QVERIFY(menu.cacheIndexes().isEmpty());

        menu.data(menu.index(1), QMenuModel::Label);
        QVERIFY(menu.cacheIndexes().isEmpty());

        menu.data(menu.index(2), QMenuModel::Action);
        QVERIFY(menu.cacheIndexes().isEmpty());
    }

    // Verify that the link attribute always returns the same cached menu
    void testStaticMenuCache()
    {
        TestModel menu;

        QModelIndex index = menu.index(3);

        QVariant data = menu.data(index, QMenuModel::LinkSection);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 3);

        QVariant data2 = menu.data(index, QMenuModel::LinkSection);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 3);

        QVERIFY(data.value<QObject*>() == data2.value<QObject*>());

        QMenuModel *section = qvariant_cast<QMenuModel*>(data);

        index = section->index(1);
        data = menu.data(index, QMenuModel::LinkSection);
        data2 = menu.data(index, QMenuModel::LinkSection);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 3);
        QVERIFY(data.value<QObject*>() == data2.value<QObject*>());
    }

    // Verify that the cache is correctly updated after inserting a new item
    void testInsertItems()
    {
        TestModel menu;

        QModelIndex index = menu.index(3);
        QVariant data = menu.data(index, QMenuModel::LinkSection);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 3);

        menu.insertItem(0, 4, "newMenu");
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 3);

        menu.insertItem(0, 1, "newMenu");
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 4);

        index = menu.index(4);
        QVariant data2 = menu.data(index, QMenuModel::LinkSection);

        QCOMPARE(menu.cacheIndexes(), QList<int>() << 4);
        QVERIFY(data.value<QObject*>() == data2.value<QObject*>());
    }

    // Verify that the cache is correctly updated after removing an item that wasn’t cached
    void testRemoveNonCachedItem()
    {
        TestModel menu;

        QModelIndex index = menu.index(3);
        QVariant data = menu.data(index, QMenuModel::LinkSection);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 3);

        menu.removeItem(0, 1);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 2);

        index = menu.index(2);
        QVariant data2 = menu.data(index, QMenuModel::LinkSection);

        QCOMPARE(menu.cacheIndexes(), QList<int>() << 2);
        QVERIFY(data.value<QObject*>() == data2.value<QObject*>());
    }

    // Verify that the cache is correctly updated after removing a cached item
    void testRemoveCachedItem()
    {
        TestModel menu;

        QModelIndex index = menu.index(3);
        QVariant data = menu.data(index, QMenuModel::LinkSection);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 3);

        menu.removeItem(0, 3);
        QVERIFY(menu.cacheIndexes().isEmpty());
    }

    // Verify that the cache is correctly updated after multiple insertions and removals 
    void testMultiplesUpdates()
    {
        TestModel menu;
        QVERIFY(menu.cacheIndexes().isEmpty());

        menu.data(menu.index(3), QMenuModel::LinkSection);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 3);

        menu.insertItem(0, 1, "newMenu");
        menu.insertItem(0, 2, "newMenu");
        menu.insertItem(0, 6, "newMenu");
        menu.insertItem(0, 3, "newMenu");
        menu.insertItem(0, 7, "newMenu");
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 6);

        menu.removeItem(0, 4);
        menu.removeItem(0, 6);
        menu.removeItem(0, 2);
        QCOMPARE(menu.cacheIndexes(), QList<int>() << 4);

        menu.removeItem(0, 4);
        QVERIFY(menu.cacheIndexes().isEmpty());
    }
};

QTEST_MAIN(CacheTest)

#include "cachetest.moc"

