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

#ifndef MENUNODE_H
#define MENUNODE_H

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QVariant>

#include <gio/gio.h>

class MenuNode
{
public:
    MenuNode(const QString &linkType, GMenuModel *model, MenuNode *parent, int pos, QObject *listener);
    ~MenuNode();

    int position() const;
    MenuNode *parent() const;
    GMenuModel *model() const;
    QString linkType() const;

    void connect(QObject *listener);
    void disconnect();
    int size() const;
    MenuNode *child(int pos) const;

    void insertChild(MenuNode *child, int pos);
    int childPosition(GMenuModel *item)  const;
    int childPosition(const MenuNode *item) const;

    int depth() const;
    void change(int start, int added, int removed);
    MenuNode *find(GMenuModel *item);

    int realPosition(int row) const;
    void commitOperation();

    static MenuNode *create(GMenuModel *model, int pos, MenuNode *parent=0, QObject *listener=0);

private:
    GMenuModel *m_model;
    QMap<int, MenuNode*> m_children;
    MenuNode* m_parent;
    int m_size;
    QObject *m_listener;
    gulong m_signalChangedId;
    QString m_linkType;
    int m_currentOpPosition;
    int m_currentOpAdded;
    int m_currentOpRemoved;

    static void onItemsChanged(GMenuModel *model, gint position, gint removed, gint added, gpointer data);
};

#endif
