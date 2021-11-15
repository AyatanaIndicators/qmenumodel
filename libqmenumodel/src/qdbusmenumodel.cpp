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

#include <gio/gio.h>

#include "qdbusmenumodel.h"
#include "qmenumodelevents.h"

#include <QCoreApplication>

/*!
    \qmltype QDBusMenuModel
    \inherits QDBusObject

    \brief The QDBusMenuModel class defines the list model for DBus menus

    \b {This component is under heavy development.}

    This class expose the menu previous exported over DBus.

    \code
    QDBusMenuModel {
        id: menuModel
        busType: 1
        busName: "org.ayatana.menu"
        objectPath: "org/ayatana/menu"
    }

    ListView {
        id: view
        model: menuModel
        Component.onCompleted: menuModel.start()
    }
    \endcode
*/
QDBusMenuModel::QDBusMenuModel(QObject *parent)
    : QMenuModel(0, parent),
      QDBusObject(this)
{
}


/*! \internal */
QDBusMenuModel::~QDBusMenuModel()
{
}

/*!
    \qmlmethod QDBusMenuModel::start()

    Start dbus watch for the busName and wait until it appears.
    The status will change to connecting after call this function, and as soon the busName
    apperas and the objectPat was found this will change to Connected.

    \b Note: methods should only be called after the Component has completed.
*/
void QDBusMenuModel::start()
{
    QDBusObject::connect();
}

/*!
    \qmlmethod QDBusMenuModel::stop()

    Stops dbus watch and clear the model, the status wil change to Disconnected.

    \b Note: methods should only be called after the Component has completed.
*/
void QDBusMenuModel::stop()
{
    QDBusObject::disconnect();

    MenuModelEvent mme(NULL);
    QCoreApplication::sendEvent(this, &mme);
}

bool QDBusMenuModel::event(QEvent* e)
{
    if (QDBusObject::event(e)) {
        return true;
    }
    return QMenuModel::event(e);
}

/*! \internal */
void QDBusMenuModel::serviceVanish(GDBusConnection *)
{
    MenuModelEvent mme(NULL);
    QCoreApplication::sendEvent(this, &mme);
}

/*! \internal */
void QDBusMenuModel::serviceAppear(GDBusConnection *connection)
{
    GMenuModel *model = G_MENU_MODEL(g_dbus_menu_model_get(connection,
                                                           busName().toUtf8().data(),
                                                           objectPath().toUtf8().data()));

    MenuModelEvent mme(model);
    QCoreApplication::sendEvent(this, &mme);
    //event handling takes care of the ref
    g_object_unref(model);
}

/*! \internal */
void QDBusMenuModel::setIntBusType(int busType)
{
    if ((busType > DBusEnums::None) && (busType < DBusEnums::LastBusType)) {
        setBusType(static_cast<DBusEnums::BusType>(busType));
    }
}
