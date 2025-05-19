/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-enum-list-model.h"

#include <gio/gio.h>
#include <gtk/gtk.h>

/**
 * AdwEnumListModel:
 *
 * A [iface@Gio.ListModel] representing values of a given enum.
 *
 * `AdwEnumListModel` contains objects of type [class@EnumListItem].
 */

struct _AdwEnumListModel
{
  GObject parent_instance;

  GType enum_type;
  GEnumClass *enum_class;

  AdwEnumListItem **objects;
};

enum {
  PROP_0,
  PROP_ENUM_TYPE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void adw_enum_list_model_list_model_init (GListModelInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwEnumListModel, adw_enum_list_model, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_enum_list_model_list_model_init))

/**
 * AdwEnumListItem:
 *
 * `AdwEnumListItem` is the type of items in a [class@EnumListModel].
 */

struct _AdwEnumListItem
{
  GObject parent_instance;

  GEnumValue enum_value;
};

enum {
  VALUE_PROP_0,
  VALUE_PROP_VALUE,
  VALUE_PROP_NAME,
  VALUE_PROP_NICK,
  LAST_VALUE_PROP,
};

static GParamSpec *value_props[LAST_VALUE_PROP];

G_DEFINE_FINAL_TYPE (AdwEnumListItem, adw_enum_list_item, G_TYPE_OBJECT)

static void
adw_enum_list_item_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwEnumListItem *self = ADW_ENUM_LIST_ITEM (object);

  switch (prop_id) {
  case VALUE_PROP_VALUE:
    g_value_set_int (value, adw_enum_list_item_get_value (self));
    break;
  case VALUE_PROP_NAME:
    g_value_set_string (value, adw_enum_list_item_get_name (self));
    break;
  case VALUE_PROP_NICK:
    g_value_set_string (value, adw_enum_list_item_get_nick (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_enum_list_item_class_init (AdwEnumListItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = adw_enum_list_item_get_property;

  /**
   * AdwEnumListItem:value: (attributes org.gtk.Property.get=adw_enum_list_item_get_value)
   *
   * The enum value.
   */
  value_props[VALUE_PROP_VALUE] =
    g_param_spec_int ("value", NULL, NULL,
                      G_MININT, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwEnumListItem:name: (attributes org.gtk.Property.get=adw_enum_list_item_get_name)
   *
   * The enum value name.
   */
  value_props[VALUE_PROP_NAME] =
    g_param_spec_string ("name", NULL, NULL,
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwEnumListItem:nick: (attributes org.gtk.Property.get=adw_enum_list_item_get_nick)
   *
   * The enum value nick.
   */
  value_props[VALUE_PROP_NICK] =
    g_param_spec_string ("nick", NULL, NULL,
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_VALUE_PROP, value_props);
}

static void
adw_enum_list_item_init (AdwEnumListItem *self)
{
}

static AdwEnumListItem *
adw_enum_list_item_new (GEnumValue *enum_value)
{
  AdwEnumListItem *self = g_object_new (ADW_TYPE_ENUM_LIST_ITEM, NULL);

  self->enum_value = *enum_value;

  return self;
}

/**
 * adw_enum_list_item_get_value: (attributes org.gtk.Method.get_property=value)
 *
 * Gets the enum value.
 *
 * Returns: the enum value
 */
int
adw_enum_list_item_get_value (AdwEnumListItem *self)
{
  g_return_val_if_fail (ADW_IS_ENUM_LIST_ITEM (self), 0);

  return self->enum_value.value;
}

/**
 * adw_enum_list_item_get_name: (attributes org.gtk.Method.get_property=name)
 *
 * Gets the enum value name.
 *
 * Returns: the enum value name
 */
const char *
adw_enum_list_item_get_name (AdwEnumListItem *self)
{
  g_return_val_if_fail (ADW_IS_ENUM_LIST_ITEM (self), NULL);

  return self->enum_value.value_name;
}

/**
 * adw_enum_list_item_get_nick: (attributes org.gtk.Method.get_property=nick)
 *
 * Gets the enum value nick.
 *
 * Returns: the enum value nick
 */
const char *
adw_enum_list_item_get_nick (AdwEnumListItem *self)
{
  g_return_val_if_fail (ADW_IS_ENUM_LIST_ITEM (self), NULL);

  return self->enum_value.value_nick;
}

static void
adw_enum_list_model_constructed (GObject *object)
{
  AdwEnumListModel *self = ADW_ENUM_LIST_MODEL (object);
  guint i;

  self->enum_class = g_type_class_ref (self->enum_type);

  self->objects = g_new0 (AdwEnumListItem *, self->enum_class->n_values);

  for (i = 0; i < self->enum_class->n_values; i++)
    self->objects[i] = adw_enum_list_item_new (&self->enum_class->values[i]);

  G_OBJECT_CLASS (adw_enum_list_model_parent_class)->constructed (object);
}

static void
adw_enum_list_model_finalize (GObject *object)
{
  AdwEnumListModel *self = ADW_ENUM_LIST_MODEL (object);
  guint i;

  for (i = 0; i < self->enum_class->n_values; i++)
    g_object_unref (self->objects[i]);

  g_clear_pointer (&self->enum_class, g_type_class_unref);

  g_clear_pointer (&self->objects, g_free);

  G_OBJECT_CLASS (adw_enum_list_model_parent_class)->finalize (object);
}

static void
adw_enum_list_model_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdwEnumListModel *self = ADW_ENUM_LIST_MODEL (object);

  switch (prop_id) {
  case PROP_ENUM_TYPE:
    g_value_set_gtype (value, adw_enum_list_model_get_enum_type (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_enum_list_model_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdwEnumListModel *self = ADW_ENUM_LIST_MODEL (object);

  switch (prop_id) {
  case PROP_ENUM_TYPE:
    self->enum_type = g_value_get_gtype (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_enum_list_model_class_init (AdwEnumListModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_enum_list_model_constructed;
  object_class->finalize = adw_enum_list_model_finalize;
  object_class->get_property = adw_enum_list_model_get_property;
  object_class->set_property = adw_enum_list_model_set_property;

  /**
   * AdwEnumListModel:enum-type: (attributes org.gtk.Property.get=adw_enum_list_model_get_enum_type)
   *
   * The type of the enum represented by the model.
   */
  props[PROP_ENUM_TYPE] =
    g_param_spec_gtype ("enum-type", NULL, NULL,
                        G_TYPE_ENUM,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_enum_list_model_init (AdwEnumListModel *self)
{
}

static GType
adw_enum_list_model_get_item_type (GListModel *list)
{
  return ADW_TYPE_ENUM_LIST_ITEM;
}

static guint
adw_enum_list_model_get_n_items (GListModel *list)
{
  AdwEnumListModel *self = ADW_ENUM_LIST_MODEL (list);

  return self->enum_class->n_values;
}

static gpointer
adw_enum_list_model_get_item (GListModel *list,
                              guint       position)
{
  AdwEnumListModel *self = ADW_ENUM_LIST_MODEL (list);

  if (position >= self->enum_class->n_values)
    return NULL;

  return g_object_ref (self->objects[position]);
}

static void
adw_enum_list_model_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_enum_list_model_get_item_type;
  iface->get_n_items = adw_enum_list_model_get_n_items;
  iface->get_item = adw_enum_list_model_get_item;
}

/**
 * adw_enum_list_model_new:
 * @enum_type: the type of the enum to construct the model from
 *
 * Creates a new `AdwEnumListModel` for @enum_type.
 *
 * Returns: the newly created `AdwEnumListModel`
 */
AdwEnumListModel *
adw_enum_list_model_new (GType enum_type)
{
  return g_object_new (ADW_TYPE_ENUM_LIST_MODEL,
                       "enum-type", enum_type,
                       NULL);
}

/**
 * adw_enum_list_model_get_enum_type: (attributes org.gtk.Method.get_property=enum-type)
 *
 * Gets the type of the enum represented by @self.
 *
 * Returns: the enum type
 */
GType
adw_enum_list_model_get_enum_type (AdwEnumListModel *self)
{
  g_return_val_if_fail (ADW_IS_ENUM_LIST_MODEL (self), G_TYPE_INVALID);

  return self->enum_type;
}

/**
 * adw_enum_list_model_find_position:
 * @value: an enum value
 *
 * Finds the position of a given enum value in @self.
 *
 * If the value is not found, `GTK_INVALID_LIST_POSITION` is returned.
 *
 * Returns: the position of the value
 */
guint
adw_enum_list_model_find_position (AdwEnumListModel *self,
                                   int               value)
{
  guint i;

  g_return_val_if_fail (ADW_IS_ENUM_LIST_MODEL (self), 0);

  for (i = 0; i < self->enum_class->n_values; i++)
    if (self->enum_class->values[i].value == value)
      return i;

  g_critical ("%s does not contain value %d",
              G_ENUM_CLASS_TYPE_NAME (self->enum_class), value);

  return GTK_INVALID_LIST_POSITION;
}
