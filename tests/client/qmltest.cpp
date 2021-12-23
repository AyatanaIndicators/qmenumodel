/*
 * Copyright 2012 Canonical Ltd.
 * Copyright 2021 UBports Foundation
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
 *      Dalton Durst <dalton@ubports.com>
 */

#include <memory>

#include <glib-object.h>

#include "qdbusmenumodel.h"
#include "dbusmenuscript.h"
#include "qmlfiles.h"

#include <QObject>
#include <QSignalSpy>
#include <QtTest>
#include <QDebug>
#include <QString>

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>


class QMLTest : public QObject
{
    Q_OBJECT
private:
    DBusMenuScript m_script;
    std::shared_ptr<QQuickView> loadView(const QString baseDir, const QString fileLocation)
    {
        std::shared_ptr<QQuickView> view = std::make_shared<QQuickView>();
        view->engine()->addImportPath(baseDir);
        view->setSource(QUrl::fromLocalFile(fileLocation));
        return view;
    }

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
     * Test if importtest.qml can be loaded successfully
     */
    void importtest()
    {
        std::shared_ptr<QQuickView> view = loadView(QML_BASE_DIR, IMPORTTEST_QML);
        QTRY_VERIFY2((view->status() == QQuickView::Ready), "loadmodel.qml view never became ready");
    }
};

QTEST_MAIN(QMLTest)

#include "qmltest.moc"
