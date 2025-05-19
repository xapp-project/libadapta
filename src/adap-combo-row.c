/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * Heavily based on GTK 3.99.3 GtkDropDown widget initially written by Matthias
 * Clasen, and heavily modified for libadwaita by Alice Mikhaylenko on
 * behalf of Purism SPC 2020.
 */

#include "config.h"
#include "adw-combo-row.h"

/**
 * AdwComboRow:
 *
 * A [class@Gtk.ListBoxRow] used to choose from a list of items.
 *
 * <picture>
 *   <source srcset="combo-row-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="combo-row.png" alt="combo-row">
 * </picture>
 *
 * The `AdwComboRow` widget allows the user to choose from a list of valid
 * choices. The row displays the selected choice. When activated, the row
 * displays a popover which allows the user to make a new choice.
 *
 * Example of an `AdwComboRow` UI definition:
 * ```xml
 * <object class="AdwComboRow">
 *   <property name="title" translatable="yes">Combo Row</property>
 *   <property name="model">
 *     <object class="GtkStringList">
 *       <items>
 *         <item translatable="yes">Foo</item>
 *         <item translatable="yes">Bar</item>
 *         <item translatable="yes">Baz</item>
 *       </items>
 *     </object>
 *   </property>
 * </object>
 * ```
 * 
 * The [property@ComboRow:selected] and [property@ComboRow:selected-item]
 * properties can be used to keep track of the selected item and react to their
 * changes.
 *
 * `AdwComboRow` mirrors [class@Gtk.DropDown], see that widget for details.
 *
 * `AdwComboRow` is [property@Gtk.ListBoxRow:activatable] if a model is set.
 *
 * ## CSS nodes
 *
 * `AdwComboRow` has a main CSS node with name `row` and the `.combo` style
 * class.
 *
 * Its popover has the node named `popover` with the `.menu` style class, it
 * contains a [class@Gtk.ScrolledWindow], which in turn contains a
 * [class@Gtk.ListView], both are accessible via their regular nodes.
 *
 * ## Accessibility
 *
 * `AdwComboRow` uses the `GTK_ACCESSIBLE_ROLE_COMBO_BOX` role.
 */

/*
 * This was mostly inspired by code from the display panel from GNOME Settings.
 */

typedef struct
{
  GtkWidget *arrow_box;
  GtkListView *current;
  GtkListView *list;
  GtkPopover *popover;
  GtkSearchEntry *search_entry;
  gboolean use_subtitle;
  gboolean enable_search;

  gboolean uses_default_factory;
  GtkListItemFactory *factory;
  GtkListItemFactory *list_factory;
  GListModel *model;
  GListModel *filter_model;
  GtkSelectionModel *selection;
  GtkSelectionModel *popup_selection;
  GtkSelectionModel *current_selection;

  GtkExpression *expression;
} AdwComboRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwComboRow, adw_combo_row, ADW_TYPE_ACTION_ROW)

enum {
  PROP_0,
  PROP_SELECTED,
  PROP_SELECTED_ITEM,
  PROP_MODEL,
  PROP_FACTORY,
  PROP_LIST_FACTORY,
  PROP_EXPRESSION,
  PROP_USE_SUBTITLE,
  PROP_ENABLE_SEARCH,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static char *
get_item_representation (AdwComboRow *self,
                         gpointer     item)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  GValue value = G_VALUE_INIT;

  if (priv->expression &&
      gtk_expression_evaluate (priv->expression, item, &value)) {
    char *ret = g_value_dup_string (&value);

    g_value_unset (&value);

    return ret;
  } else if (GTK_IS_STRING_OBJECT (item)) {
    return g_strdup (gtk_string_object_get_string (GTK_STRING_OBJECT (item)));
  }

  return NULL;
}

static void
selection_changed (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  guint selected;
  GtkFilter *filter;

  if (!GTK_IS_SINGLE_SELECTION (priv->selection))
    return;

  selected = gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (priv->selection));

  /* reset the filter so positions are 1-1 */
  filter = gtk_filter_list_model_get_filter (GTK_FILTER_LIST_MODEL (priv->filter_model));
  if (GTK_IS_STRING_FILTER (filter))
    gtk_string_filter_set_search (GTK_STRING_FILTER (filter), "");
  gtk_single_selection_set_selected (GTK_SINGLE_SELECTION (priv->popup_selection), selected);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED]);
}

static void
selection_item_changed (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  if (priv->use_subtitle) {
    if (g_list_model_get_n_items (G_LIST_MODEL (priv->current_selection)) > 0) {
      GtkListItem *item = g_list_model_get_item (G_LIST_MODEL (priv->current_selection), 0);
      char *repr = get_item_representation (self, item);

      adw_action_row_set_subtitle (ADW_ACTION_ROW (self), repr);

      g_free (repr);
      g_object_unref (item);
    } else {
      adw_action_row_set_subtitle (ADW_ACTION_ROW (self), NULL);
    }
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_ITEM]);
}

static void
model_changed (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  guint n_items = priv->model ? g_list_model_get_n_items (priv->model) : 0;

  gtk_widget_set_sensitive (GTK_WIDGET (self), n_items > 0);
  gtk_widget_set_visible (priv->arrow_box, n_items > 1);
  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (self), n_items > 1);
}

static void
row_activated_cb (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  GtkFilter *filter;

  gtk_popover_popdown (GTK_POPOVER (priv->popover));

  /* reset the filter so positions are 1-1 */
  filter = gtk_filter_list_model_get_filter (GTK_FILTER_LIST_MODEL (priv->filter_model));
  if (GTK_IS_STRING_FILTER (filter))
    gtk_string_filter_set_search (GTK_STRING_FILTER (filter), "");
  adw_combo_row_set_selected (self, gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (priv->popup_selection)));
}

static void
notify_popover_visible_cb (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  if (gtk_widget_get_visible (GTK_WIDGET (priv->popover))) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "has-open-popup");
  } else {
    gtk_widget_remove_css_class (GTK_WIDGET (self), "has-open-popup");
    gtk_editable_set_text (GTK_EDITABLE (priv->search_entry), "");
  }
}

static void
update_filter (AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  
  if (priv->filter_model) {
      GtkFilter *filter;

      if (priv->expression) 
        filter = GTK_FILTER (gtk_string_filter_new (gtk_expression_ref (priv->expression)));
      else 
        filter = GTK_FILTER (gtk_every_filter_new ());
      gtk_filter_list_model_set_filter (GTK_FILTER_LIST_MODEL (priv->filter_model), filter);
      g_object_unref (filter);
  }
}

static void
search_changed_cb (GtkSearchEntry *entry,
                   AdwComboRow    *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  const char *text;
  GtkFilter *filter;

  text = gtk_editable_get_text (GTK_EDITABLE (entry));

  filter = gtk_filter_list_model_get_filter (GTK_FILTER_LIST_MODEL (priv->filter_model));
  if (GTK_IS_STRING_FILTER (filter))
    gtk_string_filter_set_search (GTK_STRING_FILTER (filter), text);
}

static void
search_stop_cb (GtkSearchEntry *entry,
                AdwComboRow    *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  GtkFilter *filter;

  filter = gtk_filter_list_model_get_filter (GTK_FILTER_LIST_MODEL (priv->filter_model));
  if (GTK_IS_STRING_FILTER (filter)) {
    if (gtk_string_filter_get_search (GTK_STRING_FILTER (filter)))
      gtk_string_filter_set_search (GTK_STRING_FILTER (filter), NULL);
    else
      gtk_popover_popdown (GTK_POPOVER (priv->popover));
  }
}

static void
adw_combo_row_activate (AdwActionRow *row)
{
  AdwComboRow *self = ADW_COMBO_ROW (row);
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  if (gtk_widget_get_visible (priv->arrow_box))
    gtk_popover_popup (priv->popover);
}

static void
setup_item (GtkSignalListItemFactory *factory,
            GtkListItem              *list_item,
            AdwComboRow              *self)
{
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *icon;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  label = gtk_label_new (NULL);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars (GTK_LABEL (label), 20);
  gtk_label_set_width_chars (GTK_LABEL (label), 1);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (box), label);

  icon = g_object_new (GTK_TYPE_IMAGE,
                       "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                       "icon-name", "object-select-symbolic",
                       NULL);
  gtk_box_append (GTK_BOX (box), icon);

  gtk_list_item_set_child (list_item, box);
}

static void
selected_item_changed (AdwComboRow *self,
                       GParamSpec  *pspec,
                       GtkListItem *list_item)
{
  GtkWidget *box;
  GtkWidget *icon;

  box = gtk_list_item_get_child (list_item);
  icon = gtk_widget_get_last_child (box);

  if (adw_combo_row_get_selected_item (self) == gtk_list_item_get_item (list_item))
    gtk_widget_set_opacity (icon, 1.0);
  else
    gtk_widget_set_opacity (icon, 0.0);
}

static void
root_changed (GtkWidget   *box,
              GParamSpec  *pspec,
              AdwComboRow *self)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);
  GtkWidget *icon;

  icon = gtk_widget_get_last_child (box);

  if (gtk_widget_get_ancestor (box, GTK_TYPE_POPOVER) == GTK_WIDGET (priv->popover))
    gtk_widget_set_visible (icon, TRUE);
  else
    gtk_widget_set_visible (icon, FALSE);
}

static void
bind_item (GtkSignalListItemFactory *factory,
           GtkListItem              *list_item,
           AdwComboRow              *self)
{
  gpointer item;
  GtkWidget *box;
  char *repr;

  item = gtk_list_item_get_item (list_item);
  box = gtk_list_item_get_child (list_item);

  repr = get_item_representation (self, item);

  if (repr) {
    GtkWidget *label = gtk_widget_get_first_child (box);

    gtk_label_set_label (GTK_LABEL (label), repr);
  } else {
    g_critical ("Either AdwComboRow:factory or AdwComboRow:expression must be set");
  }

  g_signal_connect (self, "notify::selected-item",
                    G_CALLBACK (selected_item_changed), list_item);
  selected_item_changed (self, NULL, list_item);

  g_signal_connect (box, "notify::root",
                    G_CALLBACK (root_changed), self);
  root_changed (box, NULL, self);

  g_clear_pointer (&repr, g_free);
}

static void
unbind_item (GtkSignalListItemFactory *factory,
             GtkListItem              *list_item,
             AdwComboRow              *self)
{
  GtkWidget *box;

  box = gtk_list_item_get_child (list_item);

  g_signal_handlers_disconnect_by_func (self, selected_item_changed, list_item);
  g_signal_handlers_disconnect_by_func (box, root_changed, self);
}

static void
set_factory (AdwComboRow        *self,
             GtkListItemFactory *factory,
             gboolean            is_default)
{
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  if (!g_set_object (&priv->factory, factory))
    return;

  gtk_list_view_set_factory (priv->current, factory);

  priv->uses_default_factory = is_default;

  if (priv->list_factory == NULL)
    gtk_list_view_set_factory (priv->list, factory);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FACTORY]);
}

static void
set_default_factory (AdwComboRow *self)
{
  GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();

  g_signal_connect (factory, "setup", G_CALLBACK (setup_item), self);
  g_signal_connect (factory, "bind", G_CALLBACK (bind_item), self);
  g_signal_connect (factory, "unbind", G_CALLBACK (unbind_item), self);

  set_factory (self, factory, TRUE);

  g_object_unref (factory);
}

static void
adw_combo_row_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  AdwComboRow *self = ADW_COMBO_ROW (object);

  switch (prop_id) {
  case PROP_SELECTED:
    g_value_set_uint (value, adw_combo_row_get_selected (self));
    break;
  case PROP_SELECTED_ITEM:
    g_value_set_object (value, adw_combo_row_get_selected_item (self));
    break;
  case PROP_MODEL:
    g_value_set_object (value, adw_combo_row_get_model (self));
    break;
  case PROP_FACTORY:
    g_value_set_object (value, adw_combo_row_get_factory (self));
    break;
  case PROP_LIST_FACTORY:
    g_value_set_object (value, adw_combo_row_get_list_factory (self));
    break;
  case PROP_EXPRESSION:
    gtk_value_set_expression (value, adw_combo_row_get_expression (self));
    break;
  case PROP_USE_SUBTITLE:
    g_value_set_boolean (value, adw_combo_row_get_use_subtitle (self));
    break;
  case PROP_ENABLE_SEARCH:
    g_value_set_boolean (value, adw_combo_row_get_enable_search (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_combo_row_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  AdwComboRow *self = ADW_COMBO_ROW (object);

  switch (prop_id) {
  case PROP_SELECTED:
    adw_combo_row_set_selected (self, g_value_get_uint (value));
    break;
  case PROP_MODEL:
    adw_combo_row_set_model (self, g_value_get_object (value));
    break;
  case PROP_FACTORY:
    adw_combo_row_set_factory (self, g_value_get_object (value));
    break;
  case PROP_LIST_FACTORY:
    adw_combo_row_set_list_factory (self, g_value_get_object (value));
    break;
  case PROP_EXPRESSION:
    adw_combo_row_set_expression (self, gtk_value_get_expression (value));
    break;
  case PROP_USE_SUBTITLE:
    adw_combo_row_set_use_subtitle (self, g_value_get_boolean (value));
    break;
  case PROP_ENABLE_SEARCH:
    adw_combo_row_set_enable_search (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_combo_row_dispose (GObject *object)
{
  AdwComboRow *self = ADW_COMBO_ROW (object);
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  gtk_list_view_set_model (priv->list, NULL);
  gtk_list_view_set_model (priv->current, NULL);

  if (priv->selection) {
    g_signal_handlers_disconnect_by_func (priv->selection, selection_changed, self);
    g_signal_handlers_disconnect_by_func (priv->selection, selection_item_changed, self);
    g_signal_handlers_disconnect_by_func (priv->selection, model_changed, self);
  }

  g_clear_pointer (&priv->expression, gtk_expression_unref);
  g_clear_object (&priv->selection);
  g_clear_object (&priv->popup_selection);
  g_clear_object (&priv->current_selection);
  g_clear_object (&priv->factory);
  g_clear_object (&priv->list_factory);
  g_clear_object (&priv->filter_model);

  g_clear_object (&priv->model);

  G_OBJECT_CLASS (adw_combo_row_parent_class)->dispose (object);
}

static void
adw_combo_row_size_allocate (GtkWidget *widget,
                             int        width,
                             int        height,
                             int        baseline)
{
  AdwComboRow *self = ADW_COMBO_ROW (widget);
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  GTK_WIDGET_CLASS (adw_combo_row_parent_class)->size_allocate (widget, width, height, baseline);

  gtk_popover_present (priv->popover);
}

static gboolean
adw_combo_row_focus (GtkWidget        *widget,
                     GtkDirectionType  direction)
{
  AdwComboRow *self = ADW_COMBO_ROW (widget);
  AdwComboRowPrivate *priv = adw_combo_row_get_instance_private (self);

  if (priv->popover && gtk_widget_get_visible (GTK_WIDGET (priv->popover)))
    return gtk_widget_child_focus (GTK_WIDGET (priv->popover), direction);
  else
    return GTK_WIDGET_CLASS (adw_combo_row_parent_class)->focus (widget, direction);
}

static void
adw_combo_row_class_init (AdwComboRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  AdwActionRowClass *row_class = ADW_ACTION_ROW_CLASS (klass);

  object_class->get_property = adw_combo_row_get_property;
  object_class->set_property = adw_combo_row_set_property;
  object_class->dispose = adw_combo_row_dispose;

  widget_class->size_allocate = adw_combo_row_size_allocate;
  widget_class->focus = adw_combo_row_focus;

  row_class->activate = adw_combo_row_activate;

  /**
   * AdwComboRow:selected: (attributes org.gtk.Property.get=adw_combo_row_get_selected org.gtk.Property.set=adw_combo_row_set_selected)
   *
   * The position of the selected item.
   *
   * If no item is selected, the property has the value
   * [const@Gtk.INVALID_LIST_POSITION]
   */
  props[PROP_SELECTED] =
    g_param_spec_uint ("selected", NULL, NULL,
                       0, G_MAXUINT, GTK_INVALID_LIST_POSITION,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:selected-item: (attributes org.gtk.Property.get=adw_combo_row_get_selected_item)
   *
   * The selected item.
   */
  props[PROP_SELECTED_ITEM] =
    g_param_spec_object ("selected-item", NULL, NULL,
                         G_TYPE_OBJECT,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwComboRow:model: (attributes org.gtk.Property.get=adw_combo_row_get_model org.gtk.Property.set=adw_combo_row_set_model)
   *
   * The model that provides the displayed items.
   */
  props[PROP_MODEL] =
    g_param_spec_object ("model", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:factory: (attributes org.gtk.Property.get=adw_combo_row_get_factory org.gtk.Property.set=adw_combo_row_set_factory)
   *
   * Factory for populating list items.
   *
   * This factory is always used for the item in the row. It is also used for
   * items in the popup unless [property@ComboRow:list-factory] is set.
   */
  props[PROP_FACTORY] =
    g_param_spec_object ("factory", NULL, NULL,
                         GTK_TYPE_LIST_ITEM_FACTORY,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:list-factory: (attributes org.gtk.Property.get=adw_combo_row_get_list_factory org.gtk.Property.set=adw_combo_row_set_list_factory)
   *
   * The factory for populating list items in the popup.
   *
   * If this is not set, [property@ComboRow:factory] is used.
   */
  props[PROP_LIST_FACTORY] =
    g_param_spec_object ("list-factory", NULL, NULL,
                         GTK_TYPE_LIST_ITEM_FACTORY,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:expression: (type GtkExpression) (attributes org.gtk.Property.get=adw_combo_row_get_expression org.gtk.Property.set=adw_combo_row_set_expression)
   *
   * An expression used to obtain strings from items.
   *
   * The expression must have a value type of `G_TYPE_STRING`.
   *
   * It's used to bind strings to labels produced by the default factory if
   * [property@ComboRow:factory] is not set, or when
   * [property@ComboRow:use-subtitle] is set to `TRUE`.
   */
  props[PROP_EXPRESSION] =
    gtk_param_spec_expression ("expression",
                               "Expression",
                               "Expression to determine strings to search for",
                               G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:use-subtitle: (attributes org.gtk.Property.get=adw_combo_row_get_use_subtitle org.gtk.Property.set=adw_combo_row_set_use_subtitle)
   *
   * Whether to use the current value as the subtitle.
   *
   * If you use a custom list item factory, you will need to give the row a
   * name conversion expression with [property@ComboRow:expression].
   *
   * If set to `TRUE`, you should not access [property@ActionRow:subtitle].
   *
   * The subtitle is interpreted as Pango markup if
   * [property@PreferencesRow:use-markup] is set to `TRUE`.
   */
  props[PROP_USE_SUBTITLE] =
    g_param_spec_boolean ("use-subtitle", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwComboRow:enable-search: (attributes org.gtk.Property.get=adw_combo_row_get_enable_search org.gtk.Property.set=adw_combo_row_set_enable_search)
   *
   * Whether to show a search entry in the popup.
   *
   * If set to `TRUE`, a search entry will be shown in the popup that
   * allows to search for items in the list.
   * 
   * Search requires [property@ComboRow:expression] to be set.
   * 
   * Since: 1.4
   */
  props[PROP_ENABLE_SEARCH] =
    g_param_spec_boolean ("enable-search", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-combo-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, current);
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, arrow_box);
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, list);
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, popover);
  gtk_widget_class_bind_template_child_private (widget_class, AdwComboRow, search_entry);
  gtk_widget_class_bind_template_callback (widget_class, row_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_popover_visible_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_stop_cb);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_COMBO_BOX);
}

static void
adw_combo_row_init (AdwComboRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (self), FALSE);
  set_default_factory (self);
  model_changed (self);
}

/**
 * adw_combo_row_new:
 *
 * Creates a new `AdwComboRow`.
 *
 * Returns: the newly created `AdwComboRow`
 */
GtkWidget *
adw_combo_row_new (void)
{
  return g_object_new (ADW_TYPE_COMBO_ROW, NULL);
}

/**
 * adw_combo_row_set_selected: (attributes org.gtk.Method.set_property=selected)
 * @self: a combo row
 * @position: the position of the item to select, or
 *   [const@Gtk.INVALID_LIST_POSITION]
 *
 * Selects the item at the given position.
 */
void
adw_combo_row_set_selected (AdwComboRow *self,
                            guint        position)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));

  priv = adw_combo_row_get_instance_private (self);

  if (priv->selection == NULL)
    return;

  if (gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (priv->selection)) == position)
    return;

  gtk_single_selection_set_selected (GTK_SINGLE_SELECTION (priv->selection), position);
}

/**
 * adw_combo_row_get_selected: (attributes org.gtk.Method.get_property=selected)
 * @self: a combo row
 *
 * Gets the position of the selected item.
 *
 * Returns: the position of the selected item, or
 *   [const@Gtk.INVALID_LIST_POSITION] if no item is selected
 */
guint
adw_combo_row_get_selected (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), GTK_INVALID_LIST_POSITION);

  priv = adw_combo_row_get_instance_private (self);

  if (priv->selection == NULL)
    return GTK_INVALID_LIST_POSITION;

  return gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (priv->selection));
}

/**
 * adw_combo_row_get_selected_item: (attributes org.gtk.Method.get_property=selected-item)
 * @self: a combo row
 *
 * Gets the selected item.
 *
 * Returns: (transfer none) (type GObject) (nullable): the selected item
 */
gpointer
adw_combo_row_get_selected_item (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  if (priv->selection == NULL)
    return NULL;

  return gtk_single_selection_get_selected_item (GTK_SINGLE_SELECTION (priv->selection));
}

/**
 * adw_combo_row_get_model: (attributes org.gtk.Method.get_property=model)
 * @self: a combo row
 *
 * Gets the model that provides the displayed items.
 *
 * Returns: (nullable) (transfer none): The model in use
 */
GListModel *
adw_combo_row_get_model (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  return priv->model;
}

/**
 * adw_combo_row_set_model: (attributes org.gtk.Method.set_property=model)
 * @self: a combo row
 * @model: (nullable) (transfer none): the model to use
 *
 * Sets the model that provides the displayed items.
 */
void
adw_combo_row_set_model (AdwComboRow *self,
                         GListModel  *model)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));
  g_return_if_fail (model == NULL || G_IS_LIST_MODEL (model));

  priv = adw_combo_row_get_instance_private (self);

  if (!g_set_object (&priv->model, model))
    return;

  if (model == NULL) {
    gtk_list_view_set_model (priv->list, NULL);
    gtk_list_view_set_model (priv->current, NULL);

    if (priv->selection) {
      g_signal_handlers_disconnect_by_func (priv->selection, selection_changed, self);
      g_signal_handlers_disconnect_by_func (priv->selection, selection_item_changed, self);
      g_signal_handlers_disconnect_by_func (priv->selection, model_changed, self);
    }

    g_clear_object (&priv->selection);
    g_clear_object (&priv->popup_selection);
    g_clear_object (&priv->current_selection);
  } else {
    GtkSelectionModel *selection;
    GListModel *filter_model;
    GListModel *current_model;

    filter_model = G_LIST_MODEL (gtk_filter_list_model_new (g_object_ref (model), NULL));
    g_set_object (&priv->filter_model, filter_model);

    update_filter (self);

    selection = GTK_SELECTION_MODEL (gtk_single_selection_new (g_object_ref (filter_model)));
    g_set_object (&priv->popup_selection, selection);
    gtk_list_view_set_model (priv->list, selection);
    g_object_unref (selection);

    selection = GTK_SELECTION_MODEL (gtk_single_selection_new (g_object_ref (model)));
    g_set_object (&priv->selection, selection);
    g_object_unref (selection);

    g_object_unref (filter_model);

    current_model = G_LIST_MODEL (gtk_selection_filter_model_new (priv->selection));
    selection = GTK_SELECTION_MODEL (gtk_no_selection_new (current_model));
    g_set_object (&priv->current_selection, selection);
    gtk_list_view_set_model (priv->current, selection);
    g_object_unref (selection);

    g_signal_connect_swapped (priv->selection, "notify::selected", G_CALLBACK (selection_changed), self);
    g_signal_connect_swapped (priv->selection, "notify::selected-item", G_CALLBACK (selection_item_changed), self);
    g_signal_connect_swapped (priv->selection, "items-changed", G_CALLBACK (model_changed), self);

    selection_changed (self);
    selection_item_changed (self);
    model_changed (self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODEL]);
}

/**
 * adw_combo_row_get_factory: (attributes org.gtk.Method.get_property=factory)
 * @self: a combo row
 *
 * Gets the factory for populating list items.
 *
 * Returns: (nullable) (transfer none): the factory in use
 */
GtkListItemFactory *
adw_combo_row_get_factory (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  return priv->factory;
}

/**
 * adw_combo_row_set_factory: (attributes org.gtk.Method.set_property=factory)
 * @self: a combo row
 * @factory: (nullable) (transfer none): the factory to use
 *
 * Sets the factory for populating list items.
 *
 * This factory is always used for the item in the row. It is also used for
 * items in the popup unless [property@ComboRow:list-factory] is set.
 */
void
adw_combo_row_set_factory (AdwComboRow        *self,
                           GtkListItemFactory *factory)
{
  g_return_if_fail (ADW_IS_COMBO_ROW (self));
  g_return_if_fail (factory == NULL || GTK_LIST_ITEM_FACTORY (factory));

  set_factory (self, factory, FALSE);
}

/**
 * adw_combo_row_get_list_factory: (attributes org.gtk.Method.get_property=list-factory)
 * @self: a combo row
 *
 * Gets the factory for populating list items in the popup.
 *
 * Returns: (nullable) (transfer none): the factory in use
 */
GtkListItemFactory *
adw_combo_row_get_list_factory (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  return priv->list_factory;
}

/**
 * adw_combo_row_set_list_factory: (attributes org.gtk.Method.set_property=list-factory)
 * @self: a combo row
 * @factory: (nullable) (transfer none): the factory to use
 *
 * Sets the factory for populating list items in the popup.
 *
 * If this is not set, [property@ComboRow:factory] is used.
 */
void
adw_combo_row_set_list_factory (AdwComboRow        *self,
                                GtkListItemFactory *factory)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));
  g_return_if_fail (factory == NULL || GTK_LIST_ITEM_FACTORY (factory));

  priv = adw_combo_row_get_instance_private (self);

  if (!g_set_object (&priv->list_factory, factory))
    return;

  if (priv->list_factory != NULL)
    gtk_list_view_set_factory (priv->list, priv->list_factory);
  else
    gtk_list_view_set_factory (priv->list, priv->factory);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LIST_FACTORY]);
}

/**
 * adw_combo_row_get_expression: (attributes org.gtk.Method.get_property=expression)
 * @self: a combo row
 *
 * Gets the expression used to obtain strings from items.
 *
 * Returns: (nullable) (transfer none): the expression used to obtain strings from items
 */
GtkExpression *
adw_combo_row_get_expression (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), NULL);

  priv = adw_combo_row_get_instance_private (self);

  return priv->expression;
}

/**
 * adw_combo_row_set_expression: (attributes org.gtk.Method.set_property=expression)
 * @self: a combo row
 * @expression: (nullable): an expression
 *
 * Sets the expression used to obtain strings from items.
 *
 * The expression must have a value type of `G_TYPE_STRING`.
 *
 * It's used to bind strings to labels produced by the default factory if
 * [property@ComboRow:factory] is not set, or when
 * [property@ComboRow:use-subtitle] is set to `TRUE`.
 */
void
adw_combo_row_set_expression (AdwComboRow   *self,
                              GtkExpression *expression)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));
  g_return_if_fail (expression == NULL ||
                    gtk_expression_get_value_type (expression) == G_TYPE_STRING);

  priv = adw_combo_row_get_instance_private (self);

  if (priv->expression == expression)
    return;

  if (priv->expression)
    gtk_expression_unref (priv->expression);
  priv->expression = expression;
  if (priv->expression)
    gtk_expression_ref (priv->expression);

  selection_changed (self);

  update_filter (self);

  if (priv->uses_default_factory)
    set_default_factory (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXPRESSION]);
}

/**
 * adw_combo_row_get_use_subtitle: (attributes org.gtk.Method.get_property=use-subtitle)
 * @self: a combo row
 *
 * Gets whether to use the current value as the subtitle.
 *
 * Returns: whether to use the current value as the subtitle
 */
gboolean
adw_combo_row_get_use_subtitle (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), FALSE);

  priv = adw_combo_row_get_instance_private (self);

  return priv->use_subtitle;
}

/**
 * adw_combo_row_set_use_subtitle: (attributes org.gtk.Method.set_property=use-subtitle)
 * @self: a combo row
 * @use_subtitle: whether to use the current value as the subtitle
 *
 * Sets whether to use the current value as the subtitle.
 *
 * If you use a custom list item factory, you will need to give the row a
 * name conversion expression with [property@ComboRow:expression].
 *
 * If set to `TRUE`, you should not access [property@ActionRow:subtitle].
 *
 * The subtitle is interpreted as Pango markup if
 * [property@PreferencesRow:use-markup] is set to `TRUE`.
 */
void
adw_combo_row_set_use_subtitle (AdwComboRow *self,
                                gboolean     use_subtitle)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));

  priv = adw_combo_row_get_instance_private (self);

  use_subtitle = !!use_subtitle;

  if (priv->use_subtitle == use_subtitle)
    return;

  priv->use_subtitle = use_subtitle;
  selection_changed (self);
  if (!use_subtitle)
    adw_action_row_set_subtitle (ADW_ACTION_ROW (self), NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_SUBTITLE]);
}

/**
 * adw_combo_row_get_enable_search: (attributes org.gtk.Method.get_property=enable-search)
 * @self: a combo row
 *
 * Gets whether search is enabled.
 * 
 * If set to `TRUE`, a search entry will be shown in the popup that
 * allows to search for items in the list.
 * 
 * Search requires [property@ComboRow:expression] to be set.
 *
 * Returns: whether the popup includes a search entry
 * 
 * Since: 1.4
 */
gboolean
adw_combo_row_get_enable_search (AdwComboRow *self)
{
  AdwComboRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_COMBO_ROW (self), FALSE);

  priv = adw_combo_row_get_instance_private (self);

  return priv->enable_search;
}

/**
 * adw_combo_row_set_enable_search: (attributes org.gtk.Method.set_property=enable-search)
 * @self: a combo row
 * @enable_search: whether to enable search
 *
 * Sets whether to enable search.
 * 
 * If set to `TRUE`, a search entry will be shown in the popup that
 * allows to search for items in the list.
 * 
 * Search requires [property@ComboRow:expression] to be set.
 * 
 * Since: 1.4
 */
void
adw_combo_row_set_enable_search (AdwComboRow *self,
                                 gboolean     enable_search)
{
  AdwComboRowPrivate *priv;

  g_return_if_fail (ADW_IS_COMBO_ROW (self));

  priv = adw_combo_row_get_instance_private (self);

  enable_search = !!enable_search;

  if (priv->enable_search == enable_search)
    return;

  priv->enable_search = enable_search;
  
  gtk_editable_set_text (GTK_EDITABLE (priv->search_entry), "");
  gtk_widget_set_visible (GTK_WIDGET (priv->search_entry), enable_search);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_SEARCH]);
}
