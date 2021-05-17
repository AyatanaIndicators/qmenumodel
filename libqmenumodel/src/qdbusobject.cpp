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
#include <gio/gio.h>

#include "qdbusobject.h"
#include "qmenumodelevents.h"

#include <QDebug>
#include <QCoreApplication>

/*!
    \qmltype QDBusObject
    \brief The QDBusObject is a base class

    \b {This component is under heavy development.}

    This is a abstracted class used by QDBusMenuModel and QDBusActionGroup
*/

/*!
    \qmlproperty int QDBusObject::busType
    This property holds the dbus session type which will be used during the connection.

    This must be seteed before call start method
    The valid values are:
    \list
        \li 1 - SessionBus
        \li 2 - SystemBus
    \endlist
*/

/*!
    \qmlproperty int QDBusObject::busName
    This property holds the dbus service name related with menu.

    This must be seteed before call start method
*/

/*!
    \qmlproperty int QDBusObject::objectPath
    This property holds the dbus object path related with the menu.

    This must be seteed before call start method
*/

/*!
    \qmlproperty int QDBusObject::status
    This property holds current dbus connection status

    Te velid status are:
    \list
        \li 0 - Disconnected
        \li 1 - Connecting
        \li 2 - Connected
    \endlist
*/

QDBusObject::QDBusObject(QObject* listener)
    :m_listener(listener),
     m_watchId(0),
     m_busType(DBusEnums::None),
     m_status(DBusEnums::Disconnected)
{
    qRegisterMetaType<DBusEnums::ConnectionStatus>("DBusEnums::ConnectionStatus");
}

QDBusObject::~QDBusObject()
{
    if (m_watchId != 0) {
        g_bus_unwatch_name (m_watchId);
        m_watchId = 0;
    }
}

DBusEnums::BusType QDBusObject::busType() const
{
    return m_busType;
}

void QDBusObject::setBusType(DBusEnums::BusType type)
{
    if (m_busType != type) {
        if (m_status != DBusEnums::Disconnected)
            disconnect();
        m_busType = type;
        Q_EMIT busTypeChanged(m_busType);
    }
}

QString QDBusObject::busName() const
{
    return m_busName;
}

void QDBusObject::setBusName(const QString &busName)
{
    if (m_busName != busName) {
        if (m_status != DBusEnums::Disconnected)
            disconnect();
        m_busName = busName;
        Q_EMIT busNameChanged(m_busName);
    }
}

QString QDBusObject::objectPath() const
{
    return m_objectPath;
}

void QDBusObject::setObjectPath(const QString &objectPath)
{
    if (m_objectPath != objectPath) {
        if (m_status != DBusEnums::Disconnected)
            disconnect();
        m_objectPath = objectPath;
        Q_EMIT objectPathChanged(m_objectPath);
    }
}

void QDBusObject::setStatus(DBusEnums::ConnectionStatus status)
{
    if (m_status != status) {
        m_status = status;
        Q_EMIT statusChanged(m_status);
    }
}

DBusEnums::ConnectionStatus QDBusObject::status() const
{
    return m_status;
}

void QDBusObject::connect()
{
    if (m_status != DBusEnums::Disconnected) {
        return;
    } else if ((m_busType > DBusEnums::None) && !m_objectPath.isEmpty() && !m_busName.isEmpty()) {
        GBusType type = m_busType == DBusEnums::SessionBus ? G_BUS_TYPE_SESSION : G_BUS_TYPE_SYSTEM;
        m_watchId = g_bus_watch_name (type,
                                      m_busName.toUtf8().data(),
                                      G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                      QDBusObject::onServiceAppeared,
                                      QDBusObject::onServiceVanished,
                                      this,
                                      NULL);

        setStatus(DBusEnums::Connecting);
    } else {
        qWarning() << "Invalid dbus connection args";
    }
}

void QDBusObject::disconnect()
{
    if (m_status != DBusEnums::Disconnected) {
        g_bus_unwatch_name (m_watchId);
        m_watchId = 0;
        setStatus(DBusEnums::Disconnected);
    }
}

void QDBusObject::onServiceAppeared(GDBusConnection *connection, const gchar *, const gchar *, gpointer data)
{
    QDBusObject *self = reinterpret_cast<QDBusObject*>(data);

    if (self->m_listener) {
        DbusObjectServiceEvent dose(connection, true);
        QCoreApplication::sendEvent(self->m_listener, &dose);
    }
}

void QDBusObject::onServiceVanished(GDBusConnection *connection, const gchar *, gpointer data)
{
    QDBusObject *self = reinterpret_cast<QDBusObject*>(data);

    if (self->m_listener) {
        DbusObjectServiceEvent dose(connection, false);
        QCoreApplication::sendEvent(self->m_listener, &dose);
    }
}

bool QDBusObject::event(QEvent* e)
{
    if (e->type() == DbusObjectServiceEvent::eventType) {
        DbusObjectServiceEvent *dose = static_cast<DbusObjectServiceEvent*>(e);
        if (dose->visible) {
            serviceAppear(dose->connection);
            setStatus(DBusEnums::Connected);
        } else {
            setStatus(DBusEnums::Connecting);
            serviceVanish(dose->connection);
        }
        return true;
    }
    return false;
}
