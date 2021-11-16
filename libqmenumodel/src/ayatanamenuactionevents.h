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

/* Event for a ayatanamenuaction add */
class AyatanaMenuActionAddEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    AyatanaMenuActionAddEvent(bool enabled, const QVariant& state);

    bool enabled;
    QVariant state;
};

/* Event for a ayatanamenuaction remove */
class AyatanaMenuActionRemoveEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    AyatanaMenuActionRemoveEvent();
};

/* Event for change in enabled value of a ayatanamenuaction */
class AyatanaMenuActionEnabledChangedEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    AyatanaMenuActionEnabledChangedEvent(bool enabled);

    int enabled;
};

/* Event for change in state value of a ayatanamenuaction */
class AyatanaMenuActionStateChangeEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    AyatanaMenuActionStateChangeEvent(const QVariant& state);

    QVariant state;
};

#endif //AYATANAMENUACTIONEVENTS_H
