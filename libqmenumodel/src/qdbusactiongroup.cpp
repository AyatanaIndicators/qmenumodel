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

#include "actionstateparser.h"
#include "qdbusactiongroup.h"
#include "qstateaction.h"
#include "converter.h"
#include "qmenumodelevents.h"

// Qt
#include <QCoreApplication>

#include <glib.h>
#include <gio/gio.h>

/*!
    \qmltype QDBusActionGroup
    \inherits QDBusObject

    \brief A DBusActionGroup implementation to be used with \l QDBusMenuModel

    \b {This component is under heavy development.}

     This class can be used as a proxy for an action group that is exported over D-Bus

    \code
    QDBusActionGroup {
        id: actionGroup
        busType: 1
        busName: "org.ayatana.menu"
        objectPath: "org/ayatana/menu/actions"
    }

    Button {
        onClicked: actionGroup.getAction("app.quit").trigger()
    }
    \endcode
*/

/*! \internal */
QDBusActionGroup::QDBusActionGroup(QObject *parent)
    :QObject(parent),
     QDBusObject(this),
     m_actionGroup(NULL),
     m_actionStateParser(new ActionStateParser(this))
{
}

/*! \internal */
QDBusActionGroup::~QDBusActionGroup()
{
    clear();
}

QStringList QDBusActionGroup::actions() const
{
    if (!m_actionGroup) return QStringList();
    QStringList list;
    gchar** actions = g_action_group_list_actions(m_actionGroup);
    for (uint i = 0; actions[i]; i++) {
        list << QString(actions[i]);
    }
    g_strfreev(actions);
    return list;
}

/*!
    \qmlmethod QDBusActionGroup::action(QString name)

    Look for a action with the same name and return a \l QStateAction object.

    \b Note: methods should only be called after the Component has completed.
*/
QStateAction *QDBusActionGroup::action(const QString &name)
{
    QStateAction *act = actionImpl(name);
    if (act == 0) {
        act = new QStateAction(this, name);
    }

    return act;
}

QVariant QDBusActionGroup::actionState(const QString &name)
{
    QVariant result;
    GVariant *state = g_action_group_get_action_state(m_actionGroup, name.toUtf8().data());

    if (m_actionStateParser != NULL) {
        result = m_actionStateParser->toQVariant(state);
    } else {
        result = Converter::toQVariant(state);
    }

    if (state) {
        g_variant_unref(state);
    }
    return result;
}


bool QDBusActionGroup::hasAction(const QString &name)
{
    if (m_actionGroup) {
        return g_action_group_has_action(m_actionGroup, name.toUtf8().data());
    } else {
        return false;
    }
}

QStateAction *QDBusActionGroup::actionImpl(const QString &name)
{
    Q_FOREACH(QStateAction *act, this->findChildren<QStateAction*>()) {
        if (act->name() == name) {
            return act;
        }
    }
    return 0;
}

/*! \internal */
void QDBusActionGroup::serviceVanish(GDBusConnection *)
{
    setActionGroup(NULL);
}

/*! \internal */
void QDBusActionGroup::serviceAppear(GDBusConnection *connection)
{
    GDBusActionGroup *ag = g_dbus_action_group_get(connection,
                                                   busName().toUtf8().data(),
                                                   objectPath().toUtf8().data());
    setActionGroup(ag);
    if (ag == NULL) {
        stop();
    }
}

/*! \internal */
void QDBusActionGroup::start()
{
    QDBusObject::connect();
}

/*! \internal */
void QDBusActionGroup::stop()
{
    QDBusObject::disconnect();
}

/*! \internal */
void QDBusActionGroup::setIntBusType(int busType)
{
    if ((busType > DBusEnums::None) && (busType < DBusEnums::LastBusType)) {
        setBusType(static_cast<DBusEnums::BusType>(busType));
    }
}

/*! \internal */
void QDBusActionGroup::setActionGroup(GDBusActionGroup *ag)
{
    if (m_actionGroup == reinterpret_cast<GActionGroup*>(ag)) {
        return;
    }

    clear();

    m_actionGroup = reinterpret_cast<GActionGroup*>(ag);

    if (m_actionGroup) {
        m_signalActionAddId = g_signal_connect(m_actionGroup,
                                               "action-added",
                                               G_CALLBACK(QDBusActionGroup::onActionAdded),
                                               this);

        m_signalActionRemovedId = g_signal_connect(m_actionGroup,
                                                   "action-removed",
                                                   G_CALLBACK(QDBusActionGroup::onActionRemoved),
                                                   this);

        m_signalStateChangedId = g_signal_connect(m_actionGroup,
                                                  "action-state-changed",
                                                   G_CALLBACK(QDBusActionGroup::onActionStateChanged),
                                                   this);

        gchar **actions = g_action_group_list_actions(m_actionGroup);
        for(guint i=0; i < g_strv_length(actions); i++) {
            DBusActionVisiblityEvent dave(actions[i], true);
            QCoreApplication::sendEvent(this, &dave);
        }
        g_strfreev(actions);
    }
}

ActionStateParser* QDBusActionGroup::actionStateParser() const
{
    return m_actionStateParser;
}

void QDBusActionGroup::setActionStateParser(ActionStateParser* actionStateParser)
{
    if (m_actionStateParser != actionStateParser) {
        m_actionStateParser = actionStateParser;
        Q_EMIT actionStateParserChanged(actionStateParser);
    }
}

/*! \internal */
void QDBusActionGroup::clear()
{
    if (m_actionGroup) {
        g_signal_handler_disconnect(m_actionGroup, m_signalActionAddId);
        g_signal_handler_disconnect(m_actionGroup, m_signalActionRemovedId);
        g_signal_handler_disconnect(m_actionGroup, m_signalStateChangedId);
        m_signalActionAddId = m_signalActionRemovedId = m_signalStateChangedId = 0;
    }

    Q_FOREACH(QStateAction *act, this->findChildren<QStateAction*>()) {
        Q_EMIT actionVanish(act->name());
    }

    if (m_actionGroup != NULL) {
        g_object_unref(m_actionGroup);
        m_actionGroup = NULL;
    }
}

/*! \internal */
void QDBusActionGroup::updateActionState(const QString &name, const QVariant &state)
{
    if (m_actionGroup != NULL) {
        g_action_group_change_action_state(m_actionGroup, name.toUtf8().data(), Converter::toGVariant(state));
    }
}

void QDBusActionGroup::activateAction(const QString &name, const QVariant &parameter)
{
    if (m_actionGroup != NULL) {
        g_action_group_activate_action(m_actionGroup, name.toUtf8().data(), Converter::toGVariant(parameter));
    }
}

bool QDBusActionGroup::event(QEvent* e)
{
    if (QDBusObject::event(e)) {
        return true;
    } else if (e->type() == DBusActionVisiblityEvent::eventType) {
        DBusActionVisiblityEvent *dave = static_cast<DBusActionVisiblityEvent*>(e);

        if (dave->visible) {
            Q_EMIT actionAppear(dave->name);
        } else {
            Q_EMIT actionVanish(dave->name);
        }
        Q_EMIT actionsChanged();
    } else if (e->type() == DBusActionStateEvent::eventType) {
        DBusActionStateEvent *dase = static_cast<DBusActionStateEvent*>(e);

        Q_EMIT actionStateChanged(dase->name, dase->state);
    }
    return QObject::event(e);
}

/*! \internal */
void QDBusActionGroup::onActionAdded(GDBusActionGroup *, gchar *name, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);

    DBusActionVisiblityEvent dave(name, true);
    QCoreApplication::sendEvent(self, &dave);
}

/*! \internal */
void QDBusActionGroup::onActionRemoved(GDBusActionGroup *, gchar *name, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);

    DBusActionVisiblityEvent dave(name, false);
    QCoreApplication::sendEvent(self, &dave);
}

/*! \internal */
void QDBusActionGroup::onActionStateChanged(GDBusActionGroup *, gchar *name, GVariant *value, gpointer data)
{
    QDBusActionGroup *self = reinterpret_cast<QDBusActionGroup*>(data);

    DBusActionStateEvent dase(name, Converter::toQVariant(value));
    QCoreApplication::sendEvent(self, &dase);
}
