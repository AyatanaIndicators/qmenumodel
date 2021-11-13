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
 * Authors: Lars Uebernickel <lars.uebernickel@canonical.com>
 */

import QtQuick 2.0
import QMenuModel 0.1

Item {
    width: 400;
    height: 500;

    UnityMenuModel {
        id: menu
        busName: "com.canonical.indicator.sound"
        actions: { "indicator": "/com/canonical/indicator/sound" }
        menuObjectPath: "/com/canonical/indicator/sound/desktop"
    }

    ListView {
        id: listview
        anchors.fill: parent
        anchors.margins: 10
        spacing: 3
        model: menu

        delegate: Loader {
            sourceComponent: {
                if (isSeparator) {
                    return separator;
                }
                else if (type == "org.ayatana.indicator.slider") {
                    listview.model.loadExtendedAttributes(index, {'min-icon': 'icon',
                                                                  'max-icon': 'icon'});
                    return slider;
                }
                else {
                    return menuitem;
                }
            }

            Component {
                id: separator
                Rectangle {
                    width: listview.width
                    height: 4
                    color: "blue"
                }
            }

            Component {
                id: slider
                Rectangle {
                    width: listview.width
                    color: "#ddd"
                    height: 40
                    Row {
                        anchors.fill: parent
                        Image {
                            source: ext.minIcon
                        }
                        Text {
                            text: model.actionState
                        }
                        Image {
                            source: ext.maxIcon
                        }
                    }
                }
            }

            Component {
                id: menuitem
                Rectangle {
                    width: listview.width
                    height: 40
                    color: "#ddd"
                    Row {
                        anchors.fill: parent
                        anchors.margins: 5
                        Image {
                            source: icon
                        }
                        Text {
                            height: parent.height
                            verticalAlignment: Text.AlignVCenter
                            color: sensitive ? "black" : "#aaa";
                            text: label
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var submenu = listview.model.submenu(index);
                            if (submenu)
                                listview.model = submenu;
                            else
                                action.activate();
                        }
                    }
                }
            }
        }
    }
}
