/*
 * Copyright 2013 Canonical Ltd.
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
 *      Nicholas Dedekind <nick.dedekind@canonical.com
 */

#include <glib-object.h>
#include <gio/gio.h>

#include "qmenumodelevents.h"

const QEvent::Type MenuNodeItemChangeEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type DBusActionStateEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type DBusActionVisiblityEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type MenuModelEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type DbusObjectServiceEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

MenuNodeItemChangeEvent::MenuNodeItemChangeEvent(MenuNode* _node, int _position, int _removed, int _added)
    : QEvent(MenuNodeItemChangeEvent::eventType),
      node(_node),
      position(_position),
      removed(_removed),
      added(_added)
{}


DBusActionEvent::DBusActionEvent(const QString& _name, QEvent::Type type)
    : QEvent(type),
      name(_name)
{
}


DBusActionVisiblityEvent::DBusActionVisiblityEvent(const QString& _name, bool _visible)
    : DBusActionEvent(_name, DBusActionVisiblityEvent::eventType),
      visible(_visible)
{
}


DBusActionStateEvent::DBusActionStateEvent(const QString& _name, const QVariant& _state)
    : DBusActionEvent(_name, DBusActionStateEvent::eventType),
      state(_state)
{
}


DbusObjectServiceEvent::DbusObjectServiceEvent(GDBusConnection* _connection, bool _visible)
    : QEvent(DbusObjectServiceEvent::eventType),
      connection(_connection),
      visible(_visible)
{
    if (connection) {
        g_object_ref(connection);
    }
}


DbusObjectServiceEvent::~DbusObjectServiceEvent()
{
    if (connection) {
        g_object_unref(connection);
    }
}


MenuModelEvent::MenuModelEvent(GMenuModel* _model)
    : QEvent(MenuModelEvent::eventType),
      model(_model)
{
    if (model) {
        g_object_ref(model);
    }
}

MenuModelEvent::~MenuModelEvent()
{
    if (model) {
        g_object_unref(model);
    }
}
