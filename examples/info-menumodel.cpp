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
 *      Olivier Tilloy <olivier.tilloy@canonical.com>
 */

/*
 * Example executable that links against libqmenumodel to monitor
 * and print information about a menu model exported on D-Bus.
 */

// QMenuModel
#include "qmenumodel/qdbusmenumodel.h"

// stdlib
#include <iostream>

// Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QObject>


#define BUS_NAME "org.ayatana.testmenu"
#define BUS_OBJECT_PATH "/org/ayatana/testmenu"


class MenuModelMonitor : public QDBusMenuModel
{
    Q_OBJECT

public:
    MenuModelMonitor(QObject* parent=0)
        : QDBusMenuModel(parent)
    {
        setProperty("busType", DBusEnums::SessionBus);
        setProperty("busName", BUS_NAME);
        setProperty("objectPath", BUS_OBJECT_PATH);
        QObject::connect(this, SIGNAL(statusChanged(DBusEnums::ConnectionStatus)),
                         SLOT(onStatusChanged(DBusEnums::ConnectionStatus)));
        QObject::connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                         SLOT(onModelChanged()));
        QObject::connect(this, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                         SLOT(onModelChanged()));
        std::cout << "Monitoring menu model "
                  << property("objectPath").toString().toUtf8().constData()
                  << " on the well-known name "
                  << property("busName").toString().toUtf8().constData()
                  << std::endl;
    }

private Q_SLOTS:
    void onStatusChanged(DBusEnums::ConnectionStatus status)
    {
        std::cout << "Status of menu model changed to " << status << std::endl;
        if (status == DBusEnums::Connected) {
            printModel(this);
        }
    }

    void onModelChanged()
    {
        printModel(this);
    }

private:
    void printModel(QMenuModel* model, int indent=0)
    {
        int count = model->rowCount();
        for (int i = 0; i < count; ++i) {
            QModelIndex index = model->index(i, 0);
            QString label = model->data(index, QMenuModel::Label).toString();
            QVariant submenu = model->data(index, QMenuModel::LinkSubMenu);
            for (int j = 0; j < indent * 2; ++j) std::cout << " ";
            std::cout << " > " << label.toUtf8().constData() << std::endl;
            if (submenu.isValid()) {
                printModel(qobject_cast<QMenuModel*>(submenu.value<QObject*>()), indent + 1);
            }
        }
    }
};

#include "info-menumodel.moc"


int main(int argc, char** argv)
{
    MenuModelMonitor monitor;
    monitor.start();
    int result = QCoreApplication(argc, argv).exec();
    monitor.stop();
    return result;
}

