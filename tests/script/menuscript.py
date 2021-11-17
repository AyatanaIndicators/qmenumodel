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

import dbus
import dbus.service
from dbus import glib
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GObject
from gi.repository import GLib
from gi.repository import Gio

SERVICE_NAME     = "org.ayatana.test"
INTERFACE_NAME   = "org.ayatana.test.menuscript"
OBJECT_PATH      = "/org/ayatana/test/menuscript"
MENU_SERVICE_NAME= SERVICE_NAME + ".menu"
MENU_OBJECT_PATH = OBJECT_PATH + "/menu"
bus = None

class Script(dbus.service.Object):
    def __init__(self, aList, session, object_path):
        dbus.service.Object.__init__(self, session, object_path)
        self._list = aList
        self._session  = session

    def run(self):
        self._loop = GObject.MainLoop()
        self._loop.run()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='', out_signature='',
                         sender_keyword='sender')
    def publishMenu(self, sender=None):
        self._list.start()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='', out_signature='',
                         sender_keyword='sender')
    def unpublishMenu(self, sender=None):
        self._list.stop()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='', out_signature='',
                         sender_keyword='sender')
    def quit(self, sender=None):
        self.unpublishMenu();
        self._loop.quit()

    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='i', out_signature='',
                         sender_keyword='sender')
    def walk(self, steps, sender=None):
        if steps == -1 or steps > self._list.size():
            steps = self._list.size()

        while(steps > 0):
            self._list.walk()
            steps -= 1

    """ TODO: We only support string states for now """
    @dbus.service.method(dbus_interface=INTERFACE_NAME,
                         in_signature='', out_signature='ss')
    def popActivatedAction(self):
        return self._list._activatedActions.pop(0)

    @staticmethod
    def create(aList):
        global bus

        GObject.threads_init()
        glib.threads_init()

        dbus_loop = DBusGMainLoop()
        bus = dbus.SessionBus(mainloop=dbus_loop)
        bus_name = dbus.service.BusName(SERVICE_NAME, bus=bus)
        return Script(aList, bus_name, OBJECT_PATH)

class Action(object):
    def __init__(self, aList, action, **kwargs):
        self._list = aList
        self._action = action
        self._kargs = kwargs

    def setProperties(self, menu, props):
        if props:
           for key in props:
               menu.set_attribute_value(key, props[key])

    def appendItem(self):
        parentId = self._kargs['parentId']
        if parentId and len(parentId):
            parent = self._list.getMenu(parentId)[0]
        else:
            parent = self._list._root

        if self._kargs['link'] == None:
            item = Gio.MenuItem.new(self._kargs['label'], self._kargs['actionName'])
            self.setProperties(item, self._kargs['properties'])
            parent.append_item(item)

            # Action
            act = Gio.SimpleAction.new(self._kargs['actionName'], self._kargs['actionStateType'])
            act.connect('activate', self._list._onActionActivated)
            self._list._rootAction.insert(act)

        elif self._kargs['link'] == 'section':
            section = Gio.Menu()
            parent.append_section(self._kargs['label'], section)
        elif self._kargs['link'] == 'submenu':
            submenu = Gio.Menu()
            parent.append_submenu(self._kargs['label'], submenu)

    def removeItem(self):
        menuId = self._kargs['menuId']
        (menu, mId) = self._list.getMenu(menuId)
        if mId != -1:
            menu.remove(mId)
            if self._kargs['actionName']:
                # Remove action
                self._list._rootAction.remove(self._kargs['actionName'])
        else:
            print("Remove menu item")

    def run(self):
        if self._action == 'append':
            self.appendItem()
        elif self._action == 'remove':
            self.removeItem()

class ActionList(object):
    def __init__(self, objectPath):
        self._actions = []
        self._actions_bk = []
        self._objectPath = objectPath
        self._bus = None
        self._exportMenuID = None
        self._exportActionID = None
        self._ownNameID = None
        self._root = None
        self._rootAction = None
        self._activatedActions = []

    def appendItem(self, label, actionName, link=None, parentId=None,  properties=None, actionStateType=None):
        self._actions.append(Action(self, 'append',
                                    parentId=parentId,
                                    label=label,
                                    actionName=actionName,
                                    link=link,
                                    properties=properties,
                                    actionStateType=actionStateType))

    def removeItem(self, menuId, actionName=None):
        self._actions.append(Action(self, 'remove',
                                    menuId=menuId,
                                    actionName=actionName))

    def _save(self):
        self._actions_bk = []
        self._actions_bk.extend(self._actions)


    def _restore(self):
        if len(self._actions_bk):
            self._actions = []
            self._actions.extend(self._actions_bk)

    def _findMenu(self, root, ids):
        if len(ids) == 0:
            return (root, -1)

        currentId = int(ids[0])
        link = root.get_item_link(currentId, 'section')
        if link == None:
            link = root.get_item_link(currentId, 'submenu')

        if link:
            return self._findMenu(link, ids[1:])
        else:
            return (root, currentId)

    def getMenu(self, menuId):
        return self._findMenu(self._root, str(menuId).split('.'));

    def walk(self):
        item = self._actions.pop(0)
        item.run()

    def size(self):
        return len(self._actions)

    def _exportService(self, connection, name):
        self._root = Gio.Menu()
        self._rootAction = Gio.SimpleActionGroup()
        self._bus = connection
        self._exportMenuID = connection.export_menu_model(MENU_OBJECT_PATH, self._root)
        self._exportActionID = connection.export_action_group(MENU_OBJECT_PATH, self._rootAction)

    def start(self):
        self._save()
        self._ownNameID = Gio.bus_own_name(2, MENU_SERVICE_NAME, 0, self._exportService, None, None)

    def stop(self):
        if self._exportMenuID:
            self._bus.unexport_menu_model(self._exportMenuID)
            self._exportMenuID = None

        if self._exportActionID:
            self._bus.unexport_action_group(self._exportActionID)
            self._exportActionID = None

        if self._ownNameID:
            Gio.bus_unown_name(self._ownNameID)
            self._ownNameID = None

        self._root = None
        self._rootAction = None
        self._restore()

    def _onActionActivated(self, action, parameter):
        self._activatedActions.append((action.get_name(), parameter.get_string()))
