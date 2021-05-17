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

#include <glib-object.h>
#include <gio/gio.h>

#include "unitymenumodelevents.h"
#include "unitymenumodel.h"

const QEvent::Type UnityMenuModelClearEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuModelAddRowEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuModelRemoveRowEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type UnityMenuModelDataChangeEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

UnityMenuModelClearEvent::UnityMenuModelClearEvent(bool _reset)
    : QEvent(UnityMenuModelClearEvent::eventType),
      reset(_reset)
{}

UnityMenuModelAddRowEvent::UnityMenuModelAddRowEvent(GPtrArray *_items, int _position)
    : QEvent(UnityMenuModelAddRowEvent::eventType),
      items(_items),
      position(_position)
{
    if (items) {
        for (gint i = 0; i < items->len; ++i)
            g_object_ref(g_ptr_array_index(items, i));
        g_ptr_array_ref(items);
    }
}

UnityMenuModelAddRowEvent::~UnityMenuModelAddRowEvent()
{
    if (items) {
        for (gint i = 0; i < items->len; ++i)
            g_object_unref(g_ptr_array_index(items, i));
        g_ptr_array_unref(items);
    }
}

UnityMenuModelRemoveRowEvent::UnityMenuModelRemoveRowEvent(int _position, int _nItems)
    : QEvent(UnityMenuModelRemoveRowEvent::eventType),
      position(_position), nItems(_nItems)
{}

UnityMenuModelDataChangeEvent::UnityMenuModelDataChangeEvent(int _position)
    : QEvent(UnityMenuModelDataChangeEvent::eventType),
      position(_position)
{}
