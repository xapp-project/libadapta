/*
 * Copyright (C) 2019 Purism SPC
 * Copyright (C) 2023 GNOME Foundation Inc
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adap-preferences-dialog.h"

#include "adap-animation-util.h"
#include "adap-action-row.h"
#include "adap-breakpoint-bin.h"
#include "adap-navigation-view.h"
#include "adap-preferences-group-private.h"
#include "adap-preferences-page-private.h"
#include "adap-toast-overlay.h"
#include "adap-view-stack.h"
#include "adap-widget-utils-private.h"

#define VIEW_SWITCHER_PAGE_THRESHOLD 110
#define VIEW_SWITCHER_FALLBACK_THRESHOLD 400

/**
 * AdapPreferencesDialog:
 *
 * A dialog showing application's preferences.
 *
 * <picture>
 *   <source srcset="preferences-dialog-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="preferences-dialog.png" alt="preferences-dialog">
 * </picture>
 *
 * The `AdapPreferencesDialog` widget presents an application's preferences
 * gathered into pages and groups. The preferences are searchable by the user.
 *
 * ## Actions
 *
 * `AdapPrefencesDialog` defines the `navigation.pop` action, it doesn't take any
 * parameters and pops the current subpage from the navigation stack, equivalent
 * to calling [method@PreferencesDialog.pop_subpage].
 *
 * ## CSS nodes
 *
 * `AdapPreferencesDialog` has a main CSS node with the name `dialog` and the
 * style class `.preferences`.
 *
 * Since: 1.5
 */

typedef struct
{
  AdapToastOverlay *toast_overlay;
  AdapNavigationView *subpages_nav_view;
  GtkWidget *breakpoint_bin;
  GtkStack *content_stack;
  AdapViewStack *pages_stack;
  GtkToggleButton *search_button;
  GtkSearchEntry *search_entry;
  GtkListBox *search_results;
  GtkStack *search_stack;
  GtkStack *title_stack;
  GtkWidget *view_switcher_stack;
  GtkWidget *view_switcher;
  GtkWidget *title;
  AdapBreakpoint *breakpoint;

  gboolean search_enabled;

  GtkFilter *filter;
  GtkFilterListModel *filter_model;

  int n_pages;
} AdapPreferencesDialogPrivate;

static void adap_preferences_dialog_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapPreferencesDialog, adap_preferences_dialog, ADAP_TYPE_DIALOG,
                         G_ADD_PRIVATE (AdapPreferencesDialog)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adap_preferences_dialog_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_VISIBLE_PAGE,
  PROP_VISIBLE_PAGE_NAME,
  PROP_SEARCH_ENABLED,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

/* Copied and modified from gtklabel.c, separate_uline_pattern() */
static char *
strip_mnemonic (const char *src)
{
  char *new_str = g_new (char, strlen (src) + 1);
  char *dest = new_str;
  gboolean underscore = FALSE;

  while (*src) {
    gunichar c;
    const char *next_src;

    c = g_utf8_get_char (src);
    if (c == (gunichar) -1) {
      g_warning ("Invalid input string");

      g_free (new_str);

      return NULL;
    }

    next_src = g_utf8_next_char (src);

    if (underscore) {
      while (src < next_src)
        *dest++ = *src++;

      underscore = FALSE;
    } else {
      if (c == '_'){
        underscore = TRUE;
        src = next_src;
      } else {
        while (src < next_src)
          *dest++ = *src++;
      }
    }
  }

  *dest = 0;

  return new_str;
}

static char *
make_comparable (const char        *src,
                 AdapPreferencesRow *row,
                 gboolean           allow_underline)
{
  char *plaintext = g_utf8_casefold (src, -1);
  GError *error = NULL;

  if (adap_preferences_row_get_use_markup (row)) {
    char *parsed = NULL;

    if (pango_parse_markup (plaintext, -1, 0, NULL, &parsed, NULL, &error)) {
      g_free (plaintext);
      plaintext = parsed;
    } else {
      g_critical ("Couldn't parse markup: %s", error->message);
      g_clear_error (&error);
    }
  }

  if (allow_underline && adap_preferences_row_get_use_underline (row)) {
    char *comparable = strip_mnemonic (plaintext);
    g_free (plaintext);
    return comparable;
  }

  return plaintext;
}

static gboolean
filter_search_results (AdapPreferencesRow    *row,
                       AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);
  char *terms, *title;
  gboolean result = FALSE;

  g_assert (ADAP_IS_PREFERENCES_ROW (row));

  terms = g_utf8_casefold (gtk_editable_get_text (GTK_EDITABLE (priv->search_entry)), -1);
  title = make_comparable (adap_preferences_row_get_title (row), row, TRUE);

  if (!!strstr (title, terms)) {
    result = TRUE;
  } else if (ADAP_IS_ACTION_ROW (row)) {
    char *subtitle = make_comparable (adap_action_row_get_subtitle (ADAP_ACTION_ROW(row)), row, FALSE);

    if (!!strstr (subtitle, terms))
      result = TRUE;

    g_free (subtitle);
  }

  g_free (title);
  g_free (terms);

  return result;
}

static int
get_n_pages (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);
  int count = 0;
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (GTK_WIDGET (priv->pages_stack));
       child;
       child = gtk_widget_get_next_sibling (child)) {
    AdapViewStackPage *page = adap_view_stack_get_page (priv->pages_stack, child);

    if (adap_view_stack_page_get_visible (page))
      count++;
  }

  return count;
}

static char *
create_search_row_subtitle (AdapPreferencesDialog *self,
                            AdapPreferencesRow    *row)
{
  GtkWidget *group, *page;
  const char *group_title = NULL;
  char *page_title = NULL;

  group = gtk_widget_get_ancestor (GTK_WIDGET (row), ADAP_TYPE_PREFERENCES_GROUP);

  if (group) {
    group_title = adap_preferences_group_get_title (ADAP_PREFERENCES_GROUP (group));

    if (g_strcmp0 (group_title, "") == 0)
      group_title = NULL;
  }

  page = gtk_widget_get_ancestor (group, ADAP_TYPE_PREFERENCES_PAGE);

  if (page) {
    const char *title = adap_preferences_page_get_title (ADAP_PREFERENCES_PAGE (page));

    if (adap_preferences_page_get_use_underline (ADAP_PREFERENCES_PAGE (page)))
      page_title = strip_mnemonic (title);
    else
      page_title = g_strdup (title);

    if (adap_preferences_row_get_use_markup (row)) {
      char *tmp = page_title;

      page_title = g_markup_escape_text (page_title, -1);

      g_free (tmp);
    }

    if (g_strcmp0 (page_title, "") == 0)
      g_clear_pointer (&page_title, g_free);
  }

  if (group_title) {
    gchar *result;

    if (get_n_pages (self) > 1)
      result = g_strdup_printf ("%s → %s", page_title ? page_title : _("Untitled page"), group_title);
    else
      result = g_strdup (group_title);

    g_free (page_title);

    return result;
  }

  return page_title;
}

static GtkWidget *
new_search_row_for_preference (AdapPreferencesRow    *row,
                               AdapPreferencesDialog *self)
{
  AdapActionRow *widget;
  GtkWidget *page;
  char *subtitle;

  g_assert (ADAP_IS_PREFERENCES_ROW (row));

  subtitle = create_search_row_subtitle (self, row);
  page = gtk_widget_get_ancestor (GTK_WIDGET (row), ADAP_TYPE_PREFERENCES_PAGE);

  widget = ADAP_ACTION_ROW (adap_action_row_new ());
  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (widget), TRUE);
  adap_preferences_row_set_use_markup (ADAP_PREFERENCES_ROW (widget),
                                      adap_preferences_row_get_use_markup (row));
  adap_preferences_row_set_use_underline (ADAP_PREFERENCES_ROW (widget),
                                         adap_preferences_row_get_use_underline (row));
  adap_preferences_row_set_title (ADAP_PREFERENCES_ROW (widget),
                                 adap_preferences_row_get_title (row));
  adap_action_row_set_subtitle (widget, subtitle);

  g_object_set_data (G_OBJECT (widget), "page", page);
  g_object_set_data (G_OBJECT (widget), "row", row);

  g_clear_pointer (&subtitle, g_free);

  return GTK_WIDGET (widget);
}

static void
search_result_activated_cb (AdapPreferencesDialog *self,
                            AdapActionRow         *widget)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);
  AdapPreferencesPage *page;
  AdapPreferencesRow *row;
  GtkRoot *root;

  gtk_toggle_button_set_active (priv->search_button, FALSE);
  page = ADAP_PREFERENCES_PAGE (g_object_get_data (G_OBJECT (widget), "page"));
  row = ADAP_PREFERENCES_ROW (g_object_get_data (G_OBJECT (widget), "row"));

  g_assert (page != NULL);
  g_assert (row != NULL);

  root = gtk_widget_get_root (GTK_WIDGET (self));

  adap_view_stack_set_visible_child (priv->pages_stack, GTK_WIDGET (page));
  gtk_widget_set_can_focus (GTK_WIDGET (row), TRUE);
  gtk_widget_grab_focus (GTK_WIDGET (row));

  if (GTK_IS_WINDOW (root))
    gtk_window_set_focus_visible (GTK_WINDOW (root), TRUE);
}

static void
search_results_map (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  gtk_list_box_bind_model (priv->search_results,
                           G_LIST_MODEL (priv->filter_model),
                           (GtkListBoxCreateWidgetFunc) new_search_row_for_preference,
                           self,
                           NULL);
}

static void
search_results_unmap (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  gtk_list_box_bind_model (priv->search_results, NULL, NULL, NULL, NULL);
}

static void
update_view_switcher (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);
  AdapBreakpoint *breakpoint;
  AdapBreakpointCondition *main_condition, *fallback_condition, *condition;

  main_condition =
    adap_breakpoint_condition_new_length (ADAP_BREAKPOINT_CONDITION_MAX_WIDTH,
                                         VIEW_SWITCHER_PAGE_THRESHOLD * MAX (1, priv->n_pages),
                                         ADAP_LENGTH_UNIT_PT);
  fallback_condition =
    adap_breakpoint_condition_new_length (ADAP_BREAKPOINT_CONDITION_MAX_WIDTH,
                                         VIEW_SWITCHER_FALLBACK_THRESHOLD,
                                         ADAP_LENGTH_UNIT_PX);

  condition = adap_breakpoint_condition_new_or (main_condition, fallback_condition);

  adap_breakpoint_set_condition (priv->breakpoint, condition);

  breakpoint = adap_breakpoint_bin_get_current_breakpoint (ADAP_BREAKPOINT_BIN (priv->breakpoint_bin));

  if (!breakpoint && priv->n_pages > 1)
    gtk_stack_set_visible_child (GTK_STACK (priv->view_switcher_stack), priv->view_switcher);
  else
    gtk_stack_set_visible_child (GTK_STACK (priv->view_switcher_stack), priv->title);

  adap_breakpoint_condition_free (condition);
}

static void
title_stack_notify_transition_running_cb (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  if (gtk_stack_get_transition_running (priv->title_stack) ||
      gtk_stack_get_visible_child (priv->title_stack) != priv->view_switcher_stack)
    return;

  gtk_editable_set_text (GTK_EDITABLE (priv->search_entry), "");
}

static void
title_stack_notify_visible_child_cb (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  if (adap_get_enable_animations (GTK_WIDGET (priv->title_stack)) ||
      gtk_stack_get_visible_child (priv->title_stack) != priv->view_switcher_stack)
    return;

  gtk_editable_set_text (GTK_EDITABLE (priv->search_entry), "");
}

static void
notify_visible_page_cb (AdapPreferencesDialog *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_PAGE]);
}

static void
notify_visible_page_name_cb (AdapPreferencesDialog *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_PAGE_NAME]);
}

static void
search_button_notify_active_cb (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  if (gtk_toggle_button_get_active (priv->search_button)) {
    gtk_stack_set_visible_child_name (priv->title_stack, "search");
    gtk_stack_set_visible_child_name (priv->content_stack, "search");
    gtk_widget_grab_focus (GTK_WIDGET (priv->search_entry));
    /* Grabbing without selecting puts the cursor at the start of the buffer, so
     * for "type to search" to work we must move the cursor at the end.
     */
    gtk_editable_set_position (GTK_EDITABLE (priv->search_entry), -1);
  } else {
    gtk_stack_set_visible_child_name (priv->title_stack, "pages");
    gtk_stack_set_visible_child_name (priv->content_stack, "pages");
  }
}

static void
search_started_cb (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  gtk_toggle_button_set_active (priv->search_button, TRUE);
}

static void
search_changed_cb (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);
  guint n;

  gtk_filter_changed (priv->filter, GTK_FILTER_CHANGE_DIFFERENT);

  n = g_list_model_get_n_items (G_LIST_MODEL (priv->filter_model));

  gtk_stack_set_visible_child_name (priv->search_stack, n > 0 ? "results" : "no-results");
}

static void
stop_search_cb (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  gtk_toggle_button_set_active (priv->search_button, FALSE);
}

static void
adap_preferences_dialog_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AdapPreferencesDialog *self = ADAP_PREFERENCES_DIALOG (object);

  switch (prop_id) {
  case PROP_VISIBLE_PAGE:
    g_value_set_object (value, adap_preferences_dialog_get_visible_page (self));
    break;
  case PROP_VISIBLE_PAGE_NAME:
    g_value_set_string (value, adap_preferences_dialog_get_visible_page_name (self));
    break;
  case PROP_SEARCH_ENABLED:
    g_value_set_boolean (value, adap_preferences_dialog_get_search_enabled (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_preferences_dialog_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AdapPreferencesDialog *self = ADAP_PREFERENCES_DIALOG (object);

  switch (prop_id) {
  case PROP_VISIBLE_PAGE:
    adap_preferences_dialog_set_visible_page (self, g_value_get_object (value));
    break;
  case PROP_VISIBLE_PAGE_NAME:
    adap_preferences_dialog_set_visible_page_name (self, g_value_get_string (value));
    break;
  case PROP_SEARCH_ENABLED:
    adap_preferences_dialog_set_search_enabled (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_preferences_dialog_dispose (GObject *object)
{
  AdapPreferencesDialog *self = ADAP_PREFERENCES_DIALOG (object);
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  g_clear_object (&priv->filter_model);

  G_OBJECT_CLASS (adap_preferences_dialog_parent_class)->dispose (object);
}

static gboolean
search_open_cb (GtkWidget *widget,
                GVariant  *args,
                gpointer   user_data)
{
  AdapPreferencesDialog *self = ADAP_PREFERENCES_DIALOG (widget);
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  if (!priv->search_enabled || gtk_toggle_button_get_active (priv->search_button))
    return GDK_EVENT_PROPAGATE;

  gtk_toggle_button_set_active (priv->search_button, TRUE);

  return GDK_EVENT_STOP;
}

static void
adap_preferences_dialog_class_init (AdapPreferencesDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adap_preferences_dialog_get_property;
  object_class->set_property = adap_preferences_dialog_set_property;
  object_class->dispose = adap_preferences_dialog_dispose;

  /**
   * AdapPreferencesDialog:visible-page: (attributes org.gtk.Property.get=adap_preferences_dialog_get_visible_page org.gtk.Property.set=adap_preferences_dialog_set_visible_page)
   *
   * The currently visible page.
   *
   * Since: 1.5
   */
  props[PROP_VISIBLE_PAGE] =
    g_param_spec_object ("visible-page", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapPreferencesDialog:visible-page-name: (attributes org.gtk.Property.get=adap_preferences_dialog_get_visible_page_name org.gtk.Property.set=adap_preferences_dialog_set_visible_page_name)
   *
   * The name of the currently visible page.
   *
   * See [property@AdapPreferencesDialog:visible-page].
   *
   * Since: 1.5
   */
  props[PROP_VISIBLE_PAGE_NAME] =
    g_param_spec_string ("visible-page-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapPreferencesDialog:search-enabled: (attributes org.gtk.Property.get=adap_preferences_dialog_get_search_enabled org.gtk.Property.set=adap_preferences_dialog_set_search_enabled)
   *
   * Whether search is enabled.
   *
   * Since: 1.5
   */
  props[PROP_SEARCH_ENABLED] =
    g_param_spec_boolean ("search-enabled", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_f, GDK_CONTROL_MASK, search_open_cb, NULL);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-preferences-dialog.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, toast_overlay);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, subpages_nav_view);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, breakpoint_bin);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, content_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, pages_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, search_button);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, search_entry);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, search_results);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, search_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, title_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, view_switcher_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, view_switcher);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, title);
  gtk_widget_class_bind_template_child_private (widget_class, AdapPreferencesDialog, breakpoint);
  gtk_widget_class_bind_template_callback (widget_class, update_view_switcher);
  gtk_widget_class_bind_template_callback (widget_class, title_stack_notify_transition_running_cb);
  gtk_widget_class_bind_template_callback (widget_class, title_stack_notify_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_visible_page_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_visible_page_name_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_button_notify_active_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_started_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_result_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_results_map);
  gtk_widget_class_bind_template_callback (widget_class, search_results_unmap);
  gtk_widget_class_bind_template_callback (widget_class, stop_search_cb);
}

static GListModel *
preferences_page_to_rows (AdapViewStackPage *page)
{
  GtkWidget *child = adap_view_stack_page_get_child (page);

  g_object_unref (page);

  return adap_preferences_page_get_rows (ADAP_PREFERENCES_PAGE (child));
}

static void
adap_preferences_dialog_init (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);
  GListModel *model;
  GtkExpression *expr;

  priv->search_enabled = FALSE;

  gtk_widget_init_template (GTK_WIDGET (self));

  priv->filter = GTK_FILTER (gtk_custom_filter_new ((GtkCustomFilterFunc) filter_search_results, self, NULL));
  expr = gtk_property_expression_new (ADAP_TYPE_VIEW_STACK_PAGE, NULL, "visible");

  model = G_LIST_MODEL (adap_view_stack_get_pages (priv->pages_stack));
  model = G_LIST_MODEL (gtk_filter_list_model_new (model, GTK_FILTER (gtk_bool_filter_new (expr))));
  model = G_LIST_MODEL (gtk_map_list_model_new (model,
                                                (GtkMapListModelMapFunc) preferences_page_to_rows,
                                                NULL,
                                                NULL));
  model = G_LIST_MODEL (gtk_flatten_list_model_new (model));
  priv->filter_model = gtk_filter_list_model_new (model, priv->filter);

  gtk_widget_set_visible (GTK_WIDGET (priv->search_button), FALSE);
}

static void
adap_preferences_dialog_buildable_add_child (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const char   *type)
{
  AdapPreferencesDialog *self = ADAP_PREFERENCES_DIALOG (buildable);
  AdapPreferencesDialogPrivate *priv = adap_preferences_dialog_get_instance_private (self);

  if (priv->content_stack && ADAP_IS_PREFERENCES_PAGE (child))
    adap_preferences_dialog_add (self, ADAP_PREFERENCES_PAGE (child));
  else if (ADAP_IS_TOAST (child))
    adap_preferences_dialog_add_toast (self, g_object_ref (ADAP_TOAST (child)));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_preferences_dialog_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adap_preferences_dialog_buildable_add_child;
}

/**
 * adap_preferences_dialog_new:
 *
 * Creates a new `AdapPreferencesDialog`.
 *
 * Returns: the newly created `AdapPreferencesDialog`
 *
 * Since: 1.5
 */
AdapDialog *
adap_preferences_dialog_new (void)
{
  return g_object_new (ADAP_TYPE_PREFERENCES_DIALOG, NULL);
}

/**
 * adap_preferences_dialog_add:
 * @self: a preferences dialog
 * @page: the page to add
 *
 * Adds a preferences page to @self.
 *
 * Since: 1.5
 */
void
adap_preferences_dialog_add (AdapPreferencesDialog *self,
                            AdapPreferencesPage   *page)
{
  AdapPreferencesDialogPrivate *priv;
  AdapViewStackPage *stack_page;

  g_return_if_fail (ADAP_IS_PREFERENCES_DIALOG (self));
  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (page));

  priv = adap_preferences_dialog_get_instance_private (self);

  stack_page = adap_view_stack_add_named (priv->pages_stack, GTK_WIDGET (page), adap_preferences_page_get_name (page));

  g_object_bind_property (page, "icon-name", stack_page, "icon-name", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "title", stack_page, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "use-underline", stack_page, "use-underline", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "name", stack_page, "name", G_BINDING_SYNC_CREATE);

  priv->n_pages++;
  update_view_switcher (self);
}

/**
 * adap_preferences_dialog_remove:
 * @self: a preferences dialog
 * @page: the page to remove
 *
 * Removes a page from @self.
 *
 * Since: 1.5
 */
void
adap_preferences_dialog_remove (AdapPreferencesDialog *self,
                               AdapPreferencesPage   *page)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_DIALOG (self));
  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (page));

  priv = adap_preferences_dialog_get_instance_private (self);

  if (gtk_widget_get_parent (GTK_WIDGET (page)) == GTK_WIDGET (priv->pages_stack))
    adap_view_stack_remove (priv->pages_stack, GTK_WIDGET (page));
  else
    ADAP_CRITICAL_CANNOT_REMOVE_CHILD (self, page);

  priv->n_pages--;
  update_view_switcher (self);
}

/**
 * adap_preferences_dialog_get_visible_page:
 * @self: a preferences dialog
 *
 * Gets the currently visible page of @self.
 *
 * Returns: (transfer none) (nullable): the visible page
 *
 * Since: 1.5
 */
AdapPreferencesPage *
adap_preferences_dialog_get_visible_page (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_DIALOG (self), NULL);

  priv = adap_preferences_dialog_get_instance_private (self);

  return ADAP_PREFERENCES_PAGE (adap_view_stack_get_visible_child (priv->pages_stack));
}

/**
 * adap_preferences_dialog_set_visible_page:
 * @self: a preferences dialog
 * @page: a page of @self
 *
 * Makes @page the visible page of @self.
 *
 * Since: 1.5
 */
void
adap_preferences_dialog_set_visible_page (AdapPreferencesDialog *self,
                                         AdapPreferencesPage   *page)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_DIALOG (self));
  g_return_if_fail (ADAP_IS_PREFERENCES_PAGE (page));

  priv = adap_preferences_dialog_get_instance_private (self);

  adap_view_stack_set_visible_child (priv->pages_stack, GTK_WIDGET (page));
}

/**
 * adap_preferences_dialog_get_visible_page_name:
 * @self: a preferences dialog
 *
 * Gets the name of currently visible page of @self.
 *
 * Returns: (transfer none) (nullable): the name of the visible page
 *
 * Since: 1.5
 */
const char *
adap_preferences_dialog_get_visible_page_name (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_DIALOG (self), NULL);

  priv = adap_preferences_dialog_get_instance_private (self);

  return adap_view_stack_get_visible_child_name (priv->pages_stack);
}

/**
 * adap_preferences_dialog_set_visible_page_name:
 * @self: a preferences dialog
 * @name: the name of the page to make visible
 *
 * Makes the page with the given name visible.
 *
 * See [property@PreferencesDialog:visible-page].
 *
 * Since: 1.5
 */
void
adap_preferences_dialog_set_visible_page_name (AdapPreferencesDialog *self,
                                              const char           *name)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_DIALOG (self));

  priv = adap_preferences_dialog_get_instance_private (self);

  adap_view_stack_set_visible_child_name (priv->pages_stack, name);
}

/**
 * adap_preferences_dialog_get_search_enabled: (attributes org.gtk.Method.get_property=search-enabled)
 * @self: a preferences dialog
 *
 * Gets whether search is enabled for @self.
 *
 * Returns: whether search is enabled for @self.
 *
 * Since: 1.5
 */
gboolean
adap_preferences_dialog_get_search_enabled (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_DIALOG (self), FALSE);

  priv = adap_preferences_dialog_get_instance_private (self);

  return priv->search_enabled;
}

/**
 * adap_preferences_dialog_set_search_enabled: (attributes org.gtk.Method.set_property=search-enabled)
 * @self: a preferences dialog
 * @search_enabled: whether search is enabled
 *
 * Sets whether search is enabled for @self.
 *
 * Since: 1.5
 */
void
adap_preferences_dialog_set_search_enabled (AdapPreferencesDialog *self,
                                           gboolean              search_enabled)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_DIALOG (self));

  priv = adap_preferences_dialog_get_instance_private (self);

  search_enabled = !!search_enabled;

  if (priv->search_enabled == search_enabled)
    return;

  priv->search_enabled = search_enabled;
  gtk_widget_set_visible (GTK_WIDGET (priv->search_button), search_enabled);
  if (search_enabled) {
    gtk_search_entry_set_key_capture_widget (priv->search_entry, GTK_WIDGET (self));
  } else {
    gtk_toggle_button_set_active (priv->search_button, FALSE);
    gtk_search_entry_set_key_capture_widget (priv->search_entry, NULL);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SEARCH_ENABLED]);
}

/**
 * adap_preferences_dialog_push_subpage:
 * @self: a preferences dialog
 * @page: the subpage
 *
 * Pushes @page onto the subpage stack of @self.
 *
 * The page will be automatically removed when popped.
 *
 * Since: 1.5
 */
void
adap_preferences_dialog_push_subpage (AdapPreferencesDialog *self,
                                     AdapNavigationPage    *page)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_DIALOG (self));
  g_return_if_fail (ADAP_IS_NAVIGATION_PAGE (page));

  priv = adap_preferences_dialog_get_instance_private (self);

  adap_navigation_view_push (priv->subpages_nav_view, page);
}

/**
 * adap_preferences_dialog_pop_subpage:
 * @self: a preferences dialog
 *
 * Pop the visible page from the subpage stack of @self.
 *
 * Returns: `TRUE` if a page has been popped
 *
 * Since: 1.5
 */
gboolean
adap_preferences_dialog_pop_subpage (AdapPreferencesDialog *self)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_PREFERENCES_DIALOG (self), FALSE);

  priv = adap_preferences_dialog_get_instance_private (self);

  return adap_navigation_view_pop (priv->subpages_nav_view);
}

/**
 * adap_preferences_dialog_add_toast:
 * @self: a preferences dialog
 * @toast: (transfer full): a toast
 *
 * Displays @toast.
 *
 * See [method@ToastOverlay.add_toast].
 *
 * Since: 1.5
 */
void
adap_preferences_dialog_add_toast (AdapPreferencesDialog *self,
                                  AdapToast             *toast)
{
  AdapPreferencesDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_PREFERENCES_DIALOG (self));

  priv = adap_preferences_dialog_get_instance_private (self);

  adap_toast_overlay_add_toast (priv->toast_overlay, toast);
}
