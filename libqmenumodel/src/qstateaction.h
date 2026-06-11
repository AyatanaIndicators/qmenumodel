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

#ifndef QDBUSACTION_H
#define QDBUSACTION_H

#include <QObject>
#include <QVariant>

// cyclical includes, forward-declare
class QDBusActionGroup;
#include "qdbusactiongroup.h"

class QStateAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QVariant state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
public:
    QVariant state() const;
    bool isValid() const;

    Q_INVOKABLE void activate(const QVariant &parameter = QVariant());
    Q_INVOKABLE void activateByVariantString(const QString &parameter = QString());
    Q_INVOKABLE void updateState(const QVariant &parameter);
    Q_INVOKABLE void updateStateByVariantString(const QString &parameter);

Q_SIGNALS:
    void stateChanged(QVariant state);
    void validChanged(bool valid);

private Q_SLOTS:
    void onActionAppear(const QString &name);
    void onActionVanish(const QString &name);
    void onActionStateChanged(const QString &name, const QVariant &state);

private:
    QDBusActionGroup *m_group;
    QVariant m_state;
    bool m_valid;
    QString m_name;

    QStateAction(QDBusActionGroup *group, const QString &name);

    void setValid(bool valid);
    void setState(const QVariant &state);
    QString name() const;

    friend class QDBusActionGroup;
};

#endif
