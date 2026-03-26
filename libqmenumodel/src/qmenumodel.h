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

#ifndef QMENUTREEMODEL_H
#define QMENUTREEMODEL_H

#include <QAbstractItemModel>

#include "menunode.h"

typedef struct _GMenuModel GMenuModel;

class QMenuModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum MenuRoles {
        Action  = Qt::DisplayRole + 1,
        Label,
        Extra,
        Depth,
        hasSection,
        hasSubMenu
    };

    ~QMenuModel();

    /* QAbstractItemModel */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QHash<int, QByteArray> roleNames() const;

Q_SIGNALS:
    void countChanged();

protected:
    QMenuModel(GMenuModel *other=0, QObject *parent=0);
    void setMenuModel(GMenuModel *model);
    GMenuModel *menuModel() const;

    virtual bool event(QEvent* e);

private:
    MenuNode *m_root;

    MenuNode* nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(MenuNode *node) const;

    QVariant getStringAttribute(MenuNode *node, int row, const QString &attribute) const;
    QVariant getExtraProperties(MenuNode *node, int row) const;
    bool hasLink(MenuNode *node, int row, const QString &linkType) const;

    QString parseExtraPropertyName(const QString &name) const;
    void clearModel();
};

#endif
