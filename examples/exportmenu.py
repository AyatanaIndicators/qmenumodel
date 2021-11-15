#!/usr/bin/env python3
# -*- encoding: utf-8 -*-
#
#  Copyright 2013 Canonical Ltd.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; version 3.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

"""
This example script exports a menu model on the session bus under the name
org.ayatana.testmenu and at the object path /org/ayatana/testmenu.
The menu model contains items that have attributes with custom values, as well
as sub-menus.
"""

import sys

from gi.repository import Gio
from gi.repository import GLib


BUS_NAME = 'org.ayatana.testmenu'
BUS_OBJECT_PATH = '/org/ayatana/testmenu'

def bus_acquired(bus, name):
    menu = Gio.Menu()
    foo = Gio.MenuItem.new('foo', 'app.foo')
    foo.set_attribute_value('x-additionaltext',
                            GLib.Variant.new_string('lorem ipsum'))
    foo.set_attribute_value('x-enabled', GLib.Variant.new_boolean(True))
    menu.append_item(foo)
    bar = Gio.MenuItem.new('bar', 'bar')
    bar.set_attribute_value('x-defaultvalue',
                            GLib.Variant.new_string('Hello World!'))
    bar.set_attribute_value('x-canonical-currentvalue',
                            GLib.Variant.new_string('awesome'))
    bar.set_attribute_value('x-velocity', GLib.Variant.new_uint64(83374))
    menu.append_item(bar)
    menu.append('bleh', 'app.bleh')
    submenu = Gio.Menu()
    submenu.append('submenu A', 'app.suba')
    submenu2 = Gio.Menu()
    submenu2.append('submenu2 A', 'app.sub2a')
    submenu2.append('submenu2 B', 'app.sub2b')
    submenu2.append('submenu2 C', 'app.sub2c')
    submenu.append_submenu('submenu submenu', submenu2)
    submenu.append('submenu C', 'app.subc')
    menu.append_submenu('submenu', submenu)
    menu.append('baz', 'app.baz')
    bus.export_menu_model(BUS_OBJECT_PATH, menu)

    actions = Gio.SimpleActionGroup.new()
    actions.add_action(Gio.SimpleAction.new("bar", None))
    bus.export_action_group(BUS_OBJECT_PATH, actions)

if __name__ == '__main__':
    Gio.bus_own_name(Gio.BusType.SESSION,  BUS_NAME, 0, bus_acquired, None, None)
    GLib.MainLoop().run()
