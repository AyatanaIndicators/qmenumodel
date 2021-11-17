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
This example script exports an action group on the session bus under the name
org.ayatana.testactiongroup and at the object path
/org/ayatana/testactiongroup.
"""

import sys

from gi.repository import Gio
from gi.repository import GLib


BUS_NAME = 'org.ayatana.testactiongroup'
BUS_OBJECT_PATH = '/org/ayatana/testactiongroup'


def action_activated(action, data):
    name = action.get_name()
    if action.get_state_type() is None:
        print('stateless action activated: %s' % name)
    else:
        print('stateful action activated: %s (current state: %s)' %
              (name, action.get_state()))


if __name__ == '__main__':
    bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
    # Claim well-known bus name and ensure only one instance of self is running
    # at any given time.
    # http://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-names
    proxy = Gio.DBusProxy.new_sync(bus, 0, None,
                                   'org.freedesktop.DBus',
                                   '/org/freedesktop/DBus',
                                   'org.freedesktop.DBus', None)
    result = proxy.RequestName('(su)', BUS_NAME, 0x4)
    if result != 1 :
        print >> sys.stderr, ("Name '%s' is already owned on the session bus."
                              "Aborting.") % BUS_NAME
        sys.exit(1)

    group = Gio.SimpleActionGroup()
    foo = Gio.SimpleAction.new('foo', None)
    group.insert(foo)
    bar = Gio.SimpleAction.new_stateful('bar', None, GLib.Variant.new_boolean(False))
    group.insert(bar)
    bleh = Gio.SimpleAction.new_stateful('bleh', None, GLib.Variant.new_string('bleh'))
    group.insert(bleh)
    for name in group.list_actions():
        action = group.lookup_action(name)
        action.connect('activate', action_activated)
    bus.export_action_group(BUS_OBJECT_PATH, group)

    GLib.MainLoop().run()

