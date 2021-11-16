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
 * Authors: Nick Dedekind <nick.dedekind@canonical.com>
 */

#include "ayatanamenuaction.h"
#include "ayatanamenumodel.h"
#include "ayatanamenuactionevents.h"

UnityMenuAction::UnityMenuAction(QObject* parent)
    :   QObject(parent),
        m_valid(false),
        m_enabled(false),
        m_model(NULL),
        m_index(-1)
{
}

UnityMenuAction::~UnityMenuAction()
{
    unregisterAction();
}

QString UnityMenuAction::name() const
{
    return m_name;
}

void UnityMenuAction::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged(m_name);
    }
}

UnityMenuModel* UnityMenuAction::model() const
{
    return m_model;
}

void UnityMenuAction::setModel(UnityMenuModel* model)
{
    if (m_model != model) {
        unregisterAction();
        m_model = model;
        registerAction();
        Q_EMIT modelChanged(model);
    }
}

QVariant UnityMenuAction::state() const
{
    return m_state;
}

void UnityMenuAction::setState(const QVariant& state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}

bool UnityMenuAction::isEnabled() const
{
    return m_enabled;
}

void UnityMenuAction::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged(m_enabled);
    }
}

bool UnityMenuAction::isValid() const
{
    return m_valid;
}

void UnityMenuAction::setValid(bool valid)
{
    if (m_valid != valid) {
        m_valid = valid;
        Q_EMIT validChanged(m_valid);
    }
}

int UnityMenuAction::index() const
{
    return m_index;
}

void UnityMenuAction::setIndex(int i)
{
    if (i != m_index) {
        m_index = i;
        Q_EMIT indexChanged(m_index);
    }
}

void UnityMenuAction::registerAction()
{
    if (m_model) {
        m_model->registerAction(this);
    }
}

void UnityMenuAction::unregisterAction()
{
    if (m_model) {
        m_model->unregisterAction(this);
    }
}

bool UnityMenuAction::event(QEvent* e)
{
    if (e->type() == UnityMenuActionAddEvent::eventType) {
        UnityMenuActionAddEvent *umaae = static_cast<UnityMenuActionAddEvent*>(e);

        setEnabled(umaae->enabled);
        setState(umaae->state);
        setValid(true);
        return true;
    } else if (e->type() == UnityMenuActionEnabledChangedEvent::eventType) {
        UnityMenuActionEnabledChangedEvent *umaece = static_cast<UnityMenuActionEnabledChangedEvent*>(e);

        setEnabled(umaece->enabled);
        return true;
    } else if (e->type() == UnityMenuActionStateChangeEvent::eventType) {
        UnityMenuActionStateChangeEvent *umasce = static_cast<UnityMenuActionStateChangeEvent*>(e);

        setState(umasce->state);
        return true;
    } else if (e->type() == UnityMenuActionRemoveEvent::eventType) {
        UnityMenuActionRemoveEvent *umare = static_cast<UnityMenuActionRemoveEvent*>(e);

        setValid(false);
        return true;
    }
    return QObject::event(e);
}
