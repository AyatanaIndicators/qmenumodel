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

#ifndef QMENUMODELEVENTS_H
#define QMENUMODELEVENTS_H

#include <QEvent>
#include <QVariant>

#include "menunode.h"

typedef struct _GDBusConnection GDBusConnection;
typedef struct _GMenuModel GMenuModel;

/* Event for a connection update for a dbus object */
class DbusObjectServiceEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    DbusObjectServiceEvent(GDBusConnection* connection, bool visible);
    ~DbusObjectServiceEvent();

    GDBusConnection* connection;
    bool visible;
};

/* Event for an update to the gmenumodel */
class MenuModelEvent : public QEvent
{
public:
    static const QEvent::Type eventType;

    MenuModelEvent(GMenuModel *model);
    ~MenuModelEvent();

    GMenuModel *model;
};

/* Event for a GAction (base) */
class DBusActionEvent : public QEvent
{
public:
    QString name;

protected:
    DBusActionEvent(const QString& name, QEvent::Type type);
};

/* Event for a GAction add/remove */
class DBusActionVisiblityEvent : public DBusActionEvent
{
public:
    static const QEvent::Type eventType;
    DBusActionVisiblityEvent(const QString& name, bool visible);

    bool visible;
};

/* Event for a GAction state value update */
class DBusActionStateEvent : public DBusActionEvent
{
public:
    static const QEvent::Type eventType;

    DBusActionStateEvent(const QString& name, const QVariant& state);

    QVariant state;
};

/* Event for changing gmenumodel entries */
class MenuNodeItemChangeEvent : public QEvent
{
public:
    static const QEvent::Type eventType;

    MenuNodeItemChangeEvent(MenuNode* node, int position, int removed, int added);

    MenuNode* node;
    int position;
    int removed;
    int added;
};

#endif //QMENUMODELEVENTS_H
