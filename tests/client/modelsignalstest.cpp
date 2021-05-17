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

#include "qmenumodel.h"

#include <QObject>
#include <QtTest>
#include <QDebug>

#include <gio/gio.h>


class MenuModelTestClass : public QMenuModel
{
    Q_OBJECT
public:
    MenuModelTestClass()
        : QMenuModel(G_MENU_MODEL(g_menu_new())), m_step(0)
    {
    }

    void loadModel()
    {
        GMenu *root = G_MENU(menuModel());

        // STEP 0
        m_step = 0;
        GMenu *section = g_menu_new();
        g_menu_append(section, "msg1", NULL);
        g_menu_append_section(root, "section1", G_MENU_MODEL(section));

        // STEP 1
        m_step++;
        GMenu *section2 = g_menu_new();
        g_menu_append_section(root, "section2", G_MENU_MODEL(section2));

        // STEP 2
        m_step++;
        g_menu_append(root, "item1", NULL);

        // STEP 3
        m_step++;
        g_menu_insert(root, 1, "item2", NULL);
    }

    void clear()
    {
        GMenu *root = G_MENU(menuModel());

        // STEP 0
        m_step = 0;
        g_menu_remove(root, 0);

        // STEP 1
        m_step++;
        g_menu_remove(root, 2);

        // STEP 2
        m_step++;
        g_menu_remove(root, 0);

        // STEP 3
        m_step++;
        g_menu_remove(root, 0);
    }

public Q_SLOTS:
    void checkModelStateBeforeInsert(const QModelIndex &parent, int start, int end)
    {
        if (m_step == 0) {
            QCOMPARE(rowCount(), 0);
            QVERIFY(data(index(1), QMenuModel::Label).isNull());
        } else if (m_step == 1) {
            QCOMPARE(rowCount(), 1);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("section1"));
            QVERIFY(data(index(1), QMenuModel::Label).isNull());
        } else if (m_step == 2) {
            QCOMPARE(rowCount(), 2);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("section1"));
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("section2"));
            QVERIFY(data(index(2), QMenuModel::Label).isNull());
        } else if (m_step == 3) {
            QCOMPARE(rowCount(), 3);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("section1"));
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("section2"));
            QCOMPARE(data(index(2), QMenuModel::Label).toString(), QString("item1"));
            QVERIFY(data(index(3), QMenuModel::Label).isNull());
        }
    }

    void checkModelStateAfterInsert(const QModelIndex &parent, int start, int end)
    {
        if (m_step == 0) {
            QCOMPARE(rowCount(), 1);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("section1"));
            QVERIFY(data(index(1), QMenuModel::Label).isNull());
        } else if (m_step == 1) {
            QCOMPARE(rowCount(), 2);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("section1"));
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("section2"));
            QVERIFY(data(index(2), QMenuModel::Label).isNull());
        } else if (m_step == 2) {
            QCOMPARE(rowCount(), 3);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("section1"));
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("section2"));
            QCOMPARE(data(index(2), QMenuModel::Label).toString(), QString("item1"));
            QVERIFY(data(index(3), QMenuModel::Label).isNull());
        } else if (m_step == 3) {
            QCOMPARE(rowCount(), 4);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("section1"));
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("item2"));
            QCOMPARE(data(index(2), QMenuModel::Label).toString(), QString("section2"));
            QCOMPARE(data(index(3), QMenuModel::Label).toString(), QString("item1"));
            QVERIFY(data(index(4), QMenuModel::Label).isNull());
        }
    }

    void checkModelStateBeforeRemove(const QModelIndex &parent, int start, int end)
    {
        if (m_step == 0) {
            QCOMPARE(rowCount(), 4);
            QVERIFY(data(index(0), QMenuModel::Label).isNull());
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("item2"));
            QCOMPARE(data(index(2), QMenuModel::Label).toString(), QString("section2"));
            QCOMPARE(data(index(3), QMenuModel::Label).toString(), QString("item1"));
            QVERIFY(data(index(4), QMenuModel::Label).isNull());
        } else if (m_step == 1) {
            QCOMPARE(rowCount(), 3);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("item2"));
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("section2"));
            QVERIFY(data(index(3), QMenuModel::Label).isNull());
            QVERIFY(data(index(4), QMenuModel::Label).isNull());
        } else if (m_step == 2) {
            QCOMPARE(rowCount(), 2);
            QVERIFY(data(index(0), QMenuModel::Label).isNull());
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("section2"));
            QVERIFY(data(index(2), QMenuModel::Label).isNull());
        } else if (m_step == 3) {
            QCOMPARE(rowCount(), 1);
            QVERIFY(data(index(0), QMenuModel::Label).isNull());
            QVERIFY(data(index(1), QMenuModel::Label).isNull());
        }
    }

    void checkModelStateAfterRemove(const QModelIndex &parent, int start, int end)
    {
        if (m_step == 0) {
            QCOMPARE(rowCount(), 3);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("item2"));
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("section2"));
            QCOMPARE(data(index(2), QMenuModel::Label).toString(), QString("item1"));
            QVERIFY(data(index(3), QMenuModel::Label).isNull());
        } else if (m_step == 1) {
            QCOMPARE(rowCount(), 2);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("item2"));
            QCOMPARE(data(index(1), QMenuModel::Label).toString(), QString("section2"));
            QVERIFY(data(index(3), QMenuModel::Label).isNull());
        } else if (m_step == 2) {
            QCOMPARE(rowCount(), 1);
            QCOMPARE(data(index(0), QMenuModel::Label).toString(), QString("section2"));
            QVERIFY(data(index(1), QMenuModel::Label).isNull());
        } else if (m_step == 3) {
            QCOMPARE(rowCount(), 0);
            QVERIFY(data(index(0), QMenuModel::Label).isNull());
        }
    }

private:
    int m_step;
};


class ModelSignalsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    /*
     * Test if the model state still correct before and after insert a new row
     */
    void testSignalInsertRows()
    {
        MenuModelTestClass model;
        QObject::connect(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                         &model, SLOT(checkModelStateBeforeInsert(QModelIndex,int,int)));
        QObject::connect(&model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                         &model, SLOT(checkModelStateAfterInsert(QModelIndex,int,int)));
        model.loadModel();
    }

    /*
     * Test if the model state still correct before and after remove a row
     */
    void testSignalRemoveRows()
    {
        MenuModelTestClass model;
        model.loadModel();

        QObject::connect(&model, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                         &model, SLOT(checkModelStateBeforeRemove(QModelIndex,int,int)));
        QObject::connect(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                         &model, SLOT(checkModelStateAfterRemove(QModelIndex,int,int)));
        model.clear();
    }
};

QTEST_MAIN(ModelSignalsTest)

#include "modelsignalstest.moc"
