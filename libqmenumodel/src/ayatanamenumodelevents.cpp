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

#include "ayatanamenumodelevents.h"
#include "ayatanamenumodel.h"

const QEvent::Type AyatanaMenuModelClearEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type AyatanaMenuModelAddRowEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type AyatanaMenuModelRemoveRowEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());
const QEvent::Type AyatanaMenuModelDataChangeEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

AyatanaMenuModelClearEvent::AyatanaMenuModelClearEvent(bool _reset)
    : QEvent(AyatanaMenuModelClearEvent::eventType),
      reset(_reset)
{}

AyatanaMenuModelAddRowEvent::AyatanaMenuModelAddRowEvent(GPtrArray *_items, int _position)
    : QEvent(AyatanaMenuModelAddRowEvent::eventType),
      items(_items),
      position(_position)
{
    if (items) {
        for (gint i = 0; i < items->len; ++i)
            g_object_ref(g_ptr_array_index(items, i));
        g_ptr_array_ref(items);
    }
}

AyatanaMenuModelAddRowEvent::~AyatanaMenuModelAddRowEvent()
{
    if (items) {
        for (gint i = 0; i < items->len; ++i)
            g_object_unref(g_ptr_array_index(items, i));
        g_ptr_array_unref(items);
    }
}

AyatanaMenuModelRemoveRowEvent::AyatanaMenuModelRemoveRowEvent(int _position, int _nItems)
    : QEvent(AyatanaMenuModelRemoveRowEvent::eventType),
      position(_position), nItems(_nItems)
{}

AyatanaMenuModelDataChangeEvent::AyatanaMenuModelDataChangeEvent(int _position)
    : QEvent(AyatanaMenuModelDataChangeEvent::eventType),
      position(_position)
{}
