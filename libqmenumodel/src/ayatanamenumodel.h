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

#ifndef AYATANAMENUMODEL_H
#define AYATANAMENUMODEL_H

#include <QAbstractListModel>
class ActionStateParser;
class QQmlComponent;
class AyatanaMenuAction;

class AyatanaMenuModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QByteArray busName READ busName WRITE setBusName NOTIFY busNameChanged)
    Q_PROPERTY(QByteArray nameOwner READ nameOwner NOTIFY nameOwnerChanged)
    Q_PROPERTY(QVariantMap actions READ actions WRITE setActions NOTIFY actionsChanged)
    Q_PROPERTY(QByteArray menuObjectPath READ menuObjectPath WRITE setMenuObjectPath NOTIFY menuObjectPathChanged)
    Q_PROPERTY(ActionStateParser* actionStateParser READ actionStateParser WRITE setActionStateParser NOTIFY actionStateParserChanged)

public:
    AyatanaMenuModel(QObject *parent = NULL);
    virtual ~AyatanaMenuModel();

    QByteArray busName() const;
    void setBusName(const QByteArray &name);

    QByteArray nameOwner() const;

    QVariantMap actions() const;
    void setActions(const QVariantMap &actions);

    QByteArray menuObjectPath() const;
    void setMenuObjectPath(const QByteArray &path);

    ActionStateParser* actionStateParser() const;
    void setActionStateParser(ActionStateParser* actionStateParser);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE QObject * submenu(int position, QQmlComponent* actionStateParser = NULL);
    Q_INVOKABLE bool loadExtendedAttributes(int position, const QVariantMap &schema);
    Q_INVOKABLE QVariant get(int row, const QByteArray &role);

    Q_INVOKABLE void activate(int index, const QVariant& parameter = QVariant());
    Q_INVOKABLE void aboutToShow(int index);
    Q_INVOKABLE void activateByVariantString(int index, const QString& parameter = QString());
    Q_INVOKABLE void changeState(int index, const QVariant& parameter);
    Q_INVOKABLE void changeStateByVariantString(int index, const QString& parameter);

    void registerAction(AyatanaMenuAction* action);
    void unregisterAction(AyatanaMenuAction* action);

Q_SIGNALS:
    void busNameChanged(const QByteArray &name);
    void nameOwnerChanged(const QByteArray &owner);
    void actionsChanged(const QByteArray &path);
    void menuObjectPathChanged(const QByteArray &path);
    void actionStateParserChanged(ActionStateParser* parser);

protected Q_SLOTS:
    void onRegisteredActionNameChanged(const QString& name);
    void onRegisteredActionIndexChanged(int);
    void onRegisteredActionActivated(const QVariant& parameter);
    void onRegisteredActionStateChanged(const QVariant& parameter);

protected:
    virtual bool event(QEvent* e);

private:
    class AyatanaMenuModelPrivate *priv;
    friend class AyatanaMenuModelPrivate;

    AyatanaMenuModel(const AyatanaMenuModelPrivate& other, AyatanaMenuModel *parent);
};

#endif
