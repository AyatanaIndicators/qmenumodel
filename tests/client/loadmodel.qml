/*
 * Copyright 2012 Canonical Ltd.
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
 *      Renato Araujo Oliveira Filho <renato@canonical.com>
 */

import QtQuick 2.0
import QMenuModel 1.0

Item {
    id: root
    width: 100
    height: 100

    property bool reset: resetModel

    onResetChanged: {
        if (reset) {
            // destroy the current model and check if it will not crash the QML engine
            view.model.destroy();
        }
    }

    ListView {
        id: view
        anchors.fill: parent
        delegate: Text {
            text: label
        }
    }

    Component.onCompleted: {
        // dynamically create the model to destroy it later
        var model = Qt.createQmlObject("import QMenuModel 1.0; QDBusMenuModel { id: menuModel; busType: globalBusType; busName: globalBusName; objectPath: globalObjectPath; }", view, "");
        model.start();
        view.model = model;
    }
}
