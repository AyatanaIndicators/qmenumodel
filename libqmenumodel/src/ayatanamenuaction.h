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
 * Authors: Nick Dedekind <nick.dedekind@canonical.com>
 */

#ifndef AYATANAMENUACTION_H
#define AYATANAMENUACTION_H

#include <QObject>
#include <QVariant>
class UnityMenuModel;

class UnityMenuAction: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QVariant state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(UnityMenuModel* model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexChanged)

public:
    UnityMenuAction(QObject* parent = 0);
    ~UnityMenuAction();

    QString name() const;
    void setName(const QString& str);

    UnityMenuModel* model() const;
    void setModel(UnityMenuModel* model);

    int index() const;
    void setIndex(int i);

    QVariant state() const;
    bool isEnabled() const;
    bool isValid() const;

Q_SIGNALS:
    Q_INVOKABLE void activate(const QVariant& parameter = QVariant());
    Q_INVOKABLE void changeState(const QVariant& parameter);

    void nameChanged(const QString& name);
    void modelChanged(UnityMenuModel* model);
    void stateChanged(const QVariant& name);
    void enabledChanged(bool enabled);
    void validChanged(bool valid);
    void indexChanged(int index);

protected:
    virtual bool event(QEvent* e);

    void setState(const QVariant& state);
    void setEnabled(bool enabled);
    void setValid(bool valid);

private:
    void unregisterAction();
    void registerAction();

    QString m_name;
    QVariant m_state;
    bool m_valid;
    bool m_enabled;
    UnityMenuModel* m_model;
    int m_index;
};

#endif // AYATANAMENUACTIONGROUP_H
