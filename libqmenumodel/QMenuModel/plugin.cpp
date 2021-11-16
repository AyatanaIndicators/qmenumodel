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

#include "plugin.h"
#include "qmenumodel.h"
#include "qdbusmenumodel.h"
#include "qdbusactiongroup.h"
#include "qstateaction.h"
#include "ayatanamenuaction.h"
#include "ayatanamenumodel.h"

#include <QtQml>

void QMenuModelQmlPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
}

void QMenuModelQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterUncreatableType<QMenuModel>(uri, 0, 1, "QMenuModel",
                                           "QMenuModel is a interface");
    qmlRegisterUncreatableType<QStateAction>(uri, 0, 1, "QStateAction",
                                             "QStateAction must be created by QDBusActionGroup::action");
    qmlRegisterUncreatableType<DBusEnums>(uri, 0, 1, "DBus",
                                          "DBus is only a namespace");

    qmlRegisterType<QDBusMenuModel>(uri, 0, 1, "QDBusMenuModel");
    qmlRegisterType<QDBusActionGroup>(uri, 0, 1, "QDBusActionGroup");
    qmlRegisterType<UnityMenuModel>(uri, 0, 1, "UnityMenuModel");
    qmlRegisterType<UnityMenuAction>(uri, 0, 1, "UnityMenuAction");
}
