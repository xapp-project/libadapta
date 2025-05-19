/*
 * Copyright (c) 2013 Red Hat, Inc.
 * Copyright (C) 2019 Purism SPC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-header-bar.h"

#include "adap-back-button-private.h"
#include "adap-bin.h"
#include "adap-bottom-sheet-private.h"
#include "adap-dialog.h"
#include "adap-enums.h"
#include "adap-floating-sheet-private.h"
#include "adap-gizmo-private.h"
#include "adap-navigation-split-view.h"
#include "adap-navigation-view.h"
#include "adap-overlay-split-view.h"
#include "adap-sheet-controls-private.h"
#include "adap-widget-utils-private.h"

/**
 * AdapHeaderBar:
 *
 * A title bar widget.
 *
 * <picture>
 *   <source srcset="header-bar-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="header-bar.png" alt="header-bar">
 * </picture>
 *
 * `AdapHeaderBar` is similar to [class@Gtk.HeaderBar], but provides additional
 * features compared to it. Refer to `GtkHeaderBar` for details. It is typically
 * used as a top bar within [class@ToolbarView].
 *
 * ## Dialog Integration
 *
 * When placed inside an [class@Dialog], `AdapHeaderBar` will display the dialog
 * title intead of window title. It will also adjust the decoration layout to
 * ensure it always has a close button and nothing else. Set
 * [property@HeaderBar:show-start-title-buttons] and
 * [property@HeaderBar:show-end-title-buttons] to `FALSE` to remove it if it's
 * unwanted.
 *
 * ## Navigation View Integration
 *
 * When placed inside an [class@NavigationPage], `AdapHeaderBar` will display the
 * page title instead of window title.
 *
 * When used together with [class@NavigationView] or [class@NavigationSplitView],
 * it will also display a back button that can be used to go back to the previous
 * page. The button also has a context menu, allowing to pop multiple pages at
 * once, potentially across multiple navigation views.
 *
 * Set [property@HeaderBar:show-back-button] to `FALSE` to disable this behavior
 * in rare scenarios where it's unwanted.
 *
 * ## Split View Integration
 *
 * When placed inside [class@NavigationSplitView] or [class@OverlaySplitView],
 * `AdapHeaderBar` will automatically hide the title buttons other than at the
 * edges of the window.
 *
 * ## Centering Policy
 *
 * [property@HeaderBar:centering-policy] allows to enforce strict centering of
 * the title widget. This can be useful for entries inside [class@Clamp].
 *
 * ## Title Buttons
 *
 * Unlike `GtkHeaderBar`, `AdapHeaderBar` allows to toggle title button
 * visibility for each side individually, using the
 * [property@HeaderBar:show-start-title-buttons] and
 * [property@HeaderBar:show-end-title-buttons] properties.
 *
 * ## CSS nodes
 *
 * ```
 * headerbar
 * ╰── windowhandle
 *     ╰── box
 *         ├── widget
 *         │   ╰── box.start
 *         │       ├── windowcontrols.start
 *         │       ├── widget
 *         │       │   ╰── [button.back]
 *         │       ╰── [other children]
 *         ├── widget
 *         │   ╰── [Title Widget]
 *         ╰── widget
 *             ╰── box.end
 *                 ├── [other children]
 *                 ╰── windowcontrols.end
 * ```
 *
 * `AdapHeaderBar`'s CSS node is called `headerbar`. It contains a `windowhandle`
 * subnode, which contains a `box` subnode, which contains three `widget`
 * subnodes at the start, center and end of the header bar. The start and end
 * subnodes contain a `box` subnode with the `.start` and `.end` style classes
 * respectively, and the center node contains a node that represents the title.
 *
 * Each of the boxes contains a `windowcontrols` subnode, see
 * [class@Gtk.WindowControls] for details, as well as other children.
 *
 * When [property@HeaderBar:show-back-button] is `TRUE`, the start box also
 * contains a node with the name `widget` that contains a node with the name
 * `button` and `.back` style class.
 *
 * ## Accessibility
 *
 * `AdapHeaderBar` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 */

/**
 * AdapCenteringPolicy:
 * @ADAP_CENTERING_POLICY_LOOSE: Keep the title centered when possible
 * @ADAP_CENTERING_POLICY_STRICT: Keep the title centered at all cost
 *
 * Describes title centering behavior of a [class@HeaderBar] widget.
 */

#define MIN_TITLE_CHARS 5

#define MOBILE_WINDOW_WIDTH  480
#define MOBILE_WINDOW_HEIGHT 800

typedef struct {
  GtkWidget *split_view;
  gboolean is_sidebar;
} SplitViewData;

struct _AdapHeaderBar {
  GtkWidget parent_instance;

  GtkWidget *handle;
  GtkWidget *center_box;
  GtkWidget *start_bin;
  GtkWidget *end_bin;
  GtkWidget *center_bin;

  GtkWidget *start_box;
  GtkWidget *end_box;

  GtkWidget *title_label;
  GtkWidget *title_widget;

  GtkWidget *start_controls;
  GtkWidget *end_controls;
  GtkWidget *back_button;

  char *decoration_layout;

  guint show_start_title_buttons : 1;
  guint show_end_title_buttons : 1;
  guint show_back_button : 1;
  guint track_default_decoration : 1;

  AdapCenteringPolicy centering_policy;

  GtkSizeGroup *size_group;

  GtkWidget *title_navigation_page;
  GtkWidget *dialog;
  GtkWidget *sheet;

  GSList *split_views;
};

enum {
  PROP_0,
  PROP_TITLE_WIDGET,
  PROP_SHOW_START_TITLE_BUTTONS,
  PROP_SHOW_END_TITLE_BUTTONS,
  PROP_SHOW_BACK_BUTTON,
  PROP_DECORATION_LAYOUT,
  PROP_CENTERING_POLICY,
  PROP_SHOW_TITLE,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP] = { NULL, };

static void adap_header_bar_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapHeaderBar, adap_header_bar, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_header_bar_buildable_init));

static GtkBuildableIface *parent_buildable_iface;

static void
update_box_visibility (GtkWidget *box)
{
  gboolean has_visible = FALSE;
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (box);
       child;
       child = gtk_widget_get_next_sibling (child)) {
    if (gtk_widget_get_visible (child)) {
      has_visible = TRUE;
      break;
    }
  }

  gtk_widget_set_visible (box, has_visible);
}

static void
update_decoration_layout (AdapHeaderBar *self,
                          gboolean      start,
                          gboolean      end)
{
  if (start && self->start_controls) {
    g_object_set (self->start_controls,
                  "decoration-layout", self->decoration_layout,
                  NULL);
  }

  if (end && self->end_controls) {
    g_object_set (self->end_controls,
                  "decoration-layout", self->decoration_layout,
                  NULL);
  }
}

static void
create_start_controls (AdapHeaderBar *self)
{
  GtkWidget *controls;

  if (self->sheet)
    controls = adap_sheet_controls_new (GTK_PACK_START);
  else
    controls = gtk_window_controls_new (GTK_PACK_START);

  g_object_bind_property (controls, "empty",
                          controls, "visible",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);
  g_signal_connect_swapped (controls, "notify::visible",
                            G_CALLBACK (update_box_visibility),
                            self->start_box);
  gtk_box_prepend (GTK_BOX (self->start_box), controls);
  self->start_controls = controls;

  update_decoration_layout (self, TRUE, FALSE);
}

static void
create_end_controls (AdapHeaderBar *self)
{
  GtkWidget *controls;

  if (self->sheet)
    controls = adap_sheet_controls_new (GTK_PACK_END);
  else
    controls = gtk_window_controls_new (GTK_PACK_END);

  g_object_bind_property (controls, "empty",
                          controls, "visible",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);
  g_signal_connect_swapped (controls, "notify::visible",
                            G_CALLBACK (update_box_visibility),
                            self->end_box);
  gtk_box_append (GTK_BOX (self->end_box), controls);
  self->end_controls = controls;
}

static void
create_back_button (AdapHeaderBar *self)
{
  GtkWidget *button = adap_back_button_new ();

  gtk_box_insert_child_after (GTK_BOX (self->start_box),
                              button,
                              self->start_controls);
  g_signal_connect_swapped (button, "notify::visible",
                            G_CALLBACK (update_box_visibility),
                            self->start_box);

  self->back_button = button;
}

static void
update_start_title_buttons (AdapHeaderBar *self)
{
  gboolean show = self->show_start_title_buttons;
  GSList *l;

  for (l = self->split_views; l; l = l->next) {
    SplitViewData *data = l->data;

    if (ADAP_IS_NAVIGATION_SPLIT_VIEW (data->split_view)) {
      AdapNavigationSplitView *split_view = ADAP_NAVIGATION_SPLIT_VIEW (data->split_view);
      gboolean collapsed = adap_navigation_split_view_get_collapsed (split_view);

      show &= data->is_sidebar || collapsed;
    }

    if (ADAP_IS_OVERLAY_SPLIT_VIEW (data->split_view)) {
      AdapOverlaySplitView *split_view = ADAP_OVERLAY_SPLIT_VIEW (data->split_view);
      gboolean collapsed = adap_overlay_split_view_get_collapsed (split_view);
      gboolean show_sidebar = adap_overlay_split_view_get_show_sidebar (split_view);
      GtkPackType sidebar_pos = adap_overlay_split_view_get_sidebar_position (split_view);

      if (data->is_sidebar)
        show &= sidebar_pos == GTK_PACK_START;
      else
        show &= collapsed || !show_sidebar || sidebar_pos == GTK_PACK_END;
    }
  }

  if ((self->start_controls != NULL) == show)
    return;

  if (show) {
    create_start_controls (self);
  } else if (self->start_box && self->start_controls) {
    gtk_box_remove (GTK_BOX (self->start_box), self->start_controls);
    self->start_controls = NULL;
  }

  update_box_visibility (self->start_box);
}

static void
update_end_title_buttons (AdapHeaderBar *self)
{
  gboolean show = self->show_end_title_buttons;
  GSList *l;

  for (l = self->split_views; l; l = l->next) {
    SplitViewData *data = l->data;

    if (ADAP_IS_NAVIGATION_SPLIT_VIEW (data->split_view)) {
      AdapNavigationSplitView *split_view = ADAP_NAVIGATION_SPLIT_VIEW (data->split_view);
      gboolean collapsed = adap_navigation_split_view_get_collapsed (split_view);

      show &= !data->is_sidebar || collapsed;
    }

    if (ADAP_IS_OVERLAY_SPLIT_VIEW (data->split_view)) {
      AdapOverlaySplitView *split_view = ADAP_OVERLAY_SPLIT_VIEW (data->split_view);
      gboolean collapsed = adap_overlay_split_view_get_collapsed (split_view);
      gboolean show_sidebar = adap_overlay_split_view_get_show_sidebar (split_view);
      GtkPackType sidebar_pos = adap_overlay_split_view_get_sidebar_position (split_view);

      if (data->is_sidebar)
        show &= sidebar_pos == GTK_PACK_END;
      else
        show &= collapsed || !show_sidebar || sidebar_pos == GTK_PACK_START;
    }
  }

  if ((self->end_controls != NULL) == show)
    return;

  if (show) {
    create_end_controls (self);
  } else if (self->end_box && self->end_controls) {
    gtk_box_remove (GTK_BOX (self->end_box), self->end_controls);
    self->end_controls = NULL;
  }

  update_box_visibility (self->end_box);
}

static void
update_title_buttons (AdapHeaderBar *self)
{
  update_start_title_buttons (self);
  update_end_title_buttons (self);
}

static void
update_title (AdapHeaderBar *self)
{
  const char *title = NULL;

  if (!self->title_label)
    return;

  if (self->title_navigation_page)
    title = adap_navigation_page_get_title (ADAP_NAVIGATION_PAGE (self->title_navigation_page));

  if (!title && self->dialog)
    title = adap_dialog_get_title (ADAP_DIALOG (self->dialog));

  if (!title) {
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

    if (GTK_IS_WINDOW (root))
      title = gtk_window_get_title (GTK_WINDOW (root));
  }

  if (!title)
    title = g_get_application_name ();

  if (!title)
    title = g_get_prgname ();

  gtk_label_set_text (GTK_LABEL (self->title_label), title);
}


static void
construct_title_label (AdapHeaderBar *self)
{
  GtkWidget *label;

  g_assert (self->title_label == NULL);

  label = gtk_label_new (NULL);
  gtk_widget_add_css_class (label, "title");
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
  gtk_label_set_wrap (GTK_LABEL (label), FALSE);
  gtk_label_set_single_line_mode (GTK_LABEL (label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_width_chars (GTK_LABEL (label), MIN_TITLE_CHARS);
  adap_bin_set_child (ADAP_BIN (self->center_bin), label);

  self->title_label = label;

  update_title (self);
}

static void
adap_header_bar_root (GtkWidget *widget)
{
  AdapHeaderBar *self = ADAP_HEADER_BAR (widget);
  GtkWidget *parent;

  GTK_WIDGET_CLASS (adap_header_bar_parent_class)->root (widget);

  self->title_navigation_page =
    adap_widget_get_ancestor (widget, ADAP_TYPE_NAVIGATION_PAGE, TRUE, TRUE);

  self->dialog = adap_widget_get_ancestor (widget, ADAP_TYPE_DIALOG, TRUE, FALSE);

  self->sheet = adap_widget_get_ancestor (widget, ADAP_TYPE_BOTTOM_SHEET, TRUE, FALSE);
  if (!self->sheet)
    self->sheet = adap_widget_get_ancestor (widget, ADAP_TYPE_FLOATING_SHEET, TRUE, FALSE);

  if (self->title_navigation_page) {
    g_signal_connect_swapped (self->title_navigation_page, "notify::title",
                              G_CALLBACK (update_title), widget);
  } else if (self->dialog) {
    g_signal_connect_swapped (self->dialog, "notify::title",
                              G_CALLBACK (update_title), widget);
  } else {
    GtkRoot *root = gtk_widget_get_root (widget);

    if (GTK_IS_WINDOW (root))
      g_signal_connect_swapped (root, "notify::title",
                                G_CALLBACK (update_title), widget);
  }

  parent = gtk_widget_get_parent (widget);

  while (parent != NULL) {
    GtkWidget *split_view = NULL;
    gboolean is_sidebar = FALSE;

    if (GTK_IS_NATIVE (parent))
      break;

    if (ADAP_IS_NAVIGATION_SPLIT_VIEW (parent)) {
      AdapNavigationPage *sidebar;

      split_view = parent;

      g_signal_connect_swapped (split_view, "notify::collapsed",
                                G_CALLBACK (update_title_buttons), widget);

      sidebar = adap_navigation_split_view_get_sidebar (ADAP_NAVIGATION_SPLIT_VIEW (split_view));

      if (sidebar) {
        is_sidebar = widget == GTK_WIDGET (sidebar) ||
                     (sidebar && gtk_widget_is_ancestor (widget, GTK_WIDGET (sidebar)));
      }
    } else if (ADAP_IS_OVERLAY_SPLIT_VIEW (parent)) {
      GtkWidget *sidebar;

      split_view = parent;

      g_signal_connect_swapped (split_view, "notify::collapsed",
                                G_CALLBACK (update_title_buttons), widget);
      g_signal_connect_swapped (split_view, "notify::sidebar-position",
                                G_CALLBACK (update_title_buttons), widget);
      g_signal_connect_swapped (split_view, "notify::show-sidebar",
                                G_CALLBACK (update_title_buttons), widget);

      sidebar = adap_overlay_split_view_get_sidebar (ADAP_OVERLAY_SPLIT_VIEW (split_view));

      is_sidebar = widget == sidebar ||
                   (sidebar && gtk_widget_is_ancestor (widget, sidebar));
    }

    if (split_view) {
      SplitViewData *data = g_new0 (SplitViewData, 1);

      data->split_view = split_view;
      data->is_sidebar = is_sidebar;

      self->split_views = g_slist_prepend (self->split_views, data);
    }

    parent = gtk_widget_get_parent (parent);
  }

  update_title (self);
  update_title_buttons (self);
  update_decoration_layout (self, TRUE, TRUE);
}

static void
adap_header_bar_unroot (GtkWidget *widget)
{
  AdapHeaderBar *self = ADAP_HEADER_BAR (widget);
  GSList *l;

  if (self->title_navigation_page) {
    g_signal_handlers_disconnect_by_func (self->title_navigation_page,
                                          update_title, widget);
  } else if (self->dialog) {
    g_signal_handlers_disconnect_by_func (self->dialog,
                                          update_title, widget);
  } else {
    g_signal_handlers_disconnect_by_func (gtk_widget_get_root (widget),
                                          update_title, widget);
  }

  self->title_navigation_page = NULL;
  self->dialog = NULL;
  self->sheet = NULL;

  for (l = self->split_views; l; l = l->next) {
    SplitViewData *data = l->data;

    g_signal_handlers_disconnect_by_func (data->split_view,
                                          update_title_buttons, widget);

    g_free (data);
  }

  g_clear_pointer (&self->split_views, g_slist_free);

  GTK_WIDGET_CLASS (adap_header_bar_parent_class)->unroot (widget);
}

static void
adap_header_bar_dispose (GObject *object)
{
  AdapHeaderBar *self = ADAP_HEADER_BAR (object);

  self->title_widget = NULL;
  self->title_label = NULL;
  self->start_box = NULL;
  self->end_box = NULL;
  self->start_bin = NULL;
  self->end_bin = NULL;
  self->center_bin = NULL;

  g_clear_object (&self->size_group);
  g_clear_pointer (&self->handle, gtk_widget_unparent);

  G_OBJECT_CLASS (adap_header_bar_parent_class)->dispose (object);
}

static void
adap_header_bar_finalize (GObject *object)
{
  AdapHeaderBar *self = ADAP_HEADER_BAR (object);

  g_clear_pointer (&self->decoration_layout, g_free);

  G_OBJECT_CLASS (adap_header_bar_parent_class)->finalize (object);
}

static void
adap_header_bar_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdapHeaderBar *self = ADAP_HEADER_BAR (object);

  switch (prop_id) {
  case PROP_TITLE_WIDGET:
    g_value_set_object (value, self->title_widget);
    break;
  case PROP_SHOW_START_TITLE_BUTTONS:
    g_value_set_boolean (value, adap_header_bar_get_show_start_title_buttons (self));
    break;
  case PROP_SHOW_END_TITLE_BUTTONS:
    g_value_set_boolean (value, adap_header_bar_get_show_end_title_buttons (self));
    break;
  case PROP_SHOW_BACK_BUTTON:
    g_value_set_boolean (value, adap_header_bar_get_show_back_button (self));
    break;
  case PROP_DECORATION_LAYOUT:
    g_value_set_string (value, adap_header_bar_get_decoration_layout (self));
    break;
  case PROP_CENTERING_POLICY:
    g_value_set_enum (value, adap_header_bar_get_centering_policy (self));
    break;
  case PROP_SHOW_TITLE:
    g_value_set_boolean (value, adap_header_bar_get_show_title (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adap_header_bar_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdapHeaderBar *self = ADAP_HEADER_BAR (object);

  switch (prop_id) {
  case PROP_TITLE_WIDGET:
    adap_header_bar_set_title_widget (self, g_value_get_object (value));
    break;
  case PROP_SHOW_START_TITLE_BUTTONS:
    adap_header_bar_set_show_start_title_buttons (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_END_TITLE_BUTTONS:
    adap_header_bar_set_show_end_title_buttons (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_BACK_BUTTON:
    adap_header_bar_set_show_back_button (self, g_value_get_boolean (value));
    break;
  case PROP_DECORATION_LAYOUT:
    adap_header_bar_set_decoration_layout (self, g_value_get_string (value));
    break;
  case PROP_CENTERING_POLICY:
    adap_header_bar_set_centering_policy (self, g_value_get_enum (value));
    break;
  case PROP_SHOW_TITLE:
    adap_header_bar_set_show_title (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adap_header_bar_class_init (AdapHeaderBarClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  object_class->dispose = adap_header_bar_dispose;
  object_class->finalize = adap_header_bar_finalize;
  object_class->get_property = adap_header_bar_get_property;
  object_class->set_property = adap_header_bar_set_property;

  widget_class->root = adap_header_bar_root;
  widget_class->unroot = adap_header_bar_unroot;
  widget_class->compute_expand = adap_widget_compute_expand_horizontal_only;

  /**
   * AdapHeaderBar:title-widget: (attributes org.gtk.Property.get=adap_header_bar_get_title_widget org.gtk.Property.set=adap_header_bar_set_title_widget)
   *
   * The title widget to display.
   *
   * When set to `NULL`, the header bar will display the title of the window it
   * is contained in.
   *
   * To use a different title, use [class@WindowTitle]:
   *
   * ```xml
   * <object class="AdapHeaderBar">
   *   <property name="title-widget">
   *     <object class="AdapWindowTitle">
   *       <property name="title" translatable="yes">Title</property>
   *     </object>
   *   </property>
   * </object>
   * ```
   */
  props[PROP_TITLE_WIDGET] =
    g_param_spec_object ("title-widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapHeaderBar:show-start-title-buttons: (attributes org.gtk.Property.get=adap_header_bar_get_show_start_title_buttons org.gtk.Property.set=adap_header_bar_set_show_start_title_buttons)
   *
   * Whether to show title buttons at the start of the header bar.
   *
   * See [property@HeaderBar:show-end-title-buttons] for the other side.
   *
   * Which buttons are actually shown and where is determined by the
   * [property@HeaderBar:decoration-layout] property, and by the state of the
   * window (e.g. a close button will not be shown if the window can't be
   * closed).
   */
  props[PROP_SHOW_START_TITLE_BUTTONS] =
    g_param_spec_boolean ("show-start-title-buttons", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapHeaderBar:show-end-title-buttons: (attributes org.gtk.Property.get=adap_header_bar_get_show_end_title_buttons org.gtk.Property.set=adap_header_bar_set_show_end_title_buttons)
   *
   * Whether to show title buttons at the end of the header bar.
   *
   * See [property@HeaderBar:show-start-title-buttons] for the other side.
   *
   * Which buttons are actually shown and where is determined by the
   * [property@HeaderBar:decoration-layout] property, and by the state of the
   * window (e.g. a close button will not be shown if the window can't be
   * closed).
   */
  props[PROP_SHOW_END_TITLE_BUTTONS] =
    g_param_spec_boolean ("show-end-title-buttons", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapHeaderBar:show-back-button: (attributes org.gtk.Property.get=adap_header_bar_get_show_back_button org.gtk.Property.set=adap_header_bar_set_show_back_button)
   *
   * Whether the header bar can show the back button.
   *
   * The back button will never be shown unless the header bar is placed inside an
   * [class@NavigationView]. Usually, there is no reason to set this to `FALSE`.
   *
   * Since: 1.4
   */
  props[PROP_SHOW_BACK_BUTTON] =
    g_param_spec_boolean ("show-back-button", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapHeaderBar:decoration-layout: (attributes org.gtk.Property.get=adap_header_bar_get_decoration_layout org.gtk.Property.set=adap_header_bar_set_decoration_layout)
   *
   * The decoration layout for buttons.
   *
   * If this property is not set, the
   * [property@Gtk.Settings:gtk-decoration-layout] setting is used.
   *
   * The format of the string is button names, separated by commas. A colon
   * separates the buttons that should appear at the start from those at the
   * end. Recognized button names are minimize, maximize, close and icon (the
   * window icon).
   *
   * For example, “icon:minimize,maximize,close” specifies an icon at the start,
   * and minimize, maximize and close buttons at the end.
   */
  props[PROP_DECORATION_LAYOUT] =
    g_param_spec_string ("decoration-layout", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapHeaderBar:centering-policy: (attributes org.gtk.Property.get=adap_header_bar_get_centering_policy org.gtk.Property.set=adap_header_bar_set_centering_policy)
   *
   * The policy for aligning the center widget.
   */
  props[PROP_CENTERING_POLICY] =
    g_param_spec_enum ("centering-policy", NULL, NULL,
                       ADAP_TYPE_CENTERING_POLICY,
                       ADAP_CENTERING_POLICY_LOOSE,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapHeaderBar:show-title: (attributes org.gtk.Property.get=adap_header_bar_get_show_title org.gtk.Property.set=adap_header_bar_set_show_title)
   *
   * Whether the title widget should be shown.
   *
   * Since: 1.4
   */
  props[PROP_SHOW_TITLE] =
    g_param_spec_boolean ("show-title", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "headerbar");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adap_header_bar_init (AdapHeaderBar *self)
{
  self->title_widget = NULL;
  self->decoration_layout = NULL;
  self->show_start_title_buttons = TRUE;
  self->show_end_title_buttons = TRUE;
  self->show_back_button = TRUE;

  self->handle = gtk_window_handle_new ();
  gtk_widget_set_parent (self->handle, GTK_WIDGET (self));

  self->center_box = gtk_center_box_new ();
  gtk_center_box_set_shrink_center_last (GTK_CENTER_BOX (self->center_box), FALSE);
  gtk_window_handle_set_child (GTK_WINDOW_HANDLE (self->handle), self->center_box);

  self->start_bin = adap_gizmo_new ("widget", NULL, NULL, NULL, NULL,
                                   (AdapGizmoFocusFunc) adap_widget_focus_child,
                                   (AdapGizmoGrabFocusFunc) adap_widget_grab_focus_child);
  gtk_widget_set_layout_manager (self->start_bin, gtk_bin_layout_new ());
  gtk_center_box_set_start_widget (GTK_CENTER_BOX (self->center_box), self->start_bin);

  self->end_bin = adap_gizmo_new ("widget", NULL, NULL, NULL, NULL,
                                 (AdapGizmoFocusFunc) adap_widget_focus_child,
                                 (AdapGizmoGrabFocusFunc) adap_widget_grab_focus_child);
  gtk_widget_set_layout_manager (self->end_bin, gtk_bin_layout_new ());
  gtk_center_box_set_end_widget (GTK_CENTER_BOX (self->center_box), self->end_bin);

  self->center_bin = adap_bin_new ();
  gtk_center_box_set_center_widget (GTK_CENTER_BOX (self->center_box), self->center_bin);

  self->start_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_halign (self->start_box, GTK_ALIGN_START);
  gtk_widget_add_css_class (self->start_box, "start");
  gtk_widget_set_parent (self->start_box, self->start_bin);

  self->end_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_halign (self->end_box, GTK_ALIGN_END);
  gtk_widget_add_css_class (self->end_box, "end");
  gtk_widget_set_parent (self->end_box, self->end_bin);

  self->size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  construct_title_label (self);
  create_back_button (self);
}

static void
adap_header_bar_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder   *builder,
                                    GObject      *child,
                                    const char   *type)
{
  if (g_strcmp0 (type, "title") == 0)
    adap_header_bar_set_title_widget (ADAP_HEADER_BAR (buildable), GTK_WIDGET (child));
  else if (g_strcmp0 (type, "start") == 0)
    adap_header_bar_pack_start (ADAP_HEADER_BAR (buildable), GTK_WIDGET (child));
  else if (g_strcmp0 (type, "end") == 0)
    adap_header_bar_pack_end (ADAP_HEADER_BAR (buildable), GTK_WIDGET (child));
  else if (type == NULL && GTK_IS_WIDGET (child))
    adap_header_bar_pack_start (ADAP_HEADER_BAR (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_header_bar_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = adap_header_bar_buildable_add_child;

  parent_buildable_iface = g_type_interface_peek_parent (iface);
}

/**
 * adap_header_bar_new:
 *
 * Creates a new `AdapHeaderBar`.
 *
 * Returns: the newly created `AdapHeaderBar`.
 */
GtkWidget *
adap_header_bar_new (void)
{
  return GTK_WIDGET (g_object_new (ADAP_TYPE_HEADER_BAR, NULL));
}

/**
 * adap_header_bar_pack_start:
 * @self: a header bar
 * @child: the widget to be added to @self
 *
 * Adds @child to @self, packed with reference to the start of the @self.
 */
void
adap_header_bar_pack_start (AdapHeaderBar *self,
                           GtkWidget    *child)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  gtk_box_append (GTK_BOX (self->start_box), child);
  update_box_visibility (self->start_box);

  g_signal_connect_swapped (child, "notify::visible",
                            G_CALLBACK (update_box_visibility),
                            self->start_box);
}

/**
 * adap_header_bar_pack_end:
 * @self: a header bar
 * @child: the widget to be added to @self
 *
 * Adds @child to @self, packed with reference to the end of @self.
 */
void
adap_header_bar_pack_end (AdapHeaderBar *self,
                         GtkWidget    *child)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  gtk_box_prepend (GTK_BOX (self->end_box), child);
  update_box_visibility (self->end_box);

  g_signal_connect_swapped (child, "notify::visible",
                            G_CALLBACK (update_box_visibility),
                            self->end_box);
}

/**
 * adap_header_bar_remove:
 * @self: a header bar
 * @child: the child to remove
 *
 * Removes a child from @self.
 *
 * The child must have been added with [method@HeaderBar.pack_start],
 * [method@HeaderBar.pack_end] or [property@HeaderBar:title-widget].
 */
void
adap_header_bar_remove (AdapHeaderBar *self,
                       GtkWidget    *child)
{
  GtkWidget *parent;

  g_return_if_fail (ADAP_IS_HEADER_BAR (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  parent = gtk_widget_get_parent (child);

  if (parent == self->start_box || parent == self->end_box) {
    g_signal_handlers_disconnect_by_func (child,
                                          update_box_visibility,
                                          parent);

    gtk_box_remove (GTK_BOX (parent), child);

    update_box_visibility (parent);
  } else if (parent == self->center_bin) {
    adap_bin_set_child (ADAP_BIN (self->center_bin), NULL);
  } else {
    ADAP_CRITICAL_CANNOT_REMOVE_CHILD (self, child);
  }
}

/**
 * adap_header_bar_get_title_widget: (attributes org.gtk.Method.get_property=title-widget)
 * @self: a header bar
 *
 * Gets the title widget widget of @self.
 *
 * Returns: (nullable) (transfer none): the title widget
 */
GtkWidget *
adap_header_bar_get_title_widget (AdapHeaderBar *self)
{
  g_return_val_if_fail (ADAP_IS_HEADER_BAR (self), NULL);

  return self->title_widget;
}

/**
 * adap_header_bar_set_title_widget: (attributes org.gtk.Method.set_property=title-widget)
 * @self: a header bar
 * @title_widget: (nullable): a widget to use for a title
 *
 * Sets the title widget for @self.
 *
 * When set to `NULL`, the header bar will display the title of the window it
 * is contained in.
 *
 * To use a different title, use [class@WindowTitle]:
 *
 * ```xml
 * <object class="AdapHeaderBar">
 *   <property name="title-widget">
 *     <object class="AdapWindowTitle">
 *       <property name="title" translatable="yes">Title</property>
 *     </object>
 *   </property>
 * </object>
 * ```
 */
void
adap_header_bar_set_title_widget (AdapHeaderBar *self,
                                 GtkWidget    *title_widget)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));

  if (title_widget)
    g_return_if_fail (GTK_IS_WIDGET (title_widget));

  /* No need to do anything if the title widget stays the same */
  if (self->title_widget == title_widget)
    return;

  adap_bin_set_child (ADAP_BIN (self->center_bin), NULL);
  self->title_widget = NULL;

  if (title_widget != NULL) {
    self->title_widget = title_widget;

    adap_bin_set_child (ADAP_BIN (self->center_bin), title_widget);

    self->title_label = NULL;
  } else if (self->title_label == NULL)
    construct_title_label (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE_WIDGET]);
}

/**
 * adap_header_bar_get_show_start_title_buttons: (attributes org.gtk.Method.get_property=show-start-title-buttons)
 * @self: a header bar
 *
 * Gets whether to show title buttons at the start of @self.
 *
 * Returns: `TRUE` if title buttons at the start are shown
 */
gboolean
adap_header_bar_get_show_start_title_buttons (AdapHeaderBar *self)
{
  g_return_val_if_fail (ADAP_IS_HEADER_BAR (self), FALSE);

  return self->show_start_title_buttons;
}

/**
 * adap_header_bar_set_show_start_title_buttons: (attributes org.gtk.Method.set_property=show-start-title-buttons)
 * @self: a header bar
 * @setting: `TRUE` to show standard title buttons
 *
 * Sets whether to show title buttons at the start of @self.
 *
 * See [property@HeaderBar:show-end-title-buttons] for the other side.
 *
 * Which buttons are actually shown and where is determined by the
 * [property@HeaderBar:decoration-layout] property, and by the state of the
 * window (e.g. a close button will not be shown if the window can't be closed).
 */
void
adap_header_bar_set_show_start_title_buttons (AdapHeaderBar *self,
                                             gboolean      setting)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));

  setting = setting != FALSE;

  if (self->show_start_title_buttons == setting)
    return;

  self->show_start_title_buttons = setting;

  if (self->start_box)
    update_start_title_buttons (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_START_TITLE_BUTTONS]);
}

/**
 * adap_header_bar_get_show_end_title_buttons: (attributes org.gtk.Method.get_property=show-end-title-buttons)
 * @self: a header bar
 *
 * Gets whether to show title buttons at the end of @self.
 *
 * Returns: `TRUE` if title buttons at the end are shown
 */
gboolean
adap_header_bar_get_show_end_title_buttons (AdapHeaderBar *self)
{
  g_return_val_if_fail (ADAP_IS_HEADER_BAR (self), FALSE);

  return self->show_end_title_buttons;
}

/**
 * adap_header_bar_set_show_end_title_buttons: (attributes org.gtk.Method.set_property=show-end-title-buttons)
 * @self: a header bar
 * @setting: `TRUE` to show standard title buttons
 *
 * Sets whether to show title buttons at the end of @self.
 *
 * See [property@HeaderBar:show-start-title-buttons] for the other side.
 *
 * Which buttons are actually shown and where is determined by the
 * [property@HeaderBar:decoration-layout] property, and by the state of the
 * window (e.g. a close button will not be shown if the window can't be closed).
 */
void
adap_header_bar_set_show_end_title_buttons (AdapHeaderBar *self,
                                           gboolean      setting)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));

  setting = setting != FALSE;

  if (self->show_end_title_buttons == setting)
    return;

  self->show_end_title_buttons = setting;

  if (self->end_box)
    update_end_title_buttons (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_END_TITLE_BUTTONS]);
}

/**
 * adap_header_bar_get_show_back_button: (attributes org.gtk.Method.get_property=show-back-button)
 * @self: a header bar
 *
 * Gets whether @self can show the back button.
 *
 * Returns: whether to show the back button
 *
 * Since: 1.4
 */
gboolean
adap_header_bar_get_show_back_button (AdapHeaderBar *self)
{
  g_return_val_if_fail (ADAP_IS_HEADER_BAR (self), FALSE);

  return self->show_back_button;
}

/**
 * adap_header_bar_set_show_back_button: (attributes org.gtk.Method.set_property=show-back-button)
 * @self: a header bar
 * @show_back_button: whether to show the back button
 *
 * Sets whether @self can show the back button.
 *
 * The back button will never be shown unless the header bar is placed inside an
 * [class@NavigationView]. Usually, there is no reason to set it to `FALSE`.
 *
 * Since: 1.4
 */
void
adap_header_bar_set_show_back_button (AdapHeaderBar *self,
                                     gboolean      show_back_button)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));

  show_back_button = !!show_back_button;

  if (self->show_back_button == show_back_button)
    return;

  self->show_back_button = show_back_button;

  if (self->start_box) {
    if (self->show_back_button) {
      create_back_button (self);
    } else if (self->back_button) {
      gtk_box_remove (GTK_BOX (self->start_box), self->back_button);
      self->back_button = NULL;
    }

    update_box_visibility (self->start_box);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_BACK_BUTTON]);
}

/**
 * adap_header_bar_get_decoration_layout: (attributes org.gtk.Method.get_property=decoration-layout)
 * @self: a header bar
 *
 * Gets the decoration layout for @self.
 *
 * Returns: (nullable): the decoration layout
 */
const char *
adap_header_bar_get_decoration_layout (AdapHeaderBar *self)
{
  g_return_val_if_fail (ADAP_IS_HEADER_BAR (self), NULL);

  return self->decoration_layout;
}

/**
 * adap_header_bar_set_decoration_layout: (attributes org.gtk.Method.set_property=decoration-layout)
 * @self: a header bar
 * @layout: (nullable): a decoration layout
 *
 * Sets the decoration layout for @self.
 *
 * If this property is not set, the
 * [property@Gtk.Settings:gtk-decoration-layout] setting is used.
 *
 * The format of the string is button names, separated by commas. A colon
 * separates the buttons that should appear at the start from those at the end.
 * Recognized button names are minimize, maximize, close and icon (the window
 * icon).
 *
 * For example, “icon:minimize,maximize,close” specifies an icon at the start,
 * and minimize, maximize and close buttons at the end.
 */
void
adap_header_bar_set_decoration_layout (AdapHeaderBar *self,
                                      const char   *layout)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));

  if (!g_set_str (&self->decoration_layout, layout))
    return;

  update_decoration_layout (self, TRUE, TRUE);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DECORATION_LAYOUT]);
}

/**
 * adap_header_bar_get_centering_policy: (attributes org.gtk.Method.get_property=centering-policy)
 * @self: a header bar
 *
 * Gets the policy for aligning the center widget.
 *
 * Returns: the centering policy
 */
AdapCenteringPolicy
adap_header_bar_get_centering_policy (AdapHeaderBar *self)
{
  g_return_val_if_fail (ADAP_IS_HEADER_BAR (self), ADAP_CENTERING_POLICY_LOOSE);

  return self->centering_policy;
}

/**
 * adap_header_bar_set_centering_policy: (attributes org.gtk.Method.set_property=centering-policy)
 * @self: a header bar
 * @centering_policy: the centering policy
 *
 * Sets the policy for aligning the center widget.
 */
void
adap_header_bar_set_centering_policy (AdapHeaderBar       *self,
                                     AdapCenteringPolicy  centering_policy)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));

  if (self->centering_policy == centering_policy)
    return;

  self->centering_policy = centering_policy;

  if (self->centering_policy == ADAP_CENTERING_POLICY_STRICT) {
    gtk_size_group_add_widget (self->size_group, self->start_bin);
    gtk_size_group_add_widget (self->size_group, self->end_bin);
  } else {
    gtk_size_group_remove_widget (self->size_group, self->start_bin);
    gtk_size_group_remove_widget (self->size_group, self->end_bin);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CENTERING_POLICY]);
}

/**
 * adap_header_bar_get_show_title: (attributes org.gtk.Method.get_property=show-title)
 * @self: a header bar
 *
 * Gets whether the title widget should be shown.
 *
 * Returns: whether the title widget should be shown.
 *
 * Since: 1.4
 */
gboolean
adap_header_bar_get_show_title (AdapHeaderBar *self)
{
  g_return_val_if_fail (ADAP_IS_HEADER_BAR (self), FALSE);

  return gtk_widget_get_visible (self->center_bin);
}

/**
 * adap_header_bar_set_show_title: (attributes org.gtk.Method.set_property=show-title)
 * @self: a header bar
 * @show_title: whether the title widget is visible
 *
 * Sets whether the title widget should be shown.
 *
 * Since: 1.4
 */
void
adap_header_bar_set_show_title (AdapHeaderBar *self,
                               gboolean      show_title)
{
  g_return_if_fail (ADAP_IS_HEADER_BAR (self));

  show_title = !!show_title;

  if (show_title == adap_header_bar_get_show_title (self))
    return;

  gtk_widget_set_visible (self->center_bin, show_title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_TITLE]);
}
