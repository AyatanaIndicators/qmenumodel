/*
 * Copyright 2013 Canonical Ltd.
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
 *      Nick Dedekind <nick.dedekind@canonical.com>
 *      Robert Tari <robert@tari.in>
 */

#include "ayatanamenuaction.h"
#include "ayatanamenumodel.h"
#include "ayatanamenuactionevents.h"

AyatanaMenuAction::AyatanaMenuAction(QObject* parent)
    :   QObject(parent),
        m_valid(false),
        m_enabled(false),
        m_model(NULL),
        m_index(-1)
{
}

AyatanaMenuAction::~AyatanaMenuAction()
{
    unregisterAction();
}

QString AyatanaMenuAction::name() const
{
    return m_name;
}

void AyatanaMenuAction::setName(const QString& name)
{
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged(m_name);
    }
}

AyatanaMenuModel* AyatanaMenuAction::model() const
{
    return m_model;
}

void AyatanaMenuAction::setModel(AyatanaMenuModel* model)
{
    if (m_model != model) {
        unregisterAction();
        m_model = model;
        registerAction();
        Q_EMIT modelChanged(model);
    }
}

QVariant AyatanaMenuAction::state() const
{
    return m_state;
}

void AyatanaMenuAction::setState(const QVariant& state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged(m_state);
    }
}

bool AyatanaMenuAction::isEnabled() const
{
    return m_enabled;
}

void AyatanaMenuAction::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged(m_enabled);
    }
}

bool AyatanaMenuAction::isValid() const
{
    return m_valid;
}

void AyatanaMenuAction::setValid(bool valid)
{
    if (m_valid != valid) {
        m_valid = valid;
        Q_EMIT validChanged(m_valid);
    }
}

int AyatanaMenuAction::index() const
{
    return m_index;
}

void AyatanaMenuAction::setIndex(int i)
{
    if (i != m_index) {
        m_index = i;
        Q_EMIT indexChanged(m_index);
    }
}

void AyatanaMenuAction::registerAction()
{
    if (m_model) {
        m_model->registerAction(this);
    }
}

void AyatanaMenuAction::unregisterAction()
{
    if (m_model) {
        m_model->unregisterAction(this);
    }
}

bool AyatanaMenuAction::event(QEvent* e)
{
    if (e->type() == AyatanaMenuActionAddEvent::eventType) {
        AyatanaMenuActionAddEvent *umaae = static_cast<AyatanaMenuActionAddEvent*>(e);

        setEnabled(umaae->enabled);
        setState(umaae->state);
        setValid(true);
        return true;
    } else if (e->type() == AyatanaMenuActionEnabledChangedEvent::eventType) {
        AyatanaMenuActionEnabledChangedEvent *umaece = static_cast<AyatanaMenuActionEnabledChangedEvent*>(e);

        setEnabled(umaece->enabled);
        return true;
    } else if (e->type() == AyatanaMenuActionStateChangeEvent::eventType) {
        AyatanaMenuActionStateChangeEvent *umasce = static_cast<AyatanaMenuActionStateChangeEvent*>(e);

        setState(umasce->state);
        return true;
    } else if (e->type() == AyatanaMenuActionRemoveEvent::eventType) {
        setValid(false);
        return true;
    }
    return QObject::event(e);
}
