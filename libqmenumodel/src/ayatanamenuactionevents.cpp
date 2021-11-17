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

const QEvent::Type AyatanaMenuActionAddEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type AyatanaMenuActionRemoveEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type AyatanaMenuActionEnabledChangedEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type AyatanaMenuActionStateChangeEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

AyatanaMenuActionAddEvent::AyatanaMenuActionAddEvent(bool _enabled, const QVariant& _state)
    : QEvent(AyatanaMenuActionAddEvent::eventType),
      enabled(_enabled),
      state(_state)
{}

AyatanaMenuActionRemoveEvent::AyatanaMenuActionRemoveEvent()
    : QEvent(AyatanaMenuActionRemoveEvent::eventType)
{
}

AyatanaMenuActionEnabledChangedEvent::AyatanaMenuActionEnabledChangedEvent(bool _enabled)
    : QEvent(AyatanaMenuActionEnabledChangedEvent::eventType),
      enabled(_enabled)
{}

AyatanaMenuActionStateChangeEvent::AyatanaMenuActionStateChangeEvent(const QVariant& _state)
    : QEvent(AyatanaMenuActionStateChangeEvent::eventType),
      state(_state)
{}
