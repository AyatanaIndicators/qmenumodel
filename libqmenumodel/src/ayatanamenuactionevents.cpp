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

#include "ayatanamenuactionevents.h"

const QEvent::Type UnityMenuActionAddEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuActionRemoveEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuActionEnabledChangedEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuActionStateChangeEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

UnityMenuActionAddEvent::UnityMenuActionAddEvent(bool _enabled, const QVariant& _state)
    : QEvent(UnityMenuActionAddEvent::eventType),
      enabled(_enabled),
      state(_state)
{}

UnityMenuActionRemoveEvent::UnityMenuActionRemoveEvent()
    : QEvent(UnityMenuActionRemoveEvent::eventType)
{
}

UnityMenuActionEnabledChangedEvent::UnityMenuActionEnabledChangedEvent(bool _enabled)
    : QEvent(UnityMenuActionEnabledChangedEvent::eventType),
      enabled(_enabled)
{}

UnityMenuActionStateChangeEvent::UnityMenuActionStateChangeEvent(const QVariant& _state)
    : QEvent(UnityMenuActionStateChangeEvent::eventType),
      state(_state)
{}
