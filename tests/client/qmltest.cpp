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

#include <glib-object.h>

#include "qdbusmenumodel.h"
#include "dbusmenuscript.h"
#include "qmlfiles.h"

#include <QObject>
#include <QSignalSpy>
#include <QtTest>
#include <QDebug>

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>


class QMLTest : public QObject
{
    Q_OBJECT
private:
    DBusMenuScript m_script;

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(m_script.connect());
    }

    void cleanupTestCase()
    {
        m_script.quit();
    }

    void init()
    {
    }

    void cleanup()
    {
        m_script.unpublishMenu();
    }

    /*
     * Test if model is destroyed without crash
     */
    void destroyModel()
    {
        m_script.publishMenu();
        m_script.run();
        QTest::qWait(500);

        QQuickView *view = new QQuickView;
        view->engine()->addImportPath(QML_BASE_DIR);
        view->engine()->rootContext()->setContextProperty("resetModel", QVariant(false));
        view->engine()->rootContext()->setContextProperty("globalBusType", DBusEnums::SessionBus);
        view->engine()->rootContext()->setContextProperty("globalBusName", MENU_SERVICE_NAME);
        view->engine()->rootContext()->setContextProperty("globalObjectPath", MENU_OBJECT_PATH);
        view->setSource(QUrl::fromLocalFile(LOADMODEL_QML));
        QTest::qWait(500);
        view->engine()->rootContext()->setContextProperty("resetModel", true);
        QTest::qWait(500);
    }

    /*
     * Test the menu model disappearing from the bus and reappearing
     * while the QML application is running.
     */
    void testServiceDisappear()
    {
        m_script.publishMenu();
        m_script.run();
        QTest::qWait(500);

        QQuickView *view = new QQuickView;
        view->engine()->addImportPath(QML_BASE_DIR);
        view->engine()->rootContext()->setContextProperty("globalBusType", DBusEnums::SessionBus);
        view->engine()->rootContext()->setContextProperty("globalBusName", MENU_SERVICE_NAME);
        view->engine()->rootContext()->setContextProperty("globalObjectPath", MENU_OBJECT_PATH);
        view->setSource(QUrl::fromLocalFile(LOADMODEL2_QML));
        QTest::qWait(500);

        m_script.unpublishMenu();
        QTest::qWait(500);

        m_script.publishMenu();
        m_script.run();
        QTest::qWait(500);

        delete view;
        QTest::qWait(1000);
    }
};

QTEST_MAIN(QMLTest)

#include "qmltest.moc"
