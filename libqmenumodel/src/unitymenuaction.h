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
 *      Nick Dedekind <nick.dedekind@canonical.com>
 */

#ifndef UNITYMENUACTION_H
#define UNITYMENUACTION_H

#include <QObject>
#include <QVariant>

typedef struct _GVariant GVariant;
class UnityMenuModel;

class UnityMenuAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant name READ name NOTIFY nameChanged)
    Q_PROPERTY(QVariant state READ state WRITE changeState NOTIFY stateChanged)
    Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexChanged)
    Q_PROPERTY(UnityMenuModel* model READ model WRITE setModel NOTIFY modelChanged)
public:
    UnityMenuAction(QObject* parent=0);

    int index() const;
    void setIndex(int index);

    UnityMenuModel* model() const;
    void setModel(UnityMenuModel* model);

    virtual QString name() const = 0;

    virtual QVariant state() const = 0;

    Q_INVOKABLE virtual void activate(const QVariant &parameter) = 0;

    Q_INVOKABLE virtual void changeState(const QVariant& vvalue) = 0;

Q_SIGNALS:
    void nameChanged();
    void stateChanged();
    void indexChanged(int index);
    void modelChanged(UnityMenuModel* model);

private:
    UnityMenuModel* m_model;
    int m_index;
};

Q_DECLARE_METATYPE(UnityMenuAction*)

#endif // UNITYMENUACTION_H