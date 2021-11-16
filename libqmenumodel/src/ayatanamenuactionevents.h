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

#ifndef AYATANAMENUACTIONEVENTS_H
#define AYATANAMENUACTIONEVENTS_H

#include <QEvent>
#include <QVariant>

/* Event for a unitymenuaction add */
class UnityMenuActionAddEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    UnityMenuActionAddEvent(bool enabled, const QVariant& state);

    bool enabled;
    QVariant state;
};

/* Event for a unitymenuaction remove */
class UnityMenuActionRemoveEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    UnityMenuActionRemoveEvent();
};

/* Event for change in enabled value of a unitymenuaction */
class UnityMenuActionEnabledChangedEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    UnityMenuActionEnabledChangedEvent(bool enabled);

    int enabled;
};

/* Event for change in state value of a unitymenuaction */
class UnityMenuActionStateChangeEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    UnityMenuActionStateChangeEvent(const QVariant& state);

    QVariant state;
};

#endif //AYATANAMENUACTIONEVENTS_H
