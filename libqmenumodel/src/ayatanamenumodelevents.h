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

#ifndef AYATANAMENUMODELEVENTS_H
#define AYATANAMENUMODELEVENTS_H

#include <QEvent>
#include <glib.h>

typedef struct _GtkMenuTrackerItem GtkMenuTrackerItem;

/* Event for a unitymenumodel clear */
class UnityMenuModelClearEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    UnityMenuModelClearEvent(bool reset);

    bool reset;
};

/* Event for a row add for unitymenumodel */
class UnityMenuModelAddRowEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    UnityMenuModelAddRowEvent(GPtrArray *_items, int position);
    ~UnityMenuModelAddRowEvent();

    GPtrArray *items;
    int position;
};

/* Event for a row remove for unitymenumodel */
class UnityMenuModelRemoveRowEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    UnityMenuModelRemoveRowEvent(int position, int nItems);

    int position;
    int nItems;
};

/* Event for a row data change for unitymenumodel */
class UnityMenuModelDataChangeEvent : public QEvent
{
public:
    static const QEvent::Type eventType;
    UnityMenuModelDataChangeEvent(int position);

    int position;
};

#endif //AYATANAMENUMODELEVENTS_H
