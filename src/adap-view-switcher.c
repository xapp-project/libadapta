/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * Based on gtkstackswitcher.c, Copyright (c) 2013 Red Hat, Inc.
 * https://gitlab.gnome.org/GNOME/gtk/blob/a0129f556b1fd655215165739d0277d7f7a2c1a8/gtk/gtkstackswitcher.c
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-enums.h"
#include "adw-view-switcher.h"
#include "adw-view-switcher-button-private.h"

/**
 * AdwViewSwitcher:
 *
 * An adaptive view switcher.
 *
 * <picture>
 *   <source srcset="view-switcher-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="view-switcher.png" alt="view-switcher">
 * </picture>
 *
 * An adaptive view switcher designed to switch between multiple views
 * contained in a [class@ViewStack] in a similar fashion to
 * [class@Gtk.StackSwitcher].
 *
 * `AdwViewSwitcher` buttons always have an icon and a label. They can be
 * displayed side by side, or icon on top of the label. This can be controlled
 * via the [property@ViewSwitcher:policy] property.
 *
 * `AdwViewSwitcher` is intended to be used in a header bar together with
 * [class@ViewSwitcherBar] at the bottom of the window, and a [class@Breakpoint]
 * showing the view switcher bar on narrow sizes, while removing the view
 * switcher from the header bar, as follows:
 *
 * ```xml
 * <object class="AdwWindow">
 *   <property name="width-request">360</property>
 *   <property name="height-request">200</property>
 *   <child>
 *     <object class="AdwBreakpoint">
 *       <condition>max-width: 550sp</condition>
 *       <setter object="switcher_bar" property="reveal">True</setter>
 *       <setter object="header_bar" property="title-widget"/>
 *     </object>
 *   </child>
 *   <property name="content">
 *     <object class="AdwToolbarView">
 *       <child type="top">
 *         <object class="AdwHeaderBar" id="header_bar">
 *           <property name="title-widget">
 *             <object class="AdwViewSwitcher">
 *               <property name="stack">stack</property>
 *               <property name="policy">wide</property>
 *             </object>
 *           </property>
 *         </object>
 *       </child>
 *       <property name="content">
 *         <object class="AdwViewStack" id="stack"/>
 *       </property>
 *       <child type="bottom">
 *         <object class="AdwViewSwitcherBar" id="switcher_bar">
 *           <property name="stack">stack</property>
 *         </object>
 *       </child>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * It's recommended to set [property@ViewSwitcher:policy] to
 * `ADW_VIEW_SWITCHER_POLICY_WIDE` in this case.
 *
 * You may have to adjust the breakpoint condition for your specific pages.
 *
 * ## CSS nodes
 *
 * `AdwViewSwitcher` has a single CSS node with name `viewswitcher`. It can have
 * the style classes `.wide` and `.narrow`, matching its policy.
 *
 * ## Accessibility
 *
 * `AdwViewSwitcher` uses the `GTK_ACCESSIBLE_ROLE_TAB_LIST` role and uses the
 * `GTK_ACCESSIBLE_ROLE_TAB` for its buttons.
 */

/**
 * AdwViewSwitcherPolicy:
 * @ADW_VIEW_SWITCHER_POLICY_NARROW: Force the narrow mode
 * @ADW_VIEW_SWITCHER_POLICY_WIDE: Force the wide mode
 *
 * Describes the adaptive modes of [class@ViewSwitcher].
 */

enum {
  PROP_0,
  PROP_POLICY,
  PROP_STACK,
  LAST_PROP,
};

struct _AdwViewSwitcher
{
  GtkWidget parent_instance;

  AdwViewStack *stack;
  GtkSelectionModel *pages;
  GHashTable *buttons;

  AdwViewSwitcherPolicy policy;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdwViewSwitcher, adw_view_switcher, GTK_TYPE_WIDGET)

static void
on_button_toggled (GtkWidget       *button,
                   GParamSpec      *pspec,
                   AdwViewSwitcher *self)
{
  gboolean active;
  guint index;

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
  index = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (button), "child-index"));

  if (active) {
      gtk_selection_model_select_item (self->pages, index, TRUE);
  } else {
    gboolean selected = gtk_selection_model_is_selected (self->pages, index);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), selected);
  }
}

static void
update_button (AdwViewSwitcher  *self,
               AdwViewStackPage *page,
               GtkWidget        *button)
{
  char *title;
  char *icon_name;
  gboolean needs_attention;
  guint badge_number;
  gboolean visible;
  gboolean use_underline;

  g_object_get (page,
                "title", &title,
                "icon-name", &icon_name,
                "needs-attention", &needs_attention,
                "visible", &visible,
                "badge-number", &badge_number,
                "use-underline", &use_underline,
                NULL);

  g_object_set (G_OBJECT (button),
                "icon-name", icon_name,
                "label", title,
                "needs-attention", needs_attention,
                "badge-number", badge_number,
                "use-underline", use_underline,
                NULL);

  gtk_widget_set_visible (button, visible && (title != NULL || icon_name != NULL));

  g_free (title);
  g_free (icon_name);
}

static void
on_page_updated (AdwViewStackPage *page,
                 GParamSpec       *pspec,
                 AdwViewSwitcher  *self)
{
  GtkWidget *button;

  button = g_hash_table_lookup (self->buttons, page);
  update_button (self, page, button);
}

static void
add_child (AdwViewSwitcher *self,
           guint            position)
{
  AdwViewSwitcherButton *button = ADW_VIEW_SWITCHER_BUTTON (adw_view_switcher_button_new ());
  AdwViewStackPage *page;
  gboolean selected;

  page = g_list_model_get_item (G_LIST_MODEL (self->pages), position);
  update_button (self, page, GTK_WIDGET (button));

  gtk_widget_set_parent (GTK_WIDGET (button), GTK_WIDGET (self));

  g_object_set_data (G_OBJECT (button), "child-index", GUINT_TO_POINTER (position));
  selected = gtk_selection_model_is_selected (self->pages, position);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), selected);

  gtk_accessible_update_state (GTK_ACCESSIBLE (button),
                               GTK_ACCESSIBLE_STATE_SELECTED, selected,
                               -1);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (button),
                                  self->policy == ADW_VIEW_SWITCHER_POLICY_WIDE ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);

  g_signal_connect (button, "notify::active", G_CALLBACK (on_button_toggled), self);
  g_signal_connect (page, "notify", G_CALLBACK (on_page_updated), self);

  g_hash_table_insert (self->buttons, g_object_ref (page), button);

  g_object_unref (page);
}

static void
populate_switcher (AdwViewSwitcher *self)
{
  guint i, n;

  n = g_list_model_get_n_items (G_LIST_MODEL (self->pages));
  for (i = 0; i < n; i++)
    add_child (self, i);
}

static void
clear_switcher (AdwViewSwitcher *self)
{
  GHashTableIter iter;
  GtkWidget *page;
  GtkWidget *button;

  g_hash_table_iter_init (&iter, self->buttons);
  while (g_hash_table_iter_next (&iter, (gpointer *) &page, (gpointer *) &button)) {
    gtk_widget_unparent (button);
    g_signal_handlers_disconnect_by_func (page, on_page_updated, self);
    g_hash_table_iter_remove (&iter);
  }
}


static void
items_changed_cb (AdwViewSwitcher *self)
{
  clear_switcher (self);
  populate_switcher (self);
}

static void
selection_changed_cb (AdwViewSwitcher   *self,
                      guint              position,
                      guint              n_items)
{
  guint i;

  for (i = position; i < position + n_items; i++) {
    AdwViewStackPage *page = NULL;
    GtkWidget *button;
    gboolean selected;

    page = g_list_model_get_item (G_LIST_MODEL (self->pages), i);
    button = g_hash_table_lookup (self->buttons, page);

    if (button) {
      selected = gtk_selection_model_is_selected (self->pages, i);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), selected);

      gtk_accessible_update_state (GTK_ACCESSIBLE (button),
                                   GTK_ACCESSIBLE_STATE_SELECTED, selected,
                                   -1);
    }

    g_object_unref (page);
  }
}

static void
disconnect_stack_signals (AdwViewSwitcher *self)
{
  g_signal_handlers_disconnect_by_func (self->pages, items_changed_cb, self);
  g_signal_handlers_disconnect_by_func (self->pages, selection_changed_cb, self);
}

static void
connect_stack_signals (AdwViewSwitcher *self)
{
  g_signal_connect_swapped (self->pages, "items-changed", G_CALLBACK (items_changed_cb), self);
  g_signal_connect_swapped (self->pages, "selection-changed", G_CALLBACK (selection_changed_cb), self);
}

static void
set_stack (AdwViewSwitcher *self,
           AdwViewStack    *stack)
{
  if (!stack)
    return;

  self->stack = g_object_ref (stack);
  self->pages = adw_view_stack_get_pages (stack);
  populate_switcher (self);
  connect_stack_signals (self);
}

static void
unset_stack (AdwViewSwitcher *self)
{
  if (!self->stack)
    return;

  disconnect_stack_signals (self);
  clear_switcher (self);
  g_clear_object (&self->stack);
  g_clear_object (&self->pages);
}

static void
adw_view_switcher_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (object);

  switch (prop_id) {
  case PROP_POLICY:
    g_value_set_enum (value, adw_view_switcher_get_policy (self));
    break;
  case PROP_STACK:
    g_value_set_object (value, adw_view_switcher_get_stack (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_view_switcher_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (object);

  switch (prop_id) {
  case PROP_POLICY:
    adw_view_switcher_set_policy (self, g_value_get_enum (value));
    break;
  case PROP_STACK:
    adw_view_switcher_set_stack (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_view_switcher_dispose (GObject *object)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (object);

  unset_stack (self);

  G_OBJECT_CLASS (adw_view_switcher_parent_class)->dispose (object);
}

static void
adw_view_switcher_finalize (GObject *object)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (object);

  g_hash_table_destroy (self->buttons);

  G_OBJECT_CLASS (adw_view_switcher_parent_class)->finalize (object);
}

static void
adw_view_switcher_class_init (AdwViewSwitcherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_view_switcher_get_property;
  object_class->set_property = adw_view_switcher_set_property;
  object_class->dispose = adw_view_switcher_dispose;
  object_class->finalize = adw_view_switcher_finalize;

  /**
   * AdwViewSwitcher:policy: (attributes org.gtk.Property.get=adw_view_switcher_get_policy org.gtk.Property.set=adw_view_switcher_set_policy)
   *
   * The policy to determine which mode to use.
   */
  props[PROP_POLICY] =
    g_param_spec_enum ("policy", NULL, NULL,
                       ADW_TYPE_VIEW_SWITCHER_POLICY,
                       ADW_VIEW_SWITCHER_POLICY_NARROW,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewSwitcher:stack: (attributes org.gtk.Property.get=adw_view_switcher_get_stack org.gtk.Property.set=adw_view_switcher_set_stack)
   *
   * The stack the view switcher controls.
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack", NULL, NULL,
                         ADW_TYPE_VIEW_STACK,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "viewswitcher");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_TAB_LIST);
}

static void
adw_view_switcher_init (AdwViewSwitcher *self)
{
  GtkLayoutManager *layout = gtk_widget_get_layout_manager (GTK_WIDGET (self));

  gtk_box_layout_set_homogeneous (GTK_BOX_LAYOUT (layout), TRUE);

  gtk_widget_add_css_class (GTK_WIDGET (self), "narrow");

  self->buttons = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, NULL);
}

/**
 * adw_view_switcher_new:
 *
 * Creates a new `AdwViewSwitcher`.
 *
 * Returns: the newly created `AdwViewSwitcher`
 */
GtkWidget *
adw_view_switcher_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER, NULL);
}

/**
 * adw_view_switcher_get_policy: (attributes org.gtk.Method.get_property=policy)
 * @self: a view switcher
 *
 * Gets the policy of @self.
 *
 * Returns: the policy of @self
 */
AdwViewSwitcherPolicy
adw_view_switcher_get_policy (AdwViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER (self), ADW_VIEW_SWITCHER_POLICY_NARROW);

  return self->policy;
}

/**
 * adw_view_switcher_set_policy: (attributes org.gtk.Method.set_property=policy)
 * @self: a view switcher
 * @policy: the new policy
 *
 * Sets the policy of @self.
 */
void
adw_view_switcher_set_policy (AdwViewSwitcher       *self,
                              AdwViewSwitcherPolicy  policy)
{
  GHashTableIter iter;
  GtkWidget *button;

  g_return_if_fail (ADW_IS_VIEW_SWITCHER (self));

  if (self->policy == policy)
    return;

  self->policy = policy;

  g_hash_table_iter_init (&iter, self->buttons);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &button))
    gtk_orientable_set_orientation (GTK_ORIENTABLE (button),
                                    self->policy == ADW_VIEW_SWITCHER_POLICY_WIDE ?
                                      GTK_ORIENTATION_HORIZONTAL :
                                      GTK_ORIENTATION_VERTICAL);

  if (self->policy == ADW_VIEW_SWITCHER_POLICY_WIDE) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "wide");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "narrow");
  } else {
    gtk_widget_add_css_class (GTK_WIDGET (self), "narrow");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "wide");
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POLICY]);
}

/**
 * adw_view_switcher_get_stack: (attributes org.gtk.Method.get_property=stack)
 * @self: a view switcher
 *
 * Gets the stack controlled by @self.
 *
 * Returns: (nullable) (transfer none): the stack
 */
AdwViewStack *
adw_view_switcher_get_stack (AdwViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER (self), NULL);

  return self->stack;
}

/**
 * adw_view_switcher_set_stack: (attributes org.gtk.Method.set_property=stack)
 * @self: a view switcher
 * @stack: (nullable): a stack
 *
 * Sets the stack controlled by @self.
 */
void
adw_view_switcher_set_stack (AdwViewSwitcher *self,
                             AdwViewStack    *stack)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER (self));
  g_return_if_fail (stack == NULL || ADW_IS_VIEW_STACK (stack));

  if (self->stack == stack)
    return;

  unset_stack (self);
  set_stack (self, stack);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}
