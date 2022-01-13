/*
 * Copyright 2012 Canonical Ltd.
 * Copyright 2022 Robert Tari
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
 *      Robert Tari <robert@tari.in>
 */

#ifndef DBUSMENUSCRIPT_H
#define DBUSMENUSCRIPT_H

#include <QObject>
#include <QDBusInterface>
#include <QPair>

#define SCRIPT_SERVICE_NAME     "org.ayatana.test"
#define SCRIPT_OBJECT_PATH      "/org/ayatana/test/menuscript"
#define SCRIPT_INTERFACE_NAME   "org.ayatana.test.menuscript"

#define MENU_SERVICE_NAME       SCRIPT_SERVICE_NAME ".menu"
#define MENU_OBJECT_PATH        SCRIPT_OBJECT_PATH "/menu"


class DBusMenuScript
{
public:
    DBusMenuScript();
    ~DBusMenuScript();

    bool connect();
    void quit();

    void walk(int steps = 1);
    void run();

    void publishMenu();
    void unpublishMenu();

    QPair<QString, QVariant> popActivatedAction();

private:
    QDBusInterface *m_script;
};

#endif
