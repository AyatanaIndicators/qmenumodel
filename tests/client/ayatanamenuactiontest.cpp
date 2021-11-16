/*
 * Copyright 2014 Canonical Ltd.
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
 *      Nick Dedekind <nick.dedekind@canonical.com>
 */

#include "ayatanamenumodel.h"
#include "ayatanamenuaction.h"

#include <QObject>
#include <QtTest>

class UnityMenuActionTest : public QObject
{
    Q_OBJECT
private:

private Q_SLOTS:

    /*
     * Test if the propety busType handle correct integer values
     */
    void testDestroyAfterModel()
    {
        UnityMenuModel* model = new UnityMenuModel;
        UnityMenuAction* action = new UnityMenuAction;
        action->setModel(model);

        delete model;
        delete action;
    }

    /*
     * Test if the propety busType handle correct integer values
     */
    void testDestroyBeforeModel()
    {
        UnityMenuModel* model = new UnityMenuModel;
        UnityMenuAction* action = new UnityMenuAction;
        action->setModel(model);

        delete action;
        delete model;
    }
};

QTEST_MAIN(UnityMenuActionTest)

#include "unitymenuactiontest.moc"
