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

#include "ayatanamenumodel.h"
#include "converter.h"
#include "actionstateparser.h"
#include "ayatanamenumodelevents.h"
#include "ayatanamenuaction.h"
#include "ayatanamenuactionevents.h"
#include "logging.h"

#include <QIcon>
#include <QQmlComponent>
#include <QCoreApplication>
#include <QKeySequence>

extern "C" {
  #include "gtk/gtkactionmuxer.h"
  #include "gtk/gtkmenutracker.h"
  #include "gtk/gtksimpleactionobserver.h"
}

Q_LOGGING_CATEGORY(ayatanamenumodel, "qmenumodel.ayatanamenumodel", QtCriticalMsg)

G_DEFINE_QUARK (AYATANA_MENU_MODEL, ayatana_menu_model)
G_DEFINE_QUARK (AYATANA_SUBMENU_MODEL, ayatana_submenu_model)
G_DEFINE_QUARK (AYATANA_MENU_ITEM_EXTENDED_ATTRIBUTES, ayatana_menu_item_extended_attributes)
G_DEFINE_QUARK (AYATANA_MENU_ACTION, ayatana_menu_action)


enum MenuRoles {
    LabelRole  = Qt::DisplayRole + 1,
    SensitiveRole,
    IsSeparatorRole,
    IconRole,
    TypeRole,
    ExtendedAttributesRole,
    ActionRole,
    ActionStateRole,
    IsCheckRole,
    IsRadioRole,
    IsToggledRole,
    ShortcutRole,
    HasSubmenuRole
};

class AyatanaMenuModelPrivate
{
public:
    AyatanaMenuModelPrivate(AyatanaMenuModel *model);
    AyatanaMenuModelPrivate(const AyatanaMenuModelPrivate& other, AyatanaMenuModel *model);
    ~AyatanaMenuModelPrivate();

    void clearItems(bool resetModel=true);
    void clearName();
    void updateActions();
    void updateMenuModel();
    QVariant itemState(GtkMenuTrackerItem *item);

    AyatanaMenuModel *model;
    GtkActionMuxer *muxer;
    GtkMenuTracker *menutracker;
    GSequence *items;
    GDBusConnection *connection;
    QByteArray busName;
    QByteArray nameOwner;
    guint nameWatchId;
    QVariantMap actions;
    QByteArray menuObjectPath;
    QHash<QByteArray, int> roles;
    ActionStateParser* actionStateParser;
    QHash<AyatanaMenuAction*, GtkSimpleActionObserver*> registeredActions;
    bool destructorGuard;

    static void nameAppeared(GDBusConnection *connection, const gchar *name, const gchar *owner, gpointer user_data);
    static void nameVanished(GDBusConnection *connection, const gchar *name, gpointer user_data);
    static void menuItemInserted(GPtrArray *items, gint position, gpointer user_data);
    static void menuItemRemoved(gint position, gint n_items, gpointer user_data);
    static void menuItemChanged(GObject *object, GParamSpec *pspec, gpointer user_data);

    static void registeredActionAdded(GtkSimpleActionObserver    *observer_item,
                                      const gchar          *action_name,
                                      gboolean              enabled,
                                      GVariant             *state);
    static void registeredActionEnabledChanged(GtkSimpleActionObserver *observer_item,  const gchar *action_name, gboolean enabled);
    static void registeredActionStateChanged(GtkSimpleActionObserver *observer_item, const gchar *action_name, GVariant *state);
    static void registeredActionRemoved(GtkSimpleActionObserver *observer_item, const gchar *action_name);

    gchar * fullActionName(AyatanaMenuAction *action);
    void updateRegisteredAction(AyatanaMenuAction *action);
};

void menu_item_free (gpointer data)
{
    GtkMenuTrackerItem *item = (GtkMenuTrackerItem *) data;

    g_signal_handlers_disconnect_by_func (item, (gpointer) AyatanaMenuModelPrivate::menuItemChanged, NULL);
    g_object_unref (item);
}

AyatanaMenuModelPrivate::AyatanaMenuModelPrivate(AyatanaMenuModel *model)
{
    this->model = model;
    this->menutracker = NULL;
    this->connection = NULL;
    this->nameWatchId = 0;
    this->actionStateParser = new ActionStateParser(model);
    this->destructorGuard = false;

    this->muxer = gtk_action_muxer_new ();

    this->items = g_sequence_new (menu_item_free);
}

AyatanaMenuModelPrivate::AyatanaMenuModelPrivate(const AyatanaMenuModelPrivate& other, AyatanaMenuModel *model)
{
    this->model = model;
    this->menutracker = NULL;
    this->connection = NULL;
    this->nameWatchId = 0;
    this->actionStateParser = new ActionStateParser(model);
    this->destructorGuard = false;

    this->muxer = GTK_ACTION_MUXER( g_object_ref(other.muxer));

    this->items = g_sequence_new (menu_item_free);
}

AyatanaMenuModelPrivate::~AyatanaMenuModelPrivate()
{
    this->destructorGuard = true;
    this->clearItems(false);

    g_sequence_free(this->items);
    g_clear_pointer (&this->menutracker, gtk_menu_tracker_free);
    g_clear_object (&this->muxer);
    g_clear_object (&this->connection);

    QHash<AyatanaMenuAction*, GtkSimpleActionObserver*>::const_iterator it = this->registeredActions.constBegin();
    for (; it != this->registeredActions.constEnd(); ++it) {
        g_object_unref(it.value());
        it.key()->setModel(NULL);
    }
    this->registeredActions.clear();

    if (this->nameWatchId)
        g_bus_unwatch_name (this->nameWatchId);
}

void AyatanaMenuModelPrivate::clearItems(bool resetModel)
{
    AyatanaMenuModelClearEvent ummce(resetModel);
    QCoreApplication::sendEvent(model, &ummce);
}

void AyatanaMenuModelPrivate::clearName()
{
    this->clearItems();

    this->nameOwner = QByteArray();

    this->updateActions();
    this->updateMenuModel();

    Q_EMIT model->nameOwnerChanged (this->nameOwner);
}

void AyatanaMenuModelPrivate::updateActions()
{
    Q_FOREACH (QString prefix, this->actions.keys())
        gtk_action_muxer_remove (this->muxer, prefix.toUtf8());

    if (this->nameOwner.isEmpty())
      return;

    for (QVariantMap::const_iterator it = this->actions.constBegin(); it != this->actions.constEnd(); ++it) {
        GDBusActionGroup *actions;

        actions = g_dbus_action_group_get (this->connection, this->nameOwner, it.value().toByteArray());
        gtk_action_muxer_insert (this->muxer, it.key().toUtf8(), G_ACTION_GROUP (actions));

        g_object_unref (actions);
    }
}

void AyatanaMenuModelPrivate::updateMenuModel()
{
    this->clearItems();
    g_clear_pointer (&this->menutracker, gtk_menu_tracker_free);

    if (!this->nameOwner.isEmpty()) {
        GDBusMenuModel *menu;

        menu = g_dbus_menu_model_get (this->connection, this->nameOwner, this->menuObjectPath.constData());
        this->menutracker = gtk_menu_tracker_new (GTK_ACTION_OBSERVABLE (this->muxer),
                                                  G_MENU_MODEL (menu), TRUE, NULL,
                                                  menuItemInserted, menuItemRemoved, this);

        g_object_unref (menu);
    }
}

QVariant AyatanaMenuModelPrivate::itemState(GtkMenuTrackerItem *item)
{
    QVariant result;

    GVariant *state = gtk_menu_tracker_item_get_action_state (item);
    if (state != NULL) {
        if (actionStateParser != NULL) {
            result = actionStateParser->toQVariant(state);
        }
        g_variant_unref (state);
    }

    return result;
}

void AyatanaMenuModelPrivate::nameAppeared(GDBusConnection *connection, const gchar *name, const gchar *owner, gpointer user_data)
{
    AyatanaMenuModelPrivate *priv = (AyatanaMenuModelPrivate *)user_data;

    priv->connection = (GDBusConnection *) g_object_ref (connection);
    priv->nameOwner = owner;

    priv->updateActions();
    priv->updateMenuModel();

    Q_EMIT priv->model->nameOwnerChanged (priv->nameOwner);
}

void AyatanaMenuModelPrivate::nameVanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    AyatanaMenuModelPrivate *priv = (AyatanaMenuModelPrivate *)user_data;

    priv->clearName();
}

void AyatanaMenuModelPrivate::menuItemInserted(GPtrArray *items, gint position, gpointer user_data)
{
    AyatanaMenuModelPrivate *priv = (AyatanaMenuModelPrivate *)user_data;

    AyatanaMenuModelAddRowEvent ummare(items, position);
    QCoreApplication::sendEvent(priv->model, &ummare);
}

void AyatanaMenuModelPrivate::menuItemRemoved(gint position, gint n_items, gpointer user_data)
{
    AyatanaMenuModelPrivate *priv = (AyatanaMenuModelPrivate *)user_data;

    AyatanaMenuModelRemoveRowEvent ummrre(position, n_items);
    QCoreApplication::sendEvent(priv->model, &ummrre);
}

void AyatanaMenuModelPrivate::menuItemChanged(GObject *object, GParamSpec *pspec, gpointer user_data)
{
    GSequenceIter *it = (GSequenceIter *) user_data;
    GtkMenuTrackerItem *item;
    GtkActionObservable *muxer;
    AyatanaMenuModel *model;
    gint position;

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    muxer = _gtk_menu_tracker_item_get_observable (item);
    model = (AyatanaMenuModel *) g_object_get_qdata (G_OBJECT (item), ayatana_menu_model_quark ());
    position = g_sequence_iter_get_position (it);

    AyatanaMenuModelDataChangeEvent ummdce(position);
    QCoreApplication::sendEvent(model, &ummdce);
}

AyatanaMenuModel::AyatanaMenuModel(QObject *parent):
    QAbstractListModel(parent)
{
    priv = new AyatanaMenuModelPrivate(this);
}

AyatanaMenuModel::AyatanaMenuModel(const AyatanaMenuModelPrivate& other, AyatanaMenuModel *parent):
    QAbstractListModel(parent)
{
    priv = new AyatanaMenuModelPrivate(other, this);
}

AyatanaMenuModel::~AyatanaMenuModel()
{
    delete priv;
}

QByteArray AyatanaMenuModel::busName() const
{
    return priv->busName;
}

QByteArray AyatanaMenuModel::nameOwner() const
{
    return priv->nameOwner;
}

void AyatanaMenuModel::setBusName(const QByteArray &name)
{
    if (name == priv->busName)
        return;

    priv->clearName();

    if (priv->nameWatchId)
        g_bus_unwatch_name (priv->nameWatchId);

    priv->nameWatchId = g_bus_watch_name (G_BUS_TYPE_SESSION, name.constData(), G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                          AyatanaMenuModelPrivate::nameAppeared, AyatanaMenuModelPrivate::nameVanished,
                                          priv, NULL);
    priv->busName = name;
    Q_EMIT busNameChanged (priv->busName);
}

QVariantMap AyatanaMenuModel::actions() const
{
    return priv->actions;
}

void AyatanaMenuModel::setActions(const QVariantMap &actions)
{
    priv->actions = actions;
    priv->updateActions();
}

QByteArray AyatanaMenuModel::menuObjectPath() const
{
    return priv->menuObjectPath;
}

void AyatanaMenuModel::setMenuObjectPath(const QByteArray &path)
{
    priv->menuObjectPath = path;
    priv->updateMenuModel();
}

ActionStateParser* AyatanaMenuModel::actionStateParser() const
{
    return priv->actionStateParser;
}

void AyatanaMenuModel::setActionStateParser(ActionStateParser* actionStateParser)
{
    if (priv->actionStateParser != actionStateParser) {
        priv->actionStateParser = actionStateParser;
        Q_EMIT actionStateParserChanged(actionStateParser);
    }
}

int AyatanaMenuModel::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? g_sequence_get_length (priv->items) : 0;
}

int AyatanaMenuModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

static QString iconUri(GIcon *icon)
{
    QString uri;

    if (G_IS_THEMED_ICON (icon)) {
        const gchar* const* iconNames = g_themed_icon_get_names (G_THEMED_ICON (icon));
        guint index = 0;
        while(iconNames[index] != NULL) {
            if (QIcon::hasThemeIcon(iconNames[index])) {
                uri = QString("image://theme/") + iconNames[index];
                break;
            }
            index++;
        }
    }
    else if (G_IS_FILE_ICON (icon)) {
        GFile *file;

        file = g_file_icon_get_file (G_FILE_ICON (icon));
        if (file) {
            gchar *fileuri;

            fileuri = g_file_get_uri (file);
            uri = QString(fileuri);

            g_free (fileuri);
        }
    }
    else if (G_IS_BYTES_ICON (icon)) {
        gsize size;
        gconstpointer data;
        gchar *base64;

        data = g_bytes_get_data (g_bytes_icon_get_bytes (G_BYTES_ICON (icon)), &size);
        base64 = g_base64_encode ((const guchar *) data, size);

        uri = QString("data://");
        uri.append (base64);

        g_free (base64);
    }

    return uri;
}

QVariant AyatanaMenuModel::data(const QModelIndex &index, int role) const
{
    GSequenceIter *it;
    GtkMenuTrackerItem *item;

    it = g_sequence_get_iter_at_pos (priv->items, index.row());
    if (g_sequence_iter_is_end (it)) {
        return QVariant();
    }

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    if (!item) {
        return QVariant();
    }

    switch (role) {
        case LabelRole:
            return gtk_menu_tracker_item_get_label (item);

        case SensitiveRole:
            return gtk_menu_tracker_item_get_sensitive (item) == TRUE ? true : false;

        case IsSeparatorRole:
            return gtk_menu_tracker_item_get_is_separator (item) == TRUE ? true : false;

        case IconRole: {
            GIcon *icon = gtk_menu_tracker_item_get_icon (item);
            if (icon) {
                QString uri = iconUri(icon);
                g_object_unref (icon);
                return uri;
            }
            else
                return QString();
        }

        case TypeRole: {
            gchar *type;
            auto ret = gtk_menu_tracker_item_get_attribute (item, "x-ayatana-type", "s", &type);

            // If we can't get x-ayatana-type, try legacy x-canonical-type type
            if (!ret)
                ret = gtk_menu_tracker_item_get_attribute (item, "x-canonical-type", "s", &type);

            if (ret) {
                QVariant v(type);
                g_free (type);
                return v;
            }
            else
                return QVariant();
        }

        case ExtendedAttributesRole: {
            QVariantMap *map = (QVariantMap *) g_object_get_qdata (G_OBJECT (item), ayatana_menu_item_extended_attributes_quark ());
            return map ? *map : QVariant();
        }

        case ActionRole: {
            gchar *action_name = gtk_menu_tracker_item_get_action_name (item);
            QString v(action_name);
            g_free(action_name);
            return v;
        }

        case ActionStateRole:
            return priv->itemState(item);

        case IsCheckRole:
            return gtk_menu_tracker_item_get_role (item) == GTK_MENU_TRACKER_ITEM_ROLE_CHECK;

        case IsRadioRole:
            return gtk_menu_tracker_item_get_role (item) == GTK_MENU_TRACKER_ITEM_ROLE_RADIO;

        case IsToggledRole:
            return gtk_menu_tracker_item_get_toggled (item) != FALSE;

        case ShortcutRole:
            return QKeySequence(gtk_menu_tracker_item_get_accel (item), QKeySequence::NativeText);

        case HasSubmenuRole:
            return gtk_menu_tracker_item_get_has_submenu (item) != FALSE;

        default:
            return QVariant();
    }
}

QModelIndex AyatanaMenuModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column);
}

QModelIndex AyatanaMenuModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

QHash<int, QByteArray> AyatanaMenuModel::roleNames() const
{
    QHash<int, QByteArray> names;

    names[LabelRole] = "label";
    names[SensitiveRole] = "sensitive";
    names[IsSeparatorRole] = "isSeparator";
    names[IconRole] = "icon";
    names[TypeRole] = "type";
    names[ExtendedAttributesRole] = "ext";
    names[ActionRole] = "action";
    names[ActionStateRole] = "actionState";
    names[IsCheckRole] = "isCheck";
    names[IsRadioRole] = "isRadio";
    names[IsToggledRole] = "isToggled";
    names[ShortcutRole] = "shortcut";
    names[HasSubmenuRole] = "hasSubmenu";

    return names;
}

QObject * AyatanaMenuModel::submenu(int position, QQmlComponent* actionStateParser)
{
    GSequenceIter *it;
    GtkMenuTrackerItem *item;
    AyatanaMenuModel *model;

    it = g_sequence_get_iter_at_pos (priv->items, position);
    if (g_sequence_iter_is_end (it)) {
        return NULL;
    }

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    if (!item || !gtk_menu_tracker_item_get_has_submenu (item)) {
        return NULL;
    }

    model = (AyatanaMenuModel *) g_object_get_qdata (G_OBJECT (item), ayatana_submenu_model_quark ());
    if (model == NULL) {
        model = new AyatanaMenuModel(*priv, this);

        if (actionStateParser) {
            ActionStateParser* parser = qobject_cast<ActionStateParser*>(actionStateParser->create());
            if (parser) {
                model->setActionStateParser(parser);
            }
        }

        model->priv->menutracker = gtk_menu_tracker_new_for_item_submenu (item,
                                                                          AyatanaMenuModelPrivate::menuItemInserted,
                                                                          AyatanaMenuModelPrivate::menuItemRemoved,
                                                                          model->priv);
        g_object_set_qdata (G_OBJECT (item), ayatana_submenu_model_quark (), model);
    }

    return model;
}

static void freeExtendedAttrs(gpointer data)
{
    QVariantMap *extendedAttrs = (QVariantMap *) data;
    delete extendedAttrs;
}

static QVariant attributeToQVariant(GVariant *value, const QString &type)
{
    QVariant result;

    if (type == "int") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_INT32)) {
            result =  QVariant(g_variant_get_int32(value));
        }
    }
    else if (type == "int64") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_INT64)) {
            result =  QVariant((qlonglong)g_variant_get_int64(value));
        }
    }
    else if (type == "bool") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_BOOLEAN)) {
            result = QVariant(g_variant_get_boolean(value));
        }
    }
    else if (type == "string") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_STRING)) {
            result = QVariant(g_variant_get_string(value, NULL));
        }
    }
    else if (type == "double") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_DOUBLE)) {
            result = QVariant(g_variant_get_double(value));
        }
    }
    else if (type == "variant") {
        if (g_variant_is_of_type (value, G_VARIANT_TYPE_VARIANT)) {
            result = Converter::toQVariant(value);
        }
    }
    else if (type == "icon") {
        GIcon *icon = g_icon_deserialize (value);
        if (icon) {
            result = iconUri(icon);
            g_object_unref (icon);
        }
        else {
            result = QVariant("");
        }
    }

    return result;
}

/* convert 'some-key' to 'someKey' or 'SomeKey'. (from dconf-qt) */
static QString qtify_name(const char *name)
{
    bool next_cap = false;
    QString result;

    while (*name) {
        if (*name == '-') {
            next_cap = true;
        } else if (next_cap) {
            result.append(toupper(*name));
            next_cap = false;
        } else {
            result.append(*name);
        }

        name++;
    }

    return result;
}

bool AyatanaMenuModel::loadExtendedAttributes(int position, const QVariantMap &schema)
{
    GSequenceIter *it;
    GtkMenuTrackerItem *item;
    QVariantMap *extendedAttrs;

    it = g_sequence_get_iter_at_pos (priv->items, position);
    if (g_sequence_iter_is_end (it)) {
        return false;
    }

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    if (!item) {
        return false;
    }

    extendedAttrs = new QVariantMap;

    for (QVariantMap::const_iterator it = schema.constBegin(); it != schema.constEnd(); ++it) {
        QString name = it.key();
        QString type = it.value().toString();

        GVariant *value = gtk_menu_tracker_item_get_attribute_value (item, name.toUtf8(), NULL);
        if (value == NULL) {
            qCWarning(ayatanamenumodel, "loadExtendedAttributes: menu item does not contain '%s'", it.key().toUtf8().constData());
            continue;
        }

        const QVariant &qvalue = attributeToQVariant(value, type);
        if (qvalue.isValid())
            extendedAttrs->insert(qtify_name (name.toUtf8()), qvalue);
        else
            qCWarning(ayatanamenumodel, "loadExtendedAttributes: key '%s' is of type '%s' (expected '%s')",
                     name.toUtf8().constData(), g_variant_get_type_string(value), type.toUtf8().constData());

        g_variant_unref (value);
    }

    g_object_set_qdata_full (G_OBJECT (item), ayatana_menu_item_extended_attributes_quark (),
                             extendedAttrs, freeExtendedAttrs);

    Q_EMIT dataChanged(index(position, 0), index(position, 0), QVector<int>() << ExtendedAttributesRole);
    return true;
}

QVariant AyatanaMenuModel::get(int row, const QByteArray &role)
{
    if (priv->roles.isEmpty()) {
        QHash<int, QByteArray> names = roleNames();
        Q_FOREACH (int role, names.keys())
            priv->roles.insert(names[role], role);
    }

    return this->data(this->index(row, 0), priv->roles[role]);
}

void AyatanaMenuModel::activate(int index, const QVariant& parameter)
{
    GSequenceIter *it;
    GtkMenuTrackerItem *item;
    GVariant *value;
    const GVariantType *parameter_type;

    it = g_sequence_get_iter_at_pos (priv->items, index);
    if (g_sequence_iter_is_end (it)) {
        return;
    }

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    if (!item) {
        return;
    }

    if (parameter.isValid()) {
        gchar *action;

        action = gtk_menu_tracker_item_get_action_name (item);
        parameter_type = g_action_group_get_action_parameter_type (G_ACTION_GROUP (priv->muxer), action);
        value = Converter::toGVariantWithSchema(parameter, g_variant_type_peek_string (parameter_type));
        g_action_group_activate_action (G_ACTION_GROUP (priv->muxer), action, value);

        g_free (action);
    } else {
        gtk_menu_tracker_item_activated (item);
    }
}

void AyatanaMenuModel::aboutToShow(int index)
{
    GSequenceIter *it = g_sequence_get_iter_at_pos (priv->items, index);
    if (g_sequence_iter_is_end (it)) {
        return;
    }

    auto item = static_cast<GtkMenuTrackerItem*>(g_sequence_get(it));
    if (!item) {
        return;
    }

    quint64 actionTag;
    if (gtk_menu_tracker_item_get_attribute (item, "qtubuntu-tag", "t", &actionTag)) {
        // Child AyatanaMenuModel have priv->connection null, so climb to the parent until we find a non null one
        AyatanaMenuModelPrivate *privToUse = priv;
        while (privToUse && !privToUse->connection) {
            auto pModel = dynamic_cast<AyatanaMenuModel*>(privToUse->model->QObject::parent());
            if (pModel) {
                privToUse = pModel->priv;
            } else {
                privToUse = nullptr;
            }
        }
        if (privToUse) {
            g_dbus_connection_call (privToUse->connection,
                                    privToUse->busName,
                                    privToUse->menuObjectPath,
                                    "qtubuntu.actions.extra",
                                    "aboutToShow",
                                    g_variant_new("(t)", actionTag),
                                    nullptr,
                                    G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                    G_MAXINT,
                                    nullptr,
                                    nullptr,
                                    nullptr);
        }
    }
}

void AyatanaMenuModel::activateByVariantString(int index, const QString& parameter)
{
    activate(index, Converter::toQVariantFromVariantString(parameter));
}

void AyatanaMenuModel::changeStateByVariantString(int index, const QString& parameter)
{
    changeState(index, Converter::toQVariantFromVariantString(parameter));
}

void AyatanaMenuModel::changeState(int index, const QVariant& parameter)
{
    GSequenceIter *it;
    GtkMenuTrackerItem* item;
    GVariant* data;
    GVariant* current_state;

    it = g_sequence_get_iter_at_pos (priv->items, index);
    if (g_sequence_iter_is_end (it)) {
        return;
    }

    item = (GtkMenuTrackerItem *) g_sequence_get (it);
    if (!item) {
        return;
    }

    current_state = gtk_menu_tracker_item_get_action_state (item);
    if (current_state) {
        // Attempt to convert the parameter to the expected type
        data = Converter::toGVariantWithSchema(parameter, g_variant_get_type_string(current_state));
        g_variant_unref (current_state);
    } else {
        data = Converter::toGVariant(parameter);
    }

    gtk_menu_tracker_item_change_state (item, data);
    if (data) {
        g_variant_unref(data);
    }
}


bool AyatanaMenuModel::event(QEvent* e)
{
    if (e->type() == AyatanaMenuModelClearEvent::eventType) {
        AyatanaMenuModelClearEvent *emmce = static_cast<AyatanaMenuModelClearEvent*>(e);

        GSequenceIter *begin;
        GSequenceIter *end;

        if (emmce->reset)
            beginResetModel();

        begin = g_sequence_get_begin_iter (priv->items);
        end = g_sequence_get_end_iter (priv->items);
        g_sequence_remove_range (begin, end);

        if (emmce->reset)
            endResetModel();

        return true;
    } else if (e->type() == AyatanaMenuModelAddRowEvent::eventType) {
        AyatanaMenuModelAddRowEvent *ummrce = static_cast<AyatanaMenuModelAddRowEvent*>(e);

        GSequenceIter *it;
        it = g_sequence_get_iter_at_pos (priv->items, ummrce->position);

        beginInsertRows(QModelIndex(), ummrce->position, ummrce->position + ummrce->items->len - 1);

        for (gint i = ummrce->items->len - 1; i >= 0; --i) {
            GtkMenuTrackerItem *item = (GtkMenuTrackerItem*)g_ptr_array_index(ummrce->items, i);
            it = g_sequence_insert_before (it, g_object_ref (item));
            g_object_set_qdata (G_OBJECT (item), ayatana_menu_model_quark (), this);
            g_signal_connect (item, "notify", G_CALLBACK (AyatanaMenuModelPrivate::menuItemChanged), it);
        }

        endInsertRows();
        return true;
    } else if (e->type() == AyatanaMenuModelRemoveRowEvent::eventType) {
        AyatanaMenuModelRemoveRowEvent *ummrre = static_cast<AyatanaMenuModelRemoveRowEvent*>(e);

        beginRemoveRows(QModelIndex(), ummrre->position, ummrre->position + ummrre->nItems - 1);
        for (int i = 0; i < ummrre->nItems; ++i) {
            GSequenceIter *it = g_sequence_get_iter_at_pos (priv->items, ummrre->position);
            if (!g_sequence_iter_is_end (it)) {
                g_sequence_remove (it);
            }
        }
        endRemoveRows();

        return true;
    } else if (e->type() == AyatanaMenuModelDataChangeEvent::eventType) {
        AyatanaMenuModelDataChangeEvent *ummdce = static_cast<AyatanaMenuModelDataChangeEvent*>(e);

        Q_EMIT dataChanged(index(ummdce->position, 0), index(ummdce->position, 0));
        return true;
    }
    return QAbstractListModel::event(e);
}

void AyatanaMenuModel::registerAction(AyatanaMenuAction* action)
{
    if (priv->destructorGuard)
        return;

    if (!priv->registeredActions.contains(action)) {
        GtkSimpleActionObserver* observer_item;
        observer_item = gtk_simple_action_observer_new(GTK_ACTION_OBSERVABLE (priv->muxer),
                                                     AyatanaMenuModelPrivate::registeredActionAdded,
                                                     AyatanaMenuModelPrivate::registeredActionEnabledChanged,
                                                     AyatanaMenuModelPrivate::registeredActionStateChanged,
                                                     AyatanaMenuModelPrivate::registeredActionRemoved);

        g_object_set_qdata (G_OBJECT (observer_item), ayatana_menu_action_quark (), action);

        priv->registeredActions[action] = observer_item;

        connect(action, SIGNAL(nameChanged(const QString&)), SLOT(onRegisteredActionNameChanged(const QString&)));
        connect(action, SIGNAL(indexChanged(int)), SLOT(onRegisteredActionIndexChanged(int)));
        connect(action, SIGNAL(activate(const QVariant&)), SLOT(onRegisteredActionActivated(const QVariant&)));
        connect(action, SIGNAL(changeState(const QVariant&)), SLOT(onRegisteredActionStateChanged(const QVariant&)));
    }
}

void AyatanaMenuModel::unregisterAction(AyatanaMenuAction* action)
{
    if (priv->destructorGuard)
        return;

    if (priv->registeredActions.contains(action)) {
        GtkSimpleActionObserver* observer_item;
        observer_item = priv->registeredActions[action];
        g_object_unref(observer_item);
        priv->registeredActions.remove(action);

        disconnect(action);
    }
}

/* Returns the full name for @action
 *
 * If @action is associated with a menu item that is inside of a
 * section or submenu with "action-namespace" set, this namespace
 * is prepended to @action->name()
 */
char * AyatanaMenuModelPrivate::fullActionName(AyatanaMenuAction *action)
{
    GSequenceIter *iter;
    QByteArray bytes;
    const gchar *name;
    gchar *full_name = NULL;

    bytes = action->name().toUtf8();
    name = bytes.constData();

    iter = g_sequence_get_iter_at_pos (this->items, action->index());
    if (!g_sequence_iter_is_end (iter)) {
        GtkMenuTrackerItem *item;
        const gchar *action_namespace;

        item = (GtkMenuTrackerItem *) g_sequence_get (iter);
        if (!item) {
            return g_strdup (name);
        }

        action_namespace = gtk_menu_tracker_item_get_action_namespace (item);
        if (action_namespace != NULL)
          return g_strjoin (".", action_namespace, name, NULL);
    }

    return g_strdup (name);
}

void AyatanaMenuModelPrivate::updateRegisteredAction(AyatanaMenuAction *action)
{
    GtkSimpleActionObserver *observer_item;
    gchar *action_name;
    gboolean enabled;
    GVariant *state;

    if (!action || !this->registeredActions.contains(action))
        return;

    action_name = fullActionName(action);

    observer_item = this->registeredActions[action];
    gtk_simple_action_observer_register_action (observer_item, action_name);

    if (g_action_group_query_action (G_ACTION_GROUP (this->muxer), action_name,
                                     &enabled, NULL, NULL, NULL, &state))
    {
        AyatanaMenuActionAddEvent umaae(enabled, Converter::toQVariant(state));
        QCoreApplication::sendEvent(action, &umaae);

        if (state) {
            g_variant_unref (state);
        }
    }

    g_free(action_name);
}

void AyatanaMenuModel::onRegisteredActionNameChanged(const QString& name)
{
    priv->updateRegisteredAction(qobject_cast<AyatanaMenuAction*>(sender()));
}

void AyatanaMenuModel::onRegisteredActionIndexChanged(int index)
{
    priv->updateRegisteredAction(qobject_cast<AyatanaMenuAction*>(sender()));
}

void AyatanaMenuModel::onRegisteredActionActivated(const QVariant& parameter)
{
    AyatanaMenuAction* action = qobject_cast<AyatanaMenuAction*>(sender());
    if (!action || action->name().isEmpty())
        return;

    gchar* action_name = priv->fullActionName(action);

    g_action_group_activate_action (G_ACTION_GROUP (priv->muxer), action_name, Converter::toGVariant(parameter));

    g_free(action_name);
}

void AyatanaMenuModel::onRegisteredActionStateChanged(const QVariant& parameter)
{
    AyatanaMenuAction* action = qobject_cast<AyatanaMenuAction*>(sender());
    if (!action || action->name().isEmpty())
        return;

    gchar* action_name = priv->fullActionName(action);

    g_action_group_change_action_state (G_ACTION_GROUP (priv->muxer), action_name, Converter::toGVariant(parameter));

    g_free(action_name);
}

void AyatanaMenuModelPrivate::registeredActionAdded(GtkSimpleActionObserver    *observer_item,
                                  const gchar          *action_name,
                                  gboolean              enabled,
                                  GVariant             *state)
{
    AyatanaMenuAction *action;
    action = (AyatanaMenuAction *) g_object_get_qdata (G_OBJECT (observer_item), ayatana_menu_action_quark ());

    if (action) {
        AyatanaMenuActionAddEvent umaae(enabled, Converter::toQVariant(state));
        QCoreApplication::sendEvent(action, &umaae);
    }
}

void AyatanaMenuModelPrivate::registeredActionEnabledChanged(GtkSimpleActionObserver *observer_item,  const gchar *action_name, gboolean enabled)
{
    AyatanaMenuAction *action;
    action = (AyatanaMenuAction *) g_object_get_qdata (G_OBJECT (observer_item), ayatana_menu_action_quark ());

    if (action) {
        AyatanaMenuActionEnabledChangedEvent umaece(enabled);
        QCoreApplication::sendEvent(action, &umaece);
    }
}

void AyatanaMenuModelPrivate::registeredActionStateChanged(GtkSimpleActionObserver *observer_item, const gchar *action_name, GVariant *state)
{
    AyatanaMenuAction *action;
    action = (AyatanaMenuAction *) g_object_get_qdata (G_OBJECT (observer_item), ayatana_menu_action_quark ());

    if (action) {
        AyatanaMenuActionStateChangeEvent umasce(Converter::toQVariant(state));
        QCoreApplication::sendEvent(action, &umasce);
    }
}

void AyatanaMenuModelPrivate::registeredActionRemoved(GtkSimpleActionObserver *observer_item, const gchar *action_name)
{
    AyatanaMenuAction *action;
    action = (AyatanaMenuAction *) g_object_get_qdata (G_OBJECT (observer_item), ayatana_menu_action_quark ());

    if (action) {
        AyatanaMenuActionRemoveEvent umare;
        QCoreApplication::sendEvent(action, &umare);
    }
}
