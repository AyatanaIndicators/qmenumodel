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

#include "dbusmenuscript.h"
#include <QObject>
#include <QtTestGui>
#include <QDebug>

#define WAIT_TIMEOUT   2000

DBusMenuScript::DBusMenuScript()
    :m_script(0)
{
}

DBusMenuScript::~DBusMenuScript()
{
    quit();
}

bool DBusMenuScript::connect()
{
    QTest::qWait(WAIT_TIMEOUT);
    m_script = new QDBusInterface(SCRIPT_SERVICE_NAME,
                                  SCRIPT_OBJECT_PATH,
                                  SCRIPT_INTERFACE_NAME,
                                  QDBusConnection::sessionBus(), 0);
    if (m_script && m_script->isValid()) {
        return true;
    } else {
        qWarning() << "DBUS ERROR:" << m_script->lastError().message();
        return false;
    }
}

void DBusMenuScript::publishMenu()
{
    if (m_script) {
        m_script->call("publishMenu");
        QTest::qWait(WAIT_TIMEOUT);
    }
}

void DBusMenuScript::unpublishMenu()
{
    if (m_script) {
        m_script->call("unpublishMenu");
        QTest::qWait(WAIT_TIMEOUT);
    }
}


void DBusMenuScript::quit()
{
    if (m_script) {
        m_script->call("quit");
        QTest::qWait(WAIT_TIMEOUT);
        delete m_script;
        m_script = 0;
    }
}

void DBusMenuScript::walk(int steps)
{
    if (m_script) {
        m_script->call("walk", steps);
        QTest::qWait(WAIT_TIMEOUT);
    }
}

void DBusMenuScript::run()
{
    if (m_script) {
        m_script->call("walk", -1);
        QTest::qWait(WAIT_TIMEOUT);
    }
}

QPair<QString, QVariant> DBusMenuScript::popActivatedAction()
{
    if (m_script) {
        QDBusMessage reply = m_script->call("popActivatedAction");
        if (reply.arguments().count() > 0) {
            QVariant value;
            QString name = reply.arguments()[0].toString();
            if (reply.arguments().count() > 1) {
                value = reply.arguments()[1];
            }
            return qMakePair(name, value);
        }
    }

    return qMakePair(QString(), QVariant());
}

