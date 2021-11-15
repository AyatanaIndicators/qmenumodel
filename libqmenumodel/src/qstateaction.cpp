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

#include "qstateaction.h"

#include "converter.h"
#include "qdbusactiongroup.h"

/*!
    \qmltype QStateAction
    \inherits QAction

    \brief A QStateAction implementation to be used with \l QDBusActionGroup

    \b {This component is under heavy development.}

     This class can be used as a proxy for an action that is exported over D-Bus

    \code
    QDBusActionGroup {
        id: actionGroup
        busType: 1
        busName: "org.ayatana.menu"
        objectPath: "org/ayatana/menu/actions"
    }

    Button {
        visible: actionGroup.getAction("button.bvisible").status
    }
    \endcode
*/

/*! \internal */
QStateAction::QStateAction(QDBusActionGroup *group, const QString &name)
    : QObject(group),
      m_group(group),
      m_name(name)
{
    // This keep the code clean
    // But maybe we need move the action state control to QActionGroup to optimizations
    QObject::connect(m_group, SIGNAL(actionAppear(QString)),
                     this, SLOT(onActionAppear(QString)));
    QObject::connect(m_group, SIGNAL(actionVanish(QString)),
                     this, SLOT(onActionVanish(QString)));
    QObject::connect(m_group, SIGNAL(actionStateChanged(QString,QVariant)),
                     this, SLOT(onActionStateChanged(QString,QVariant)));



    m_valid = m_group->hasAction(name);
    if (m_valid) {
        setState(m_group->actionState(name));
    }
}

/*!
    \qmlproperty int QStateAction::state
    This property holds the current action state
*/
QVariant QStateAction::state() const
{
    return m_state;
}

/*!
    \qmlproperty int QStateAction::isValid
    This property return if the current Action is valid or not
    A valid Action is a action which has a DBus action linked
*/
bool QStateAction::isValid() const
{
    return m_valid;
}

/*!
    Request for the state of action to be changed to \a paramenter.
    This call merely requests a change. The action may refuse to change its state or may change its state to something other than \a paramenter.
*/
void QStateAction::updateState(const QVariant &state)
{
    QVariant v = state;
    if (v.convert(m_state.type()))
      m_group->updateActionState(m_name, v);
}

/*!
    Request for the state of action to be changed to \a paramenter.
    This call merely requests a change. The action may refuse to change its state or may change its state to something other than \a paramenter.
*/
void QStateAction::updateStateByVariantString(const QString &state)
{
    updateState(Converter::toQVariantFromVariantString(state));
}

/*!
    Activates the action passing \a parameter.
    \a parameter must be the correct type of parameter for the action
*/
void QStateAction::activate(const QVariant &parameter)
{
    m_group->activateAction(m_name, parameter);
}

/*!
    Activates the action passing \a parameter using GLib Variant String format.
    \a parameter must be the correct type of parameter for the action
*/
void QStateAction::activateByVariantString(const QString &parameter)
{
    activate(Converter::toQVariantFromVariantString(parameter));
}


/*! \internal */
QString QStateAction::name() const
{
    return m_name;
}

/*! \internal */
void QStateAction::setValid(bool valid)
{
    if (m_valid != valid) {
        m_valid = valid;
        Q_EMIT validChanged(m_valid);
    }
}

/*! \internal */
void QStateAction::setState(const QVariant &state)
{
    QVariant v = state;
    if (!m_state.isValid() || (v.convert(m_state.type()) && v != m_state)) {
        m_state = v;
        Q_EMIT stateChanged(m_state);
    }
}

/*! \internal */
void QStateAction::onActionAppear(const QString &name)
{
    if (m_name == name) {
        setState(m_group->actionState(name));
        setValid(true);
    }
}

/*! \internal */
void QStateAction::onActionVanish(const QString &name)
{
    if (m_name == name) {
        setState(QVariant());
        setValid(false);
    }
}

/*! \internal */
void QStateAction::onActionStateChanged(const QString &name, const QVariant &state)
{
    if (m_name == name) {
        setState(state);
    }
}
