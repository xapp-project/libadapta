/*
 * Copyright (C) 2020-2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-tab-grid-private.h"

#include "adw-animation-util.h"
#include "adw-easing.h"
#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-tab-overview-private.h"
#include "adw-tab-view-private.h"
#include "adw-timed-animation.h"
#include "adw-widget-utils-private.h"
#include <math.h>

#define SPACING 5
#define DND_THRESHOLD_MULTIPLIER 4
#define DROP_SWITCH_TIMEOUT 500

#define AUTOSCROLL_SPEED 2.5

#define OPEN_ANIMATION_DURATION 200
#define CLOSE_ANIMATION_DURATION 200
#define FOCUS_ANIMATION_DURATION 200
#define RESIZE_ANIMATION_DURATION 200
#define REORDER_ANIMATION_DURATION 250
#define ICON_RESIZE_ANIMATION_DURATION 200

#define MIN_SCALE 0.75
#define SCROLL_PADDING 16

#define MIN_COLUMNS 2
#define MAX_COLUMNS 8

#define MIN_THUMBNAIL_WIDTH 100
#define MAX_THUMBNAIL_WIDTH 500
#define SINGLE_TAB_MAX_PERCENTAGE 0.5

#define SMALL_GRID_WIDTH 360
#define SMALL_GRID_PERCENTAGE 1
#define SMALL_NAT_THUMBNAIL_WIDTH 200

#define LARGE_GRID_WIDTH 2560
#define LARGE_GRID_PERCENTAGE 0.85
#define LARGE_NAT_THUMBNAIL_WIDTH 360

typedef enum {
  TAB_RESIZE_NORMAL,
  TAB_RESIZE_FIXED_TAB_SIZE
} TabResizeMode;

typedef struct {
  GdkDrag *drag;

  AdwTabThumbnail *tab;

  int hotspot_x;
  int hotspot_y;

  int width;
  int height;

  int initial_width;
  int initial_height;

  int target_width;
  int target_height;
  AdwAnimation *resize_animation;
} DragIcon;

typedef struct {
  AdwTabGrid *box;
  AdwTabPage *page;
  AdwTabThumbnail *tab;
  GtkWidget *container;

  int final_x;
  int final_y;
  int final_width;
  int final_height;

  int unshifted_x;
  int unshifted_y;
  int pos_x;
  int pos_y;
  int width;
  int height;
  int last_width;
  int last_height;

  double index;
  double final_index;

  double end_reorder_offset;
  double reorder_offset;

  AdwAnimation *reorder_animation;
  gboolean reorder_ignore_bounds;

  double appear_progress;
  AdwAnimation *appear_animation;

  gboolean visible;
  gboolean is_hidden;
} TabInfo;

struct _AdwTabGrid
{
  GtkWidget parent_instance;

  gboolean pinned;
  AdwTabOverview *tab_overview;
  AdwTabView *view;
  gboolean inverted;

  GtkEventController *view_drop_target;
  GtkGesture *drag_gesture;

  GList *tabs;
  int n_tabs;

  GtkWidget *context_menu;

  int allocated_width;
  int allocated_height;
  int last_height;
  int end_padding;
  int initial_end_padding;
  int final_end_padding;
  TabResizeMode tab_resize_mode;
  AdwAnimation *resize_animation;

  TabInfo *selected_tab;

  gboolean hovering;
  TabInfo *pressed_tab;
  TabInfo *reordered_tab;
  AdwAnimation *reorder_animation;

  int reorder_x;
  int reorder_y;
  int reorder_index;
  int reorder_window_x;
  int reorder_window_y;
  gboolean continue_reorder;
  gboolean indirect_reordering;

  gboolean dragging;
  double drag_offset_x;
  double drag_offset_y;

  guint drag_autoscroll_cb_id;
  gint64 drag_autoscroll_prev_time;

  AdwTabPage *detached_page;
  int detached_index;
  TabInfo *reorder_placeholder;
  AdwTabPage *placeholder_page;
  gboolean can_remove_placeholder;
  DragIcon *drag_icon;
  gboolean should_detach_into_new_window;

  TabInfo *drop_target_tab;
  guint drop_switch_timeout_id;
  guint reset_drop_target_tab_id;
  double drop_target_x;
  double drop_target_y;

  TabInfo *scroll_animation_tab;

  GdkDragAction extra_drag_actions;
  GType *extra_drag_types;
  gsize extra_drag_n_types;
  gboolean extra_drag_preload;

  double n_columns;
  double max_n_columns;
  double initial_max_n_columns;
  int tab_width;
  int tab_height;

  double visible_lower;
  double visible_upper;
  double page_size;
  double lower_inset;
  double upper_inset;

  GtkStringFilter *title_filter;
  GtkStringFilter *tooltip_filter;
  GtkStringFilter *keyword_filter;
  GtkFilter *filter;
  gboolean searching;

  gboolean empty;

  TabInfo *middle_clicked_tab;
};

G_DEFINE_FINAL_TYPE (AdwTabGrid, adw_tab_grid, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_PINNED,
  PROP_TAB_OVERVIEW,
  PROP_VIEW,
  PROP_RESIZE_FROZEN,
  PROP_EMPTY,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_SCROLL_RELATIVE,
  SIGNAL_SCROLL_TO_TAB,
  SIGNAL_EXTRA_DRAG_DROP,
  SIGNAL_EXTRA_DRAG_VALUE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

/* Helpers */

static void
remove_and_free_tab_info (TabInfo *info)
{
  gtk_widget_unparent (GTK_WIDGET (info->container));

  g_free (info);
}

static inline int
get_tab_x (AdwTabGrid *self,
           TabInfo    *info,
           gboolean    final)
{
  if (info == self->reordered_tab)
    return self->reorder_window_x;

  return final ? info->final_x : info->pos_x;
}

static inline int
get_tab_y (AdwTabGrid *self,
           TabInfo    *info,
           gboolean    final)
{
  if (info == self->reordered_tab)
    return self->reorder_window_y;

  return final ? info->final_y : info->pos_y;
}

static inline TabInfo *
find_tab_info_at (AdwTabGrid *self,
                  double      x,
                  double      y)
{
  GList *l;

  if (self->reordered_tab) {
    int pos_x = get_tab_x (self, self->reordered_tab, FALSE);
    int pos_y = get_tab_y (self, self->reordered_tab, FALSE);

    if (pos_x <= x && x < pos_x + self->reordered_tab->width &&
        pos_y <= y && y < pos_y + self->reordered_tab->height)
      return self->reordered_tab;
  }

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (!gtk_widget_should_layout (info->container))
      continue;

    if (info != self->reordered_tab &&
        info->pos_x <= x && x < info->pos_x + info->width &&
        info->pos_y <= y && y < info->pos_y + info->height)
      return info;
  }

  return NULL;
}

static inline GList *
find_link_for_page (AdwTabGrid *self,
                    AdwTabPage *page)
{
  GList *l;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info->page == page)
      return l;
  }

  return NULL;
}

static inline TabInfo *
find_info_for_page (AdwTabGrid *self,
                    AdwTabPage *page)
{
  GList *l = find_link_for_page (self, page);

  return l ? l->data : NULL;
}

static inline GList *
find_link_for_widget (AdwTabGrid *self,
                      GtkWidget  *widget)
{
  GList *l;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info->container == widget)
      return l;
  }

  return NULL;
}

static GList *
find_nth_alive_tab (AdwTabGrid *self,
                    guint       position)
{
  GList *l;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (!info->page)
        continue;

    if (!position--)
        return l;
  }

  return NULL;
}

static int
get_n_visible_tabs (AdwTabGrid *self)
{
  GList *l;
  int ret = 0;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info->page && info->visible)
      ret++;
  }

  return ret;
}

static GList *
find_nth_visible_tab (AdwTabGrid *self,
                      guint       position)
{
  GList *l;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (!info->page)
        continue;

    if (!info->visible)
        continue;

    if (!position--)
        return l;
  }

  return NULL;
}

static TabInfo *
get_focused_info (AdwTabGrid *self)
{
  GtkWidget *focus_child = gtk_widget_get_focus_child (GTK_WIDGET (self));
  GList *l;

  if (!focus_child)
    return NULL;

  l = find_link_for_widget (self, focus_child);

  if (!l || !l->data)
    return NULL;

  return l->data;
}

static int
get_focused_column (AdwTabGrid *self)
{
  TabInfo *info = get_focused_info (self);

  if (!info)
    return -1;

  return (int) round (fmod (info->final_index, self->n_columns));
}

/* Layout */

static inline AdwTabGrid *
get_other_tab_grid (AdwTabGrid *self)
{
  if (self->pinned)
    return adw_tab_overview_get_tab_grid (self->tab_overview);
  else
    return adw_tab_overview_get_pinned_tab_grid (self->tab_overview);
}

static double
get_max_n_columns (AdwTabGrid *self)
{
  GList *l;
  double max_columns = 0;
  double other_max_columns = 0;
  AdwTabGrid *other_grid = get_other_tab_grid (self);
  int n_tabs = 0;
  int other_n_tabs = 0;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    max_columns += info->appear_progress;

    if (info->page)
      n_tabs++;
  }

  for (l = other_grid->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    other_max_columns += info->appear_progress;

    if (info->page)
      other_n_tabs++;
  }

  max_columns = MAX (1, max_columns);
  other_max_columns = MAX (1, other_max_columns);

  /* Let's say we have one pinned and two regular tab, and we pin one of them.
   * During this animation max number of columns goes from 2 back to 2, but
   * dips in the middle of the animation. We want to keep it at 2 the whole
   * animation instead. */
  if ((n_tabs == other_n_tabs + 1 &&
       max_columns < n_tabs &&
       other_max_columns > other_n_tabs) ||
      (other_n_tabs == n_tabs + 1 &&
       max_columns > n_tabs &&
       other_max_columns < other_n_tabs)) {
    return MAX (n_tabs, other_n_tabs);
  }

  return MAX (max_columns, other_max_columns);
}

static double
get_n_columns (AdwTabGrid *self,
               int         for_width,
               double      max_n_columns)
{
  double t;
  double nat_width;

  if (for_width < 0)
    return MAX (max_n_columns, 1);

  max_n_columns = CLAMP (max_n_columns, 1, MAX_COLUMNS);

  t = CLAMP (((double) for_width - SMALL_GRID_WIDTH) /
             (LARGE_GRID_WIDTH - SMALL_GRID_WIDTH), 0, 1);
  nat_width = adw_lerp (SMALL_NAT_THUMBNAIL_WIDTH, LARGE_NAT_THUMBNAIL_WIDTH,
                        adw_easing_ease (ADW_EASE_OUT_CUBIC, t));

  return CLAMP (ceil ((double) for_width / nat_width),
                MIN (MIN_COLUMNS, max_n_columns), max_n_columns);
}

static int
get_tab_width (AdwTabGrid *self,
               int         for_width)
{
  double n = get_n_columns (self, for_width, self->max_n_columns);
  double total_size = for_width;
  double t;
  int ret;

  t = CLAMP ((total_size - SMALL_GRID_WIDTH) / (LARGE_GRID_WIDTH - SMALL_GRID_WIDTH), 0, 1);
  total_size *= adw_lerp (SMALL_GRID_PERCENTAGE, LARGE_GRID_PERCENTAGE,
                          adw_easing_ease (ADW_EASE_OUT_CUBIC, t));

  if (G_APPROX_VALUE (n, self->max_n_columns, DBL_EPSILON) || n < self->max_n_columns) {
    double max = get_n_columns (self, for_width, MAX_COLUMNS);

    total_size *= (SINGLE_TAB_MAX_PERCENTAGE + (1 - SINGLE_TAB_MAX_PERCENTAGE) * n / max);
  }

  ret = (int) ceil ((double) (total_size - SPACING * (n + 1)) / n);

  return CLAMP (ret, MIN_THUMBNAIL_WIDTH, MAX_THUMBNAIL_WIDTH);
}

static int
get_tab_height (AdwTabGrid *self,
                int         tab_width)
{
  int height = 0;
  GList *l;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    int tab_height;

    gtk_widget_measure (GTK_WIDGET (info->tab), GTK_ORIENTATION_VERTICAL,
                        tab_width, NULL, &tab_height, NULL, NULL);

    height = MAX (height, tab_height);
  }

  return height;
}

static void
get_position_for_index (AdwTabGrid *self,
                        double      index,
                        gboolean    is_rtl,
                        int        *pos_x,
                        int        *pos_y)
{
  int n_columns = ceil (self->n_columns);
  double col = fmod (index, n_columns);
  double row = (index - col) / n_columns;
  double offset = self->allocated_width;
  int x, y;

  offset -= self->n_columns * (self->tab_width + SPACING) - SPACING;
  offset /= 2;

  if (col > n_columns - 1) {
    double start, end, t;

    if (is_rtl) {
      start = self->allocated_width - offset - self->tab_width;
      end = offset;
    } else {
      start = offset;
      end = self->allocated_width - offset - self->tab_width;
    }

    t = n_columns - col;
    x = adw_lerp (start, end, t);
    y = SPACING + (row + 1 - t) * (self->tab_height + SPACING);
  } else {
    x = is_rtl ? self->allocated_width - offset - self->tab_width : offset;

    if (is_rtl)
      x -= col * (self->tab_width + SPACING);
    else
      x += col * (self->tab_width + SPACING);

    y = SPACING + row * (self->tab_height + SPACING);
  }

  if (pos_x)
    *pos_x = x;

  if (pos_y)
    *pos_y = y;
}

static inline int
calculate_tab_width (TabInfo *info,
                     int      base_width)
{
  return (int) floor ((base_width + SPACING) * info->appear_progress) - SPACING;
}

static void
measure_tab_grid (AdwTabGrid     *self,
                  GtkOrientation  orientation,
                  int             for_size,
                  int            *minimum,
                  int            *natural,
                  gboolean        animated)
{
  GList *l;
  int min, nat;

  min = nat = 0;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;
      int child_min, child_nat;

      if (!gtk_widget_should_layout (info->container))
        continue;

      gtk_widget_measure (info->container, orientation, -1,
                          &child_min, &child_nat, NULL, NULL);

      if (animated)
        min = MAX (min, calculate_tab_width (info, child_min));
      else
        min = MAX (min, child_min) + SPACING;

      nat += child_nat + SPACING;
    }

    nat += SPACING;
    min += SPACING;
  } else {
    double n_columns, n_rows;
    int child_width = -1, child_height;
    double index = 0;
    int height;

    if (for_size >= 0)
      child_width = get_tab_width (self, for_size);

    child_height = get_tab_height (self, child_width);

    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;

      if (!gtk_widget_should_layout (info->container))
        continue;

      if (animated) {
        index += info->appear_progress;
      } else {
        if (info->page)
          index++;
      }
    }

    n_columns = get_n_columns (self, for_size, self->max_n_columns);
    n_rows = ceil (index / n_columns);

    if (animated) {
      double col = fmod (index, n_columns);

      if (col > 0 && col < 1)
        n_rows = n_rows + col - 1;
    }

    if (n_rows < 1)
      height = (child_height + SPACING * 2) * n_rows + self->end_padding;
    else
      height = (child_height + SPACING) * n_rows + SPACING + self->end_padding;

    if (!self->pinned)
      height = MAX (self->last_height, height);

    min = MAX (min, height);
    nat = MAX (nat, height);
  }

  if (minimum)
    *minimum = min;

  if (natural)
    *natural = nat;
}

static void
calculate_tab_layout (AdwTabGrid *self)
{
  gboolean is_rtl;
  GList *l;
  double index = 0, final_index = 0;

  if (self->tab_resize_mode != TAB_RESIZE_FIXED_TAB_SIZE &&
      self->initial_max_n_columns < 0)
    self->max_n_columns = get_max_n_columns (self);

  self->n_columns = get_n_columns (self, self->allocated_width, self->max_n_columns);

  if (self->context_menu)
    gtk_popover_present (GTK_POPOVER (self->context_menu));

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  self->tab_width = get_tab_width (self, self->allocated_width);
  self->tab_height = get_tab_height (self, self->tab_width);

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (!gtk_widget_should_layout (info->container))
      continue;

    get_position_for_index (self, final_index, is_rtl,
                            &info->unshifted_x, &info->unshifted_y);
    get_position_for_index (self, index + info->reorder_offset, is_rtl,
                            &info->pos_x, &info->pos_y);
    get_position_for_index (self, final_index + info->end_reorder_offset, is_rtl,
                            &info->final_x, &info->final_y);

    info->width = self->tab_width;
    info->final_width = self->tab_width;

    info->height = self->tab_height;
    info->final_height = self->tab_height;

    info->index = index;
    info->final_index = final_index;

    index += info->appear_progress;
    final_index++;

    if (self->tab_resize_mode == TAB_RESIZE_FIXED_TAB_SIZE) {
      self->end_padding = self->allocated_height - info->pos_y - info->height - SPACING;
      self->final_end_padding = self->allocated_height - info->final_y - info->final_height - SPACING;
    }
  }
}

static void
get_visible_range (AdwTabGrid *self,
                   int        *lower,
                   int        *upper)
{
  int min = SPACING;
  int max = self->allocated_height - SPACING;

  min = MAX (min, (int) floor (self->visible_lower) + SPACING);
  max = MIN (max, (int) ceil (self->visible_upper) - SPACING);

  if (lower)
    *lower = min;

  if (upper)
    *upper = max;
}

static inline gboolean
is_touchscreen (GtkGesture *gesture)
{
  GtkEventController *controller = GTK_EVENT_CONTROLLER (gesture);
  GdkDevice *device = gtk_event_controller_get_current_event_device (controller);
  GdkInputSource input_source = gdk_device_get_source (device);

  return input_source == GDK_SOURCE_TOUCHSCREEN;
}

/* Search */

static gboolean
tab_should_be_visible (AdwTabGrid *self,
                       AdwTabPage *page)
{
  if (!self->searching)
    return TRUE;

  return gtk_filter_match (self->filter, page);
}

static void
set_empty (AdwTabGrid *self,
           gboolean    empty)
{
  if (self->empty == empty)
    return;

  self->empty = empty;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EMPTY]);
}

static void
search_changed_cb (AdwTabGrid      *self,
                   GtkFilterChange  change)
{
  GList *l;
  gboolean changed = FALSE;
  gboolean empty = TRUE;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    gboolean visible;

    if (change == GTK_FILTER_CHANGE_LESS_STRICT && info->visible) {
      empty = FALSE;
      continue;
    }

    if (change == GTK_FILTER_CHANGE_MORE_STRICT && !info->visible)
      continue;

    visible = tab_should_be_visible (self, info->page);

    if (visible)
      empty = FALSE;

    if (visible != info->visible) {
      info->visible = visible;
      gtk_widget_set_visible (info->container, visible);
      changed = TRUE;
    }
  }

  set_empty (self, empty);

  if (changed)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

/* Tab resize delay */

static void
resize_animation_value_cb (double      value,
                           AdwTabGrid *self)
{
  double target_max_n_columns = get_max_n_columns (self);

  self->end_padding = (int) floor (adw_lerp (self->initial_end_padding, 0, value));

  self->max_n_columns = adw_lerp (self->initial_max_n_columns, target_max_n_columns, value);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
resize_animation_done_cb (AdwTabGrid *self)
{
  self->end_padding = 0;
  self->final_end_padding = 0;
  self->initial_max_n_columns = -1;
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
set_tab_resize_mode_do (AdwTabGrid    *self,
                        TabResizeMode  mode)
{
  gboolean notify;

  if (self->tab_resize_mode == mode)
    return;

  if (mode == TAB_RESIZE_FIXED_TAB_SIZE) {
    GList *l;

    self->last_height = self->allocated_height;

    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;

      if (info->appear_animation)
        info->last_height = info->final_height;
      else
        info->last_height = info->height;
    }
  } else {
    self->last_height = 0;
  }

  if (mode == TAB_RESIZE_NORMAL) {
    self->initial_end_padding = self->end_padding;
    self->initial_max_n_columns = self->max_n_columns;

    adw_animation_play (self->resize_animation);
  }

  notify = (self->tab_resize_mode == TAB_RESIZE_NORMAL) !=
           (mode == TAB_RESIZE_NORMAL);

  self->tab_resize_mode = mode;

  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_RESIZE_FROZEN]);
}

static void
set_tab_resize_mode (AdwTabGrid    *self,
                     TabResizeMode  mode)
{
  set_tab_resize_mode_do (self, mode);
  set_tab_resize_mode_do (get_other_tab_grid (self), mode);
}

/* Hover */

static void
update_hover (AdwTabGrid *self)
{
  if (!self->dragging && !self->hovering)
    set_tab_resize_mode (self, TAB_RESIZE_NORMAL);
}

/* Keybindings */

static void
reorder_tab_cb (AdwTabGrid *self,
                GVariant   *args)
{
  GtkDirectionType direction;
  gboolean success = FALSE;
  TabInfo *info = get_focused_info (self);

  if (!self->view || !info || !info->page || self->searching)
    return;

  g_variant_get (args, "h", &direction);

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL) {
    if (direction == GTK_DIR_LEFT)
      direction = GTK_DIR_RIGHT;
    else if (direction == GTK_DIR_RIGHT)
      direction = GTK_DIR_LEFT;
  }

  if (direction == GTK_DIR_LEFT) {
    success = adw_tab_view_reorder_backward (self->view, info->page);
  } else if (direction == GTK_DIR_RIGHT) {
    success = adw_tab_view_reorder_forward (self->view, info->page);
  } else if (direction == GTK_DIR_UP) {
    int position = adw_tab_view_get_page_position (self->view, info->page);
    position -= self->n_columns;

    if (position >= adw_tab_view_get_n_pinned_pages (self->view) ||
        (self->pinned && position >= 0))
      success = adw_tab_view_reorder_page (self->view, info->page, position);
  } else if (direction == GTK_DIR_DOWN) {
    int position = adw_tab_view_get_page_position (self->view, info->page);
    position += self->n_columns;

    if ((self->pinned && position < adw_tab_view_get_n_pinned_pages (self->view)) ||
        (!self->pinned && position < adw_tab_view_get_n_pages (self->view)))
    success = adw_tab_view_reorder_page (self->view, info->page, position);
  }

  if (!success)
    gtk_widget_error_bell (GTK_WIDGET (self));
}

static void
add_reorder_bindings (GtkWidgetClass   *widget_class,
                      guint             keysym,
                      GtkDirectionType  direction)
{
  /* All keypad keysyms are aligned at the same order as non-keypad ones */
  guint keypad_keysym = keysym - GDK_KEY_Left + GDK_KEY_KP_Left;

  gtk_widget_class_add_binding (widget_class, keysym, GDK_SHIFT_MASK,
                                (GtkShortcutFunc) reorder_tab_cb,
                                "h", direction);
  gtk_widget_class_add_binding (widget_class, keypad_keysym, GDK_SHIFT_MASK,
                                (GtkShortcutFunc) reorder_tab_cb,
                                "h", direction);
}

static void
activate_tab (AdwTabGrid *self)
{
  TabInfo *info = get_focused_info (self);

  if (!info || !info->page)
    return;

  adw_tab_view_set_selected_page (self->view, info->page);
  adw_tab_overview_set_open (self->tab_overview, FALSE);
}

/* Scrolling */

static void
drop_switch_timeout_cb (AdwTabGrid *self)
{
  self->drop_switch_timeout_id = 0;
  adw_tab_view_set_selected_page (self->view,
                                  self->drop_target_tab->page);
}

static void
set_drop_target_tab (AdwTabGrid *self,
                     TabInfo    *info)
{
  if (self->drop_target_tab == info)
    return;

  if (self->drop_target_tab)
    g_clear_handle_id (&self->drop_switch_timeout_id, g_source_remove);

  self->drop_target_tab = info;

  if (self->drop_target_tab) {
    self->drop_switch_timeout_id =
      g_timeout_add_once (DROP_SWITCH_TIMEOUT,
                          (GSourceOnceFunc) drop_switch_timeout_cb,
                          self);
  }
}

static void
animate_scroll_relative (AdwTabGrid *self,
                         double      delta,
                         guint       duration)
{
  g_signal_emit (self, signals[SIGNAL_SCROLL_RELATIVE], 0, delta, duration);
}

static void
scroll_to_tab_full (AdwTabGrid *self,
                    TabInfo    *info,
                    double      pos,
                    guint       duration,
                    gboolean    keep_selected_visible)
{
  self->scroll_animation_tab = info;

  int tab_height;
  double padding, offset;

  tab_height = info->final_height;

  padding = MIN (SCROLL_PADDING, self->page_size / 2);

  if (pos < 0)
    pos = get_tab_y (self, info, TRUE);

  if (pos - SPACING < self->visible_lower)
    offset = -padding;
  else if (pos + tab_height + SPACING > self->visible_upper)
    offset = tab_height + padding - self->page_size;
  else
    return;

  g_signal_emit (self, signals[SIGNAL_SCROLL_TO_TAB], 0, offset, duration);
}

static void
scroll_to_tab (AdwTabGrid *self,
               TabInfo    *info,
               guint       duration)
{
  scroll_to_tab_full (self, info, -1, duration, FALSE);
}

/* Reordering */

static void
force_end_reordering (AdwTabGrid *self)
{
  GList *l;

  if (self->dragging || !self->reordered_tab)
    return;

  if (self->reorder_animation)
    adw_animation_skip (self->reorder_animation);

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info->reorder_animation)
      adw_animation_skip (info->reorder_animation);
  }
}

static void
check_end_reordering (AdwTabGrid *self)
{
  GList *l;

  if (self->dragging || !self->reordered_tab || self->continue_reorder)
    return;

  if (self->reorder_animation)
    return;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info->reorder_animation)
      return;
  }

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    info->end_reorder_offset = 0;
    info->reorder_offset = 0;
  }

  self->reordered_tab->reorder_ignore_bounds = FALSE;

  self->tabs = g_list_remove (self->tabs, self->reordered_tab);
  self->tabs = g_list_insert (self->tabs, self->reordered_tab, self->reorder_index);

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  self->reordered_tab = NULL;
}

static void
start_reordering (AdwTabGrid *self,
                  TabInfo    *info)
{
  self->reordered_tab = info;

  /* The reordered tab should be displayed above everything else */
  gtk_widget_insert_before (GTK_WIDGET (self->reordered_tab->container),
                            GTK_WIDGET (self), NULL);

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
get_reorder_position (AdwTabGrid *self,
                      int        *x,
                      int        *y)
{
  int lower, upper;
  int width;

  if (self->reordered_tab->reorder_ignore_bounds) {
    *x = self->reorder_x;
    *y = self->reorder_y;
    return;
  }

  get_visible_range (self, &lower, &upper);

  width = gtk_widget_get_width (GTK_WIDGET (self));

  *x = CLAMP (self->reorder_x, 0, width - self->reordered_tab->width);
  *y = CLAMP (self->reorder_y, lower, upper - self->reordered_tab->height);
}

static void
reorder_animation_value_cb (double   value,
                            TabInfo *dest_tab)
{
  AdwTabGrid *self = dest_tab->box;
  gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  int x1, y1, x2, y2;

  get_reorder_position (self, &x1, &y1);
  get_position_for_index (self, dest_tab->index, is_rtl, &x2, &y2);

  self->reorder_window_x = (int) round (adw_lerp (x1, x2, value));
  self->reorder_window_y = (int) round (adw_lerp (y1, y2, value));

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
reorder_animation_done_cb (AdwTabGrid *self)
{
  g_clear_object (&self->reorder_animation);
  check_end_reordering (self);
}

static void
animate_reordering (AdwTabGrid *self,
                    TabInfo    *dest_tab)
{
  AdwAnimationTarget *target;

  if (self->reorder_animation)
    adw_animation_skip (self->reorder_animation);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              reorder_animation_value_cb,
                                              dest_tab, NULL);
  self->reorder_animation =
    adw_timed_animation_new (GTK_WIDGET (self), 0, 1,
                             REORDER_ANIMATION_DURATION, target);

  g_signal_connect_swapped (self->reorder_animation, "done",
                            G_CALLBACK (reorder_animation_done_cb), self);

  adw_animation_play (self->reorder_animation);

  check_end_reordering (self);
}

static void
reorder_offset_animation_value_cb (double   value,
                                   TabInfo *info)
{
  AdwTabGrid *self = info->box;

  info->reorder_offset = value;
  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
reorder_offset_animation_done_cb (TabInfo *info)
{
  AdwTabGrid *self = info->box;

  g_clear_object (&info->reorder_animation);
  check_end_reordering (self);
}

static void
animate_reorder_offset (AdwTabGrid *self,
                        TabInfo    *info,
                        double      offset)
{
  gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  AdwAnimationTarget *target;
  double start_offset;

  offset *= (is_rtl ? -1 : 1);

  if (G_APPROX_VALUE (info->end_reorder_offset, offset, DBL_EPSILON))
    return;

  info->end_reorder_offset = offset;
  start_offset = info->reorder_offset;

  if (info->reorder_animation)
    adw_animation_skip (info->reorder_animation);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              reorder_offset_animation_value_cb,
                                              info, NULL);
  info->reorder_animation =
    adw_timed_animation_new (GTK_WIDGET (self), start_offset, offset,
                             REORDER_ANIMATION_DURATION, target);

  g_signal_connect_swapped (info->reorder_animation, "done",
                            G_CALLBACK (reorder_offset_animation_done_cb), info);

  adw_animation_play (info->reorder_animation);
}

static void
reset_reorder_animations (AdwTabGrid *self)
{
  int i, original_index;
  GList *l;

  if (!adw_get_enable_animations (GTK_WIDGET (self)))
      return;

  l = find_link_for_page (self, self->reordered_tab->page);
  original_index = g_list_position (self->tabs, l);

  if (self->reorder_index > original_index)
    for (i = 0; i < self->reorder_index - original_index; i++) {
      l = l->next;
      animate_reorder_offset (self, l->data, 0);
    }

  if (self->reorder_index < original_index)
    for (i = 0; i < original_index - self->reorder_index; i++) {
      l = l->prev;
      animate_reorder_offset (self, l->data, 0);
    }
}

static void
page_reordered_cb (AdwTabGrid *self,
                   AdwTabPage *page,
                   int         index)
{
  GList *link;
  int original_index;
  TabInfo *info, *dest_tab;
  gboolean is_rtl;

  if (adw_tab_page_get_pinned (page) != self->pinned)
    return;

  self->continue_reorder = self->reordered_tab && page == self->reordered_tab->page;

  if (self->continue_reorder)
    reset_reorder_animations (self);
  else
    force_end_reordering (self);

  link = find_link_for_page (self, page);
  info = link->data;
  original_index = g_list_position (self->tabs, link);

  if (!self->continue_reorder)
    start_reordering (self, info);

  if (self->continue_reorder) {
    self->reorder_x = self->reorder_window_x;
    self->reorder_y = self->reorder_window_y;
  } else {
    self->reorder_x = info->pos_x;
    self->reorder_y = info->pos_y;
  }

  self->reorder_index = index;

  if (!self->pinned)
    self->reorder_index -= adw_tab_view_get_n_pinned_pages (self->view);

  dest_tab = g_list_nth_data (self->tabs, self->reorder_index);

  if (info == self->selected_tab)
    scroll_to_tab_full (self, self->selected_tab, dest_tab->final_y, REORDER_ANIMATION_DURATION, FALSE);

  animate_reordering (self, dest_tab);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  /* If animations are disabled, animate_reordering() animation will have
   * already finished and called check_end_reordering () by this point, so
   * it's too late to animate these, so we get a crash.
   */

  if (adw_get_enable_animations (GTK_WIDGET (self)) &&
      gtk_widget_get_mapped (GTK_WIDGET (self))) {
    int i;

    if (self->reorder_index > original_index)
      for (i = 0; i < self->reorder_index - original_index; i++) {
        link = link->next;
        animate_reorder_offset (self, link->data, is_rtl ? 1 : -1);
      }

    if (self->reorder_index < original_index)
      for (i = 0; i < original_index - self->reorder_index; i++) {
        link = link->prev;
        animate_reorder_offset (self, link->data, is_rtl ? -1 : 1);
      }
  }

  self->continue_reorder = FALSE;
}

static void
update_drag_reodering (AdwTabGrid *self)
{
  gboolean is_rtl;
  int old_index = -1, new_index = -1;
  int x, y;
  int i = 0;
  int width, height;
  GList *l;

  if (!self->dragging)
    return;

  get_reorder_position (self, &x, &y);

  width = self->reordered_tab->final_width;
  height = self->reordered_tab->final_height;

  self->reorder_window_x = x;
  self->reorder_window_y = y;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    int center_x, center_y;

    center_x = info->unshifted_x + info->final_width / 2;
    center_y = info->unshifted_y + info->final_height / 2;

    if (is_rtl)
      center_x -= info->final_width;

    if (info == self->reordered_tab)
      old_index = i;

    if (x + width  + SPACING > center_x && center_x >= x - SPACING &&
        y + height + SPACING > center_y && center_y >= y - SPACING &&
        new_index < 0)
      new_index = i;

    if (old_index >= 0 && new_index >= 0)
      break;

    i++;
  }

  if (new_index < 0)
    new_index = g_list_length (self->tabs) - 1;

  i = 0;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    double offset = 0;

    if (i > old_index && i <= new_index)
      offset = is_rtl ? 1 : -1;

    if (i < old_index && i >= new_index)
      offset = is_rtl ? -1 : 1;

    i++;

    animate_reorder_offset (self, info, offset);
  }

  self->reorder_index = new_index;
}

static gboolean
drag_autoscroll_cb (GtkWidget     *widget,
                    GdkFrameClock *frame_clock,
                    AdwTabGrid    *self)
{
  double y, delta_ms, start_threshold, end_threshold, autoscroll_factor;
  gint64 time;
  int offset = 0;
  int tab_height = 0;
  int autoscroll_area = 0;

  if (G_APPROX_VALUE (self->visible_upper - self->visible_lower, self->allocated_height, DBL_EPSILON) ||
      self->visible_upper - self->visible_lower > self->allocated_height)
    return G_SOURCE_CONTINUE;

  if (self->reordered_tab) {
    tab_height = self->reordered_tab->height;
    y = (double) self->reorder_y - SPACING;
  } else if (self->drop_target_tab) {
    tab_height = self->drop_target_tab->height;
    y = (double) self->drop_target_y - tab_height / 2;
  } else {
    return G_SOURCE_CONTINUE;
  }

  autoscroll_area = tab_height / 4;

  y = CLAMP (y,
             autoscroll_area,
             self->allocated_height - tab_height - autoscroll_area);

  time = gdk_frame_clock_get_frame_time (frame_clock);
  delta_ms = (time - self->drag_autoscroll_prev_time) / 1000.0;

  start_threshold = self->visible_lower + autoscroll_area;
  end_threshold = self->visible_upper - tab_height - autoscroll_area;

  autoscroll_factor = 0;

  if (y < start_threshold)
    autoscroll_factor = -(start_threshold - y) / autoscroll_area;
  else if (y > end_threshold)
    autoscroll_factor = (y - end_threshold) / autoscroll_area;

  autoscroll_factor = CLAMP (autoscroll_factor, -1, 1);
  autoscroll_factor = adw_easing_ease (ADW_EASE_IN_CUBIC, autoscroll_factor);
  self->drag_autoscroll_prev_time = time;

  if (G_APPROX_VALUE (autoscroll_factor, 0, DBL_EPSILON))
    return G_SOURCE_CONTINUE;

  if (autoscroll_factor > 0)
    offset = (int) ceil (autoscroll_factor * delta_ms * AUTOSCROLL_SPEED);
  else
    offset = (int) floor (autoscroll_factor * delta_ms * AUTOSCROLL_SPEED);

  self->reorder_y += offset;
  animate_scroll_relative (self, offset, 0);
  update_drag_reodering (self);

  return G_SOURCE_CONTINUE;
}

static void
start_autoscroll (AdwTabGrid *self)
{
  GdkFrameClock *frame_clock;

  if (self->drag_autoscroll_cb_id)
    return;

  frame_clock = gtk_widget_get_frame_clock (GTK_WIDGET (self));

  self->drag_autoscroll_prev_time = gdk_frame_clock_get_frame_time (frame_clock);
  self->drag_autoscroll_cb_id =
    gtk_widget_add_tick_callback (GTK_WIDGET (self),
                                  (GtkTickCallback) drag_autoscroll_cb,
                                  self, NULL);
}

static void
end_autoscroll (AdwTabGrid *self)
{
  if (self->drag_autoscroll_cb_id) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self),
                                     self->drag_autoscroll_cb_id);
    self->drag_autoscroll_cb_id = 0;
  }
}

static void
start_drag_reodering (AdwTabGrid *self,
                      TabInfo    *info,
                      double      x,
                      double      y)
{
  if (self->dragging)
    return;

  if (self->searching)
    return;

  if (!info)
    return;

  self->continue_reorder = info == self->reordered_tab;

  if (self->continue_reorder) {
    if (self->reorder_animation)
      adw_animation_skip (self->reorder_animation);

    reset_reorder_animations (self);

    self->reorder_x = (int) round (x - self->drag_offset_x);
    self->reorder_y = (int) round (y - self->drag_offset_y);
  } else
    force_end_reordering (self);

  start_autoscroll (self);
  self->dragging = TRUE;

  if (!self->continue_reorder)
    start_reordering (self, info);
}

static void
end_drag_reodering (AdwTabGrid *self)
{
  TabInfo *dest_tab;

  if (!self->dragging)
    return;

  self->dragging = FALSE;

  end_autoscroll (self);

  dest_tab = g_list_nth_data (self->tabs, self->reorder_index);

  if (!self->indirect_reordering) {
    int index = self->reorder_index;

    if (!self->pinned)
      index += adw_tab_view_get_n_pinned_pages (self->view);

    /* We've already reordered the tab here, no need to do it again */
    g_signal_handlers_block_by_func (self->view, page_reordered_cb, self);

    adw_tab_view_reorder_page (self->view, self->reordered_tab->page, index);

    g_signal_handlers_unblock_by_func (self->view, page_reordered_cb, self);
  }

  animate_reordering (self, dest_tab);

  self->continue_reorder = FALSE;
}

static void
reorder_begin_cb (AdwTabGrid *self,
                  double      start_x,
                  double      start_y,
                  GtkGesture *gesture)
{
  self->pressed_tab = find_tab_info_at (self, start_x, start_y);

  if (!self->pressed_tab)
    return;

  self->drag_offset_x = start_x - get_tab_x (self, self->pressed_tab, FALSE);
  self->drag_offset_y = start_y - get_tab_y (self, self->pressed_tab, FALSE);

  if (!self->reorder_animation) {
    self->reorder_x = (int) round (start_x - self->drag_offset_x);
    self->reorder_y = (int) round (start_y - self->drag_offset_y);
  }
}

/* Copied from gtkdragsource.c */
static gboolean
gtk_drag_check_threshold_double (GtkWidget *widget,
                                 double     start_x,
                                 double     start_y,
                                 double     current_x,
                                 double     current_y)
{
  int drag_threshold;

  g_object_get (gtk_widget_get_settings (widget),
                "gtk-dnd-drag-threshold", &drag_threshold,
                NULL);

  return (ABS (current_x - start_x) > drag_threshold ||
          ABS (current_y - start_y) > drag_threshold);
}

static gboolean
check_dnd_threshold (AdwTabGrid *self,
                     double      x,
                     double      y)
{
  int threshold;
  graphene_rect_t rect;

  g_object_get (gtk_widget_get_settings (GTK_WIDGET (self)),
                "gtk-dnd-drag-threshold", &threshold,
                NULL);

  threshold *= DND_THRESHOLD_MULTIPLIER;

  graphene_rect_init (&rect, 0, 0,
                      gtk_widget_get_width (GTK_WIDGET (self)),
                      self->allocated_height);
  graphene_rect_inset (&rect, -threshold, -threshold);

  return !graphene_rect_contains_point (&rect, &GRAPHENE_POINT_INIT (x, y));
}

static void begin_drag (AdwTabGrid *self,
                        GdkDevice  *device);

static void
reorder_update_cb (AdwTabGrid *self,
                   double      offset_x,
                   double      offset_y,
                   GtkGesture *gesture)
{
  double start_x, start_y, x, y;
  GdkDevice *device;

  if (!self->pressed_tab || !self->pressed_tab->page) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  if (!self->dragging &&
      !gtk_drag_check_threshold_double (GTK_WIDGET (self), 0, 0,
                                        offset_x, offset_y))
    return;

  gtk_gesture_drag_get_start_point (GTK_GESTURE_DRAG (gesture),
                                    &start_x, &start_y);

  x = start_x + offset_x;
  y = start_y + offset_y;

  start_drag_reodering (self, self->pressed_tab, x, y);

  if (self->dragging) {
    adw_tab_view_set_selected_page (self->view, self->pressed_tab->page);
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
  } else {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  self->reorder_x = (int) round (x - self->drag_offset_x);
  self->reorder_y = (int) round (y - self->drag_offset_y);

  device = gtk_event_controller_get_current_event_device (GTK_EVENT_CONTROLLER (gesture));

  if (!self->pinned &&
      self->pressed_tab != self->reorder_placeholder &&
      !is_touchscreen (gesture) &&
      check_dnd_threshold (self, x, y)) {
    begin_drag (self, device);

    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  update_drag_reodering (self);
}

static void
reorder_end_cb (AdwTabGrid *self,
                double      offset_x,
                double      offset_y,
                GtkGesture *gesture)
{
  end_drag_reodering (self);
}

/* Selection */

static void
reset_focus (AdwTabGrid *self)
{
  gtk_widget_set_focus_child (GTK_WIDGET (self), NULL);
}

static void
select_page (AdwTabGrid *self,
             AdwTabPage *page)
{
  if (!page) {
    self->selected_tab = NULL;

    reset_focus (self);

    return;
  }

  self->selected_tab = find_info_for_page (self, page);

  if (!self->selected_tab) {
    if (gtk_widget_get_focus_child (GTK_WIDGET (self)))
      reset_focus (self);

    return;
  }

  gtk_widget_grab_focus (self->selected_tab->container);

  gtk_widget_set_focus_child (GTK_WIDGET (self),
                              self->selected_tab->container);

  if (self->selected_tab != self->reordered_tab &&
      self->selected_tab->width >= 0)
    scroll_to_tab (self, self->selected_tab, FOCUS_ANIMATION_DURATION);
}

/* Opening */

static gboolean
extra_drag_drop_cb (AdwTabThumbnail *tab,
                    GValue          *value,
                    GdkDragAction    preferred_action,
                    AdwTabGrid      *self)
{
  gboolean ret = GDK_EVENT_PROPAGATE;
  AdwTabPage *page = adw_tab_thumbnail_get_page (tab);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_DROP], 0, page, value, preferred_action, &ret);

  return ret;
}

static GdkDragAction
extra_drag_value_cb (AdwTabThumbnail *tab,
                     GValue          *value,
                     AdwTabGrid      *self)
{
  GdkDragAction preferred_action;
  AdwTabPage *page = adw_tab_thumbnail_get_page (tab);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_VALUE], 0, page, value, &preferred_action);

  return preferred_action;
}

static void
appear_animation_value_cb (double   value,
                           TabInfo *info)
{
  info->appear_progress = value;

  if (!info->is_hidden)
    gtk_widget_set_opacity (info->container, info->appear_progress);

  if (GTK_IS_WIDGET (info->container))
    gtk_widget_queue_resize (info->container);
}

static void
open_animation_done_cb (TabInfo *info)
{
  g_clear_object (&info->appear_animation);
}

static void
measure_tab (AdwGizmo       *widget,
             GtkOrientation  orientation,
             int             for_size,
             int            *minimum,
             int            *natural,
             int            *minimum_baseline,
             int            *natural_baseline)
{
  GtkWidget *child = gtk_widget_get_first_child (GTK_WIDGET (widget));

  gtk_widget_measure (child, orientation, for_size,
                      minimum, natural,
                      minimum_baseline,  natural_baseline);

  if (orientation == GTK_ORIENTATION_HORIZONTAL && minimum)
    *minimum = 0;
}

static void
allocate_tab (AdwGizmo *widget,
              int       width,
              int       height,
              int       baseline)
{
  TabInfo *info = g_object_get_data (G_OBJECT (widget), "info");
  GtkWidget *child = gtk_widget_get_first_child (GTK_WIDGET (widget));
  int widget_width = gtk_widget_get_width (GTK_WIDGET (widget));
  int width_diff = MAX (0, info->final_width - widget_width);

  gtk_widget_allocate (child, width + width_diff, height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (-width_diff / 2, 0)));
}

static gboolean
focus_tab (AdwGizmo         *widget,
           GtkDirectionType  direction)
{
  return gtk_widget_grab_focus (GTK_WIDGET (widget));
}

static TabInfo *
create_tab_info (AdwTabGrid *self,
                 AdwTabPage *page)
{
  TabInfo *info;

  info = g_new0 (TabInfo, 1);
  info->box = self;
  info->page = page;
  info->unshifted_x = -1;
  info->unshifted_y = -1;
  info->pos_x = -1;
  info->pos_y = -1;
  info->width = -1;
  info->height = -1;
  info->visible = tab_should_be_visible (self, page);
  info->container = adw_gizmo_new ("tabgridchild", measure_tab, allocate_tab,
                                   NULL, NULL,
                                   focus_tab,
                                   (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_self);
  gtk_widget_set_visible (info->container, info->visible);
  info->tab = adw_tab_thumbnail_new (self->view, self->pinned);

  g_object_set_data (G_OBJECT (info->container), "info", info);
  gtk_widget_set_overflow (info->container, GTK_OVERFLOW_HIDDEN);
  gtk_widget_set_focusable (info->container, TRUE);

  adw_tab_thumbnail_set_page (info->tab, page);
  adw_tab_thumbnail_set_inverted (info->tab, self->inverted);
  adw_tab_thumbnail_setup_extra_drop_target (info->tab,
                                             self->extra_drag_actions,
                                             self->extra_drag_types,
                                             self->extra_drag_n_types);
  adw_tab_thumbnail_set_extra_drag_preload (info->tab, self->extra_drag_preload);

  gtk_widget_set_parent (GTK_WIDGET (info->tab), info->container);
  gtk_widget_insert_before (info->container, GTK_WIDGET (self), NULL);

  g_signal_connect_object (info->tab, "extra-drag-drop", G_CALLBACK (extra_drag_drop_cb), self, 0);
  g_signal_connect_object (info->tab, "extra-drag-value", G_CALLBACK (extra_drag_value_cb), self, 0);

  return info;
}

static void
page_attached_cb (AdwTabGrid *self,
                  AdwTabPage *page,
                  int         position)
{
  AdwAnimationTarget *target;
  TabInfo *info;
  GList *l;

  if (adw_tab_page_get_pinned (page) != self->pinned)
    return;

  if (!self->pinned)
    position -= adw_tab_view_get_n_pinned_pages (self->view);

  set_tab_resize_mode (self, TAB_RESIZE_NORMAL);
  force_end_reordering (self);

  info = create_tab_info (self, page);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              appear_animation_value_cb,
                                              info, NULL);
  info->appear_animation =
    adw_timed_animation_new (GTK_WIDGET (self), 0, 1,
                             OPEN_ANIMATION_DURATION, target);

  g_signal_connect_swapped (info->appear_animation, "done",
                            G_CALLBACK (open_animation_done_cb), info);

  l = find_nth_alive_tab (self, position);
  self->tabs = g_list_insert_before (self->tabs, l, info);

  self->n_tabs++;

  if (!self->searching)
    set_empty (self, FALSE);

  adw_animation_play (info->appear_animation);

  calculate_tab_layout (self);

  if (page == adw_tab_view_get_selected_page (self->view)) {
    adw_tab_grid_select_page (self, page);
  } else {
    int pos = -1;

    if (l && l->next && l->next->data) {
      TabInfo *next_info = l->next->data;

      pos = next_info->final_y;

      scroll_to_tab_full (self, info, pos, OPEN_ANIMATION_DURATION, TRUE);
    }
  }
}

/* Closing */

static void
close_animation_done_cb (TabInfo *info)
{
  AdwTabGrid *self = info->box;

  g_clear_object (&info->appear_animation);

  self->tabs = g_list_remove (self->tabs, info);

  if (info->reorder_animation)
    adw_animation_skip (info->reorder_animation);

  if (self->reorder_animation)
    adw_animation_skip (self->reorder_animation);

  if (self->pressed_tab == info)
    self->pressed_tab = NULL;

  if (self->reordered_tab == info)
    self->reordered_tab = NULL;

  if (self->middle_clicked_tab == info)
    self->middle_clicked_tab = NULL;

  remove_and_free_tab_info (info);

  self->n_tabs--;

  if (self->n_tabs == 0 || (self->searching && get_n_visible_tabs (self) == 0))
    set_empty (self, TRUE);
}

static void
page_detached_cb (AdwTabGrid *self,
                  AdwTabPage *page)
{
  AdwAnimationTarget *target;
  TabInfo *info;
  GList *page_link;

  page_link = find_link_for_page (self, page);

  if (!page_link)
    return;

  info = page_link->data;
  page_link = page_link->next;

  force_end_reordering (self);

  if (self->hovering) {
    gboolean is_last = TRUE;

    while (page_link) {
      TabInfo *i = page_link->data;
      page_link = page_link->next;

      if (i->page) {
        is_last = FALSE;
        break;
      }
    }

    if (is_last && !self->pinned)
      set_tab_resize_mode (self, TAB_RESIZE_NORMAL);
    else
      set_tab_resize_mode (self, TAB_RESIZE_FIXED_TAB_SIZE);
  }

  g_assert (info->page);

  if (gtk_widget_is_focus (info->container))
    adw_tab_grid_try_focus_selected_tab (self, TRUE);

  if (info == self->selected_tab)
    adw_tab_grid_select_page (self, NULL);

  adw_tab_thumbnail_set_page (info->tab, NULL);

  info->page = NULL;

  if (info->appear_animation)
    adw_animation_skip (info->appear_animation);

  gtk_widget_insert_after (GTK_WIDGET (info->container),
                           GTK_WIDGET (self), NULL);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              appear_animation_value_cb,
                                              info, NULL);
  info->appear_animation =
    adw_timed_animation_new (GTK_WIDGET (self), info->appear_progress, 0,
                             CLOSE_ANIMATION_DURATION, target);

  g_signal_connect_swapped (info->appear_animation, "done",
                            G_CALLBACK (close_animation_done_cb), info);

  adw_animation_play (info->appear_animation);
}

/* Tab DND */

#define ADW_TYPE_TAB_GRID_ROOT_CONTENT (adw_tab_grid_root_content_get_type ())

G_DECLARE_FINAL_TYPE (AdwTabGridRootContent, adw_tab_grid_root_content, ADW, TAB_GRID_ROOT_CONTENT, GdkContentProvider)

struct _AdwTabGridRootContent
{
  GdkContentProvider parent_instance;

  AdwTabGrid *tab_grid;
};

G_DEFINE_FINAL_TYPE (AdwTabGridRootContent, adw_tab_grid_root_content, GDK_TYPE_CONTENT_PROVIDER)

static GdkContentFormats *
adw_tab_grid_root_content_ref_formats (GdkContentProvider *provider)
{
  return gdk_content_formats_new ((const char *[1]) { "application/x-rootwindow-drop" }, 1);
}

static void
adw_tab_grid_root_content_write_mime_type_async (GdkContentProvider  *provider,
                                                 const char          *mime_type,
                                                 GOutputStream       *stream,
                                                 int                  io_priority,
                                                 GCancellable        *cancellable,
                                                 GAsyncReadyCallback  callback,
                                                 gpointer             user_data)
{
  AdwTabGridRootContent *self = ADW_TAB_GRID_ROOT_CONTENT (provider);
  GTask *task;

  self->tab_grid->should_detach_into_new_window = TRUE;

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_priority (task, io_priority);
  g_task_set_source_tag (task, adw_tab_grid_root_content_write_mime_type_async);
  g_task_return_boolean (task, TRUE);

  g_object_unref (task);
}

static gboolean
adw_tab_grid_root_content_write_mime_type_finish (GdkContentProvider  *provider,
                                                  GAsyncResult        *result,
                                                  GError             **error)
{
  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
adw_tab_grid_root_content_finalize (GObject *object)
{
  AdwTabGridRootContent *self = ADW_TAB_GRID_ROOT_CONTENT (object);

  g_clear_object (&self->tab_grid);

  G_OBJECT_CLASS (adw_tab_grid_root_content_parent_class)->finalize (object);
}

static void
adw_tab_grid_root_content_class_init (AdwTabGridRootContentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GdkContentProviderClass *provider_class = GDK_CONTENT_PROVIDER_CLASS (klass);

  object_class->finalize = adw_tab_grid_root_content_finalize;

  provider_class->ref_formats = adw_tab_grid_root_content_ref_formats;
  provider_class->write_mime_type_async = adw_tab_grid_root_content_write_mime_type_async;
  provider_class->write_mime_type_finish = adw_tab_grid_root_content_write_mime_type_finish;
}

static void
adw_tab_grid_root_content_init (AdwTabGridRootContent *self)
{
}

static GdkContentProvider *
adw_tab_grid_root_content_new (AdwTabGrid *tab_grid)
{
  AdwTabGridRootContent *self = g_object_new (ADW_TYPE_TAB_GRID_ROOT_CONTENT, NULL);

  self->tab_grid = g_object_ref (tab_grid);

  return GDK_CONTENT_PROVIDER (self);
}

static int
calculate_placeholder_index (AdwTabGrid *self,
                             int         x,
                             int         y)
{
  int lower, upper, i;
  gboolean is_rtl;

  get_visible_range (self, &lower, &upper);

  x = CLAMP (x, 0, self->allocated_width);
  y = CLAMP (y, lower, upper);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  for (i = 0; i < self->n_tabs; i++) {
    int tab_x, tab_y;

    get_position_for_index (self, i, is_rtl, &tab_x, &tab_y);

    if (x <= tab_x + self->tab_height + SPACING / 2 &&
        y <= tab_y + self->tab_width + SPACING / 2)
      return i;
  }

  return i;
}

static void
insert_animation_value_cb (double   value,
                           TabInfo *info)
{
  AdwTabGrid *self = info->box;

  appear_animation_value_cb (value, info);

  update_drag_reodering (self);
}

static void
insert_placeholder (AdwTabGrid *self,
                    AdwTabPage *page,
                    int         x,
                    int         y)
{
  TabInfo *info = self->reorder_placeholder;
  double initial_progress = 0;
  AdwAnimationTarget *target;

  if (info) {
    initial_progress = info->appear_progress;

    if (info->appear_animation)
      adw_animation_skip (info->appear_animation);
  } else {
    int index;

    self->placeholder_page = page;

    info = create_tab_info (self, page);

    info->is_hidden = TRUE;
    gtk_widget_set_opacity (info->container, 0);

    info->reorder_ignore_bounds = TRUE;

    index = calculate_placeholder_index (self, x, y);

    self->tabs = g_list_insert (self->tabs, info, index);
    self->n_tabs++;

    if (!self->searching)
      set_empty (self, FALSE);

    self->reorder_placeholder = info;
    self->reorder_index = g_list_index (self->tabs, info);
  }

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              insert_animation_value_cb,
                                              info, NULL);
  info->appear_animation =
    adw_timed_animation_new (GTK_WIDGET (self), initial_progress, 1,
                             OPEN_ANIMATION_DURATION, target);

  g_signal_connect_swapped (info->appear_animation, "done",
                            G_CALLBACK (open_animation_done_cb), info);

  adw_animation_play (info->appear_animation);
}

static void
replace_animation_done_cb (TabInfo *info)
{
  AdwTabGrid *self = info->box;

  g_clear_object (&info->appear_animation);
  self->reorder_placeholder = NULL;
  self->can_remove_placeholder = TRUE;
}

static void
replace_placeholder (AdwTabGrid *self,
                     AdwTabPage *page)
{
  TabInfo *info = self->reorder_placeholder;
  double initial_progress;
  AdwAnimationTarget *target;

  self->reorder_placeholder->is_hidden = FALSE;
  gtk_widget_set_opacity (self->reorder_placeholder->container, 1);

  if (!info->appear_animation) {
    self->reorder_placeholder = NULL;

    return;
  }

  initial_progress = info->appear_progress;

  self->can_remove_placeholder = FALSE;

  adw_tab_thumbnail_set_page (info->tab, page);
  info->page = page;

  adw_animation_skip (info->appear_animation);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              appear_animation_value_cb,
                                              info, NULL);
  info->appear_animation =
    adw_timed_animation_new (GTK_WIDGET (self), initial_progress, 1,
                             OPEN_ANIMATION_DURATION, target);

  g_signal_connect_swapped (info->appear_animation, "done",
                            G_CALLBACK (replace_animation_done_cb), info);

  adw_animation_play (info->appear_animation);
}

static void
remove_animation_done_cb (TabInfo *info)
{
  AdwTabGrid *self = info->box;

  g_clear_object (&info->appear_animation);

  if (!self->can_remove_placeholder) {
    adw_tab_thumbnail_set_page (info->tab, self->placeholder_page);
    info->page = self->placeholder_page;

    return;
  }

  if (self->reordered_tab == info) {
    force_end_reordering (self);

    if (info->reorder_animation)
      adw_animation_skip (info->reorder_animation);

    self->reordered_tab = NULL;
  }

  if (self->pressed_tab == info)
    self->pressed_tab = NULL;

  self->tabs = g_list_remove (self->tabs, info);

  remove_and_free_tab_info (info);

  self->n_tabs--;

  self->reorder_placeholder = NULL;

  if (self->n_tabs == 0 || (self->searching && get_n_visible_tabs (self) == 0))
    set_empty (self, TRUE);
}

static void
remove_placeholder (AdwTabGrid *self)
{
  TabInfo *info = self->reorder_placeholder;
  AdwAnimationTarget *target;

  if (!info || !info->page)
    return;

  adw_tab_thumbnail_set_page (info->tab, NULL);
  info->page = NULL;

  if (info->appear_animation)
    adw_animation_skip (info->appear_animation);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              appear_animation_value_cb,
                                              info, NULL);
  info->appear_animation =
    adw_timed_animation_new (GTK_WIDGET (self), info->appear_progress, 0,
                             CLOSE_ANIMATION_DURATION, target);

  g_signal_connect_swapped (info->appear_animation, "done",
                            G_CALLBACK (remove_animation_done_cb), info);

  adw_animation_play (info->appear_animation);
}

static inline AdwTabGrid *
get_source_tab_grid (GtkDropTarget *target)
{
  GdkDrop *drop = gtk_drop_target_get_current_drop (target);
  GdkDrag *drag = gdk_drop_get_drag (drop);

  if (!drag)
    return NULL;

  return ADW_TAB_GRID (g_object_get_data (G_OBJECT (drag),
                      "adw-tab-overview-drag-origin"));
}

static void
do_drag_drop (AdwTabGrid *self,
              AdwTabGrid *source_tab_grid)
{
  AdwTabPage *page = source_tab_grid->detached_page;
  int offset = (self->pinned ? 0 : adw_tab_view_get_n_pinned_pages (self->view));

  if (self->reorder_placeholder) {
    replace_placeholder (self, page);
    end_drag_reodering (self);

    g_signal_handlers_block_by_func (self->view, page_attached_cb, self);

    adw_tab_view_attach_page (self->view, page, self->reorder_index + offset);

    g_signal_handlers_unblock_by_func (self->view, page_attached_cb, self);
  } else {
    adw_tab_view_attach_page (self->view, page, self->reorder_index + offset);
  }

  source_tab_grid->should_detach_into_new_window = FALSE;
  source_tab_grid->detached_page = NULL;

  self->indirect_reordering = FALSE;
}

static void
detach_into_new_window (AdwTabGrid *self)
{
  AdwTabPage *page;
  AdwTabView *new_view;

  page = self->detached_page;

  new_view = adw_tab_view_create_window (self->view);

  if (ADW_IS_TAB_VIEW (new_view))
    adw_tab_view_attach_page (new_view, page, 0);
  else
    adw_tab_view_attach_page (self->view, page, self->detached_index);

  self->should_detach_into_new_window = FALSE;
}

static gboolean
is_view_in_the_same_group (AdwTabGrid  *self,
                           AdwTabView *other_view)
{
  /* TODO when we have groups, this should do the actual check */
  return TRUE;
}

static void
drag_end (AdwTabGrid *self,
          GdkDrag    *drag,
          gboolean    success)
{
  g_signal_handlers_disconnect_by_data (drag, self);

  gdk_drag_drop_done (drag, success);

  if (!success) {
    adw_tab_view_attach_page (self->view,
                              self->detached_page,
                              self->detached_index);

    self->indirect_reordering = FALSE;
  }

  self->detached_page = NULL;

  if (self->drag_icon) {
    g_clear_object (&self->drag_icon->resize_animation);
    g_clear_pointer (&self->drag_icon, g_atomic_rc_box_release);
  }

  g_object_unref (drag);
}

static void
tab_drop_performed_cb (AdwTabGrid *self,
                       GdkDrag    *drag)
{
  /* Catch drops into our windows, but outside of tab views. If this is a false
   * positive, it will be set to FALSE in do_drag_drop(). */
  self->should_detach_into_new_window = TRUE;
}

static void
tab_dnd_finished_cb (AdwTabGrid *self,
                     GdkDrag   *drag)
{
  if (self->should_detach_into_new_window)
    detach_into_new_window (self);

  drag_end (self, drag, TRUE);
}

static void
tab_drag_cancel_cb (AdwTabGrid           *self,
                    GdkDragCancelReason  reason,
                    GdkDrag             *drag)
{
  if (reason == GDK_DRAG_CANCEL_NO_TARGET) {
    detach_into_new_window (self);
    drag_end (self, drag, TRUE);

    return;
  }

  self->should_detach_into_new_window = FALSE;
  drag_end (self, drag, FALSE);
}

static void
icon_resize_animation_value_cb (double    value,
                                DragIcon *icon)
{
  double relative_x, relative_y;

  relative_x = (double) icon->hotspot_x / icon->width;
  relative_y = (double) icon->hotspot_y / icon->height;

  icon->width = (int) round (adw_lerp (icon->initial_width, icon->target_width, value));
  icon->height = (int) round (adw_lerp (icon->initial_height, icon->target_height, value));

  gtk_widget_set_size_request (GTK_WIDGET (icon->tab), icon->width, icon->height);

  icon->hotspot_x = (int) round (icon->width * relative_x);
  icon->hotspot_y = (int) round (icon->height * relative_y);

  gdk_drag_set_hotspot (icon->drag, icon->hotspot_x, icon->hotspot_y);

  gtk_widget_queue_resize (GTK_WIDGET (icon->tab));
}

static void
create_drag_icon (AdwTabGrid *self,
                  GdkDrag    *drag)
{
  DragIcon *icon;
  AdwAnimationTarget *target;

  icon = g_atomic_rc_box_new0 (DragIcon);

  icon->drag = drag;

  icon->width = self->tab_width;
  icon->initial_width = icon->width;
  icon->target_width = icon->width;

  icon->height = self->tab_height;
  icon->initial_width = icon->height;
  icon->target_width = icon->height;

  icon->tab = adw_tab_thumbnail_new (self->view, FALSE);
  adw_tab_thumbnail_set_page (icon->tab, self->reordered_tab->page);
  adw_tab_thumbnail_set_inverted (icon->tab, self->inverted);
  gtk_widget_set_halign (GTK_WIDGET (icon->tab), GTK_ALIGN_START);

  gtk_drag_icon_set_child (GTK_DRAG_ICON (gtk_drag_icon_get_for_drag (drag)),
                           GTK_WIDGET (icon->tab));

  gtk_widget_set_size_request (GTK_WIDGET (icon->tab), icon->width, icon->height);

  icon->hotspot_x = (int) self->drag_offset_x;
  icon->hotspot_y = (int) self->drag_offset_y;

  gdk_drag_set_hotspot (drag, icon->hotspot_x, icon->hotspot_y);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              icon_resize_animation_value_cb,
                                              g_atomic_rc_box_acquire (icon), NULL);
  icon->resize_animation =
    adw_timed_animation_new (GTK_WIDGET (icon->tab), 0, 1,
                             ICON_RESIZE_ANIMATION_DURATION, target);

  self->drag_icon = icon;
}

static void
resize_drag_icon (AdwTabGrid *self,
                  int         width,
                  int         height)
{
  DragIcon *icon = self->drag_icon;

  if (width == icon->target_width && height == icon->target_height)
    return;

  icon->initial_width = icon->width;
  icon->initial_height = icon->height;

  icon->target_width = width;
  icon->target_height = height;

  adw_animation_play (icon->resize_animation);
}

static void
begin_drag (AdwTabGrid *self,
            GdkDevice  *device)
{
  GdkContentProvider *content;
  GtkNative *native;
  GdkSurface *surface;
  GdkDrag *drag;
  TabInfo *detached_info;
  GtkWidget *detached_tab;

  native = gtk_widget_get_native (GTK_WIDGET (self));
  surface = gtk_native_get_surface (native);

  self->hovering = TRUE;
  get_other_tab_grid (self)->hovering = TRUE;
  self->pressed_tab = NULL;

  detached_info = self->reordered_tab;
  detached_tab = g_object_ref (detached_info->container);
  self->detached_page = detached_info->page;

  self->indirect_reordering = TRUE;

  content = gdk_content_provider_new_union ((GdkContentProvider *[2]) {
                                              adw_tab_grid_root_content_new (self),
                                              gdk_content_provider_new_typed (ADW_TYPE_TAB_PAGE, detached_info->page)
                                            }, 2);

  drag = gdk_drag_begin (surface, device, content, GDK_ACTION_MOVE,
                         self->reorder_x, self->reorder_y);

  g_object_set_data (G_OBJECT (drag), "adw-tab-overview-drag-origin", self);

  g_signal_connect_swapped (drag, "drop-performed",
                            G_CALLBACK (tab_drop_performed_cb), self);
  g_signal_connect_swapped (drag, "dnd-finished",
                            G_CALLBACK (tab_dnd_finished_cb), self);
  g_signal_connect_swapped (drag, "cancel",
                            G_CALLBACK (tab_drag_cancel_cb), self);

  create_drag_icon (self, drag);

  end_drag_reodering (self);
  update_hover (self);

  detached_info->is_hidden = TRUE;
  gtk_widget_set_opacity (detached_tab, 0);
  self->detached_index = adw_tab_view_get_page_position (self->view, self->detached_page);

  adw_tab_view_detach_page (self->view, self->detached_page);

  self->indirect_reordering = FALSE;

  g_object_unref (content);
  g_object_unref (detached_tab);
}

static GdkDragAction
tab_drag_enter_motion_cb (AdwTabGrid    *self,
                          double         x,
                          double         y,
                          GtkDropTarget *target)
{
  AdwTabGrid *source_tab_grid;

  if (self->pinned)
    return 0;

  if (self->searching)
    return 0;

  source_tab_grid = get_source_tab_grid (target);

  if (!source_tab_grid)
    return 0;

  if (!self->view || !is_view_in_the_same_group (self, source_tab_grid->view))
    return 0;

  self->can_remove_placeholder = FALSE;

  if (!self->reorder_placeholder || !self->reorder_placeholder->page) {
    AdwTabPage *page = source_tab_grid->detached_page;
    double center_x = x - source_tab_grid->drag_icon->hotspot_x + source_tab_grid->drag_icon->width / 2;
    double center_y = y - source_tab_grid->drag_icon->hotspot_y + source_tab_grid->drag_icon->height / 2;

    insert_placeholder (self, page, center_x, center_y);

    self->indirect_reordering = TRUE;

    resize_drag_icon (source_tab_grid, self->tab_width, self->tab_height);
    adw_tab_thumbnail_set_inverted (source_tab_grid->drag_icon->tab, self->inverted);

    self->drag_offset_x = source_tab_grid->drag_icon->hotspot_x;
    self->drag_offset_y = source_tab_grid->drag_icon->hotspot_y;

    self->reorder_x = (int) round (x - source_tab_grid->drag_icon->hotspot_x);
    self->reorder_y = (int) round (y - source_tab_grid->drag_icon->hotspot_y);

    start_drag_reodering (self, self->reorder_placeholder, x, y);

    return GDK_ACTION_MOVE;
  }

  self->reorder_x = (int) round (x - source_tab_grid->drag_icon->hotspot_x);
  self->reorder_y = (int) round (y - source_tab_grid->drag_icon->hotspot_y);

  update_drag_reodering (self);

  return GDK_ACTION_MOVE;
}

static void
tab_drag_leave_cb (AdwTabGrid    *self,
                   GtkDropTarget *target)
{
  AdwTabGrid *source_tab_grid;

  if (!self->indirect_reordering)
    return;

  if (self->pinned)
    return;

  source_tab_grid = get_source_tab_grid (target);

  if (!source_tab_grid)
    return;

  if (!self->view || !is_view_in_the_same_group (self, source_tab_grid->view))
    return;

  self->can_remove_placeholder = TRUE;

  end_drag_reodering (self);
  remove_placeholder (self);

  self->indirect_reordering = FALSE;
}

static gboolean
tab_drag_drop_cb (AdwTabGrid    *self,
                  const GValue  *value,
                  double         x,
                  double         y,
                  GtkDropTarget *target)
{
  AdwTabGrid *source_tab_grid;

  if (self->pinned)
    return GDK_EVENT_PROPAGATE;

  source_tab_grid = get_source_tab_grid (target);

  if (!source_tab_grid)
    return GDK_EVENT_PROPAGATE;

  if (!self->view || !is_view_in_the_same_group (self, source_tab_grid->view))
    return GDK_EVENT_PROPAGATE;

  do_drag_drop (self, source_tab_grid);

  return GDK_EVENT_STOP;
}

static gboolean
view_drag_drop_cb (AdwTabGrid    *self,
                   const GValue  *value,
                   double         x,
                   double         y,
                   GtkDropTarget *target)
{
  AdwTabGrid *source_tab_grid;

  if (self->pinned)
    return GDK_EVENT_PROPAGATE;

  source_tab_grid = get_source_tab_grid (target);

  if (!source_tab_grid)
    return GDK_EVENT_PROPAGATE;

  if (!self->view || !is_view_in_the_same_group (self, source_tab_grid->view))
    return GDK_EVENT_PROPAGATE;

  self->reorder_index = adw_tab_view_get_n_pages (self->view) -
                        adw_tab_view_get_n_pinned_pages (self->view);

  do_drag_drop (self, source_tab_grid);

  return GDK_EVENT_STOP;
}

/* DND autoscrolling */

static void
reset_drop_target_tab_cb (AdwTabGrid *self)
{
  self->reset_drop_target_tab_id = 0;
  set_drop_target_tab (self, NULL);
}

static void
drag_leave_cb (AdwTabGrid              *self,
               GtkDropControllerMotion *controller)
{
  GdkDrop *drop = gtk_drop_controller_motion_get_drop (controller);
  GdkDrag *drag = gdk_drop_get_drag (drop);
  AdwTabGrid *source;

  source = drag ? g_object_get_data (G_OBJECT (drag), "adw-tab-overview-drag-origin") : NULL;

  if (source)
    return;

  if (!self->reset_drop_target_tab_id)
    self->reset_drop_target_tab_id =
      g_idle_add_once ((GSourceOnceFunc) reset_drop_target_tab_cb, self);

  end_autoscroll (self);
}

static void
drag_enter_motion_cb (AdwTabGrid              *self,
                      double                   x,
                      double                   y,
                      GtkDropControllerMotion *controller)
{
  TabInfo *info;
  GdkDrop *drop = gtk_drop_controller_motion_get_drop (controller);
  GdkDrag *drag = gdk_drop_get_drag (drop);
  AdwTabGrid *source;

  source = drag ? g_object_get_data (G_OBJECT (drag), "adw-tab-overview-drag-origin") : NULL;

  if (source)
    return;

  info = find_tab_info_at (self, x, y);

  if (!info) {
    drag_leave_cb (self, controller);

    return;
  }

  self->drop_target_x = x;
  self->drop_target_y = y;
  set_drop_target_tab (self, info);

  start_autoscroll (self);
}

/* Context menu */

static void
reset_setup_menu_cb (AdwTabGrid *self)
{
  g_signal_emit_by_name (self->view, "setup-menu", NULL);
}

static void
touch_menu_notify_visible_cb (AdwTabGrid *self)
{
  if (!self->context_menu || gtk_widget_get_visible (self->context_menu))
    return;

  self->hovering = FALSE;
  get_other_tab_grid (self)->hovering = FALSE;
  update_hover (self);

  g_idle_add_once ((GSourceOnceFunc) reset_setup_menu_cb, self);
}

static void
do_popup (AdwTabGrid *self,
          TabInfo    *info,
          double      x,
          double      y)
{
  GMenuModel *model = adw_tab_view_get_menu_model (self->view);
  GdkRectangle rect;

  if (!G_IS_MENU_MODEL (model))
    return;

  g_signal_emit_by_name (self->view, "setup-menu", info->page);

  if (!self->context_menu) {
    self->context_menu = gtk_popover_menu_new_from_model (model);
    gtk_widget_set_parent (self->context_menu, GTK_WIDGET (self));
    gtk_popover_set_position (GTK_POPOVER (self->context_menu), GTK_POS_BOTTOM);
    gtk_popover_set_has_arrow (GTK_POPOVER (self->context_menu), FALSE);

    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      gtk_widget_set_halign (self->context_menu, GTK_ALIGN_END);
    else
      gtk_widget_set_halign (self->context_menu, GTK_ALIGN_START);

    g_signal_connect_object (self->context_menu, "notify::visible",
                             G_CALLBACK (touch_menu_notify_visible_cb), self,
                             G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  }

  if ((G_APPROX_VALUE (x, 0, DBL_EPSILON) || x > 0) &&
      (G_APPROX_VALUE (y, 0, DBL_EPSILON) || y > 0)) {
    rect.x = x;
    rect.y = y;
  } else {
    rect.x = info->pos_x;
    rect.y = info->pos_y + gtk_widget_get_height (info->container);

    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      rect.x += info->width;
  }

  rect.width = 0;
  rect.height = 0;

  gtk_popover_set_pointing_to (GTK_POPOVER (self->context_menu), &rect);

  gtk_popover_popup (GTK_POPOVER (self->context_menu));
}

static void
long_pressed_cb (AdwTabGrid *self,
                 double      x,
                 double      y,
                 GtkGesture *gesture)
{
  TabInfo *info = find_tab_info_at (self, x, y);

  gtk_gesture_set_state (self->drag_gesture, GTK_EVENT_SEQUENCE_DENIED);

  if (!info || !info->page) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
  do_popup (self, info, x, y);
}

static void
popup_menu_cb (GtkWidget  *widget,
               const char *action_name,
               GVariant   *parameter)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);
  TabInfo *info = get_focused_info (self);

  if (!info || !info->page)
    return;

  do_popup (self, info, -1, -1);
}

/* Clicking */

static void
pressed_cb (AdwTabGrid *self,
            int         n_press,
            double      x,
            double      y,
            GtkGesture *gesture)
{
  TabInfo *info;
  GdkEvent *event;
  GdkEventSequence *current;
  guint button;

  if (is_touchscreen (gesture))
    return;

  info = find_tab_info_at (self, x, y);

  if (!info || !info->page) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  current = gtk_gesture_single_get_current_sequence (GTK_GESTURE_SINGLE (gesture));
  event = gtk_gesture_get_last_event (gesture, current);

   if (gdk_event_triggers_context_menu (event)) {
    do_popup (self, info, x, y);
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (gesture));

    return;
  }

  button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));

  if (button == GDK_BUTTON_MIDDLE) {
    self->middle_clicked_tab = info;
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);

    return;
  }

  if (button != GDK_BUTTON_PRIMARY) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }
}

static void
released_cb (AdwTabGrid *self,
             int         n_press,
             double      x,
             double      y,
             GtkGesture *gesture)
{
  TabInfo *info;
  guint button;

  if (x < 0 || x > gtk_widget_get_width (GTK_WIDGET (self))) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  info = find_tab_info_at (self, x, y);

  if (!info || !info->page) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));

  if (button == GDK_BUTTON_MIDDLE) {
    if (info != self->middle_clicked_tab) {
      self->middle_clicked_tab = NULL;
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
      return;
    }

    adw_tab_view_close_page (self->view, info->page);
    self->middle_clicked_tab = NULL;

    return;
  }

  adw_tab_view_set_selected_page (self->view, info->page);
  adw_tab_overview_set_open (self->tab_overview, FALSE);
}

/* Overrides */

static void
adw_tab_grid_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      int             for_size,
                      int            *minimum,
                      int            *natural,
                      int            *minimum_baseline,
                      int            *natural_baseline)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);

  measure_tab_grid (self, orientation, for_size, minimum, natural, TRUE);

  if (minimum_baseline)
    *minimum_baseline = -1;

  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adw_tab_grid_size_allocate (GtkWidget *widget,
                            int        width,
                            int        height,
                            int        baseline)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);
  GList *l;

  measure_tab_grid (self, GTK_ORIENTATION_HORIZONTAL, -1,
                    &self->allocated_width, NULL, TRUE);
  self->allocated_width = MAX (self->allocated_width, width);

  measure_tab_grid (self, GTK_ORIENTATION_VERTICAL, width,
                    &self->allocated_height, NULL, TRUE);
  self->allocated_height = MAX (self->allocated_height, height);

  calculate_tab_layout (self);

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    GskTransform *transform = NULL;
    int x, y, w, h;

    if (!gtk_widget_should_layout (info->container))
      continue;

    x = ((info == self->reordered_tab) ? self->reorder_window_x : info->pos_x);
    y = ((info == self->reordered_tab) ? self->reorder_window_y : info->pos_y);
    w = MAX (0, info->width);
    h = MAX (0, info->height);

    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (x, y));

    if (info->appear_progress < 1) {
      double scale = MIN_SCALE + (1 - MIN_SCALE) * info->appear_progress;
      transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (w / 2.0f, h / 2.0f));
      transform = gsk_transform_scale (transform, scale, scale);
      transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (-w / 2.0f, -h / 2.0f));
    }

    gtk_widget_allocate (info->container, w, h, baseline, transform);
  }
}

static GtkSizeRequestMode
adw_tab_grid_get_request_mode (GtkWidget *widget)
{
  return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static inline gboolean
page_can_be_focused (GList *l)
{
  return ((TabInfo *) l->data)->page && ((TabInfo *) l->data)->visible;
}

static gboolean
adw_tab_grid_focus (GtkWidget        *widget,
                    GtkDirectionType  direction)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);
  gboolean is_rtl;
  GtkDirectionType start, end;
  GList *l;
  TabInfo *info = NULL;
  int n_columns = (int) ceil (self->n_columns);

  is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
  start = is_rtl ? GTK_DIR_RIGHT : GTK_DIR_LEFT;
  end = is_rtl ? GTK_DIR_LEFT : GTK_DIR_RIGHT;

  l = find_link_for_widget (self, gtk_widget_get_focus_child (widget));

  if (!self->n_tabs)
    return GDK_EVENT_PROPAGATE;

  if (((direction == GTK_DIR_TAB_FORWARD ||
        direction == GTK_DIR_TAB_BACKWARD) &&
       l && l->data != self->selected_tab) || !l) {
    info = self->selected_tab;
  } else if (direction == start) {
    do {
      l = l->prev;
    } while (l && l->data && !page_can_be_focused (l));

    info = l ? l->data : NULL;
  } else if (direction == end) {
    do {
      l = l->next;
    } while (l && l->data && !page_can_be_focused (l));

    info = l ? l->data : NULL;
  } else if (direction == GTK_DIR_UP) {
    do {
      l = l->prev;

      if (l && l->data && page_can_be_focused (l))
        n_columns--;
    } while (l && l->data && n_columns > 0);

    info = l ? l->data : NULL;
  } else if (direction == GTK_DIR_DOWN) {
    GList *last_link = find_nth_visible_tab (self, get_n_visible_tabs (self) - 1);
    TabInfo *last_info = last_link->data;
    int last_col = (int) round (fmod (last_info->final_index, n_columns));
    int empty_slots = n_columns - last_col;

    do {
      l = l->next;

      if (l && l->data && page_can_be_focused (l))
        n_columns--;
    } while (l && l->data && n_columns > 0);

    if (n_columns > 0 && n_columns < empty_slots)
      l = last_link;

    info = l ? l->data : NULL;
  }

  if (!info) {
    AdwTabGrid *grid = get_other_tab_grid (self);

    if (self->pinned && direction == GTK_DIR_DOWN) {
      int column = get_focused_column (self);

      return adw_tab_grid_focus_first_row (grid, column);
    }

    if (self->pinned && direction == end)
      return adw_tab_grid_focus_first_row (grid, 0) ||
             gtk_widget_keynav_failed (widget, direction);

    if (!self->pinned && direction == GTK_DIR_UP) {
      int column = get_focused_column (self);

      return adw_tab_grid_focus_last_row (grid, column);
    }

    if (!self->pinned && direction == start)
      return adw_tab_grid_focus_last_row (grid, -1) ||
             gtk_widget_keynav_failed (widget, direction);

    if (direction != GTK_DIR_UP && direction != GTK_DIR_DOWN)
      return gtk_widget_keynav_failed (widget, direction);

    return GDK_EVENT_PROPAGATE;
  }

  scroll_to_tab (self, info, FOCUS_ANIMATION_DURATION);

  return gtk_widget_grab_focus (info->container);
}

static gboolean
adw_tab_grid_grab_focus (GtkWidget *widget)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);

  if (!self->selected_tab)
    return GDK_EVENT_PROPAGATE;

  scroll_to_tab (self, self->selected_tab, FOCUS_ANIMATION_DURATION);

  return gtk_widget_grab_focus (self->selected_tab->container);
}

static void
adw_tab_grid_unrealize (GtkWidget *widget)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);

  g_clear_pointer (&self->context_menu, gtk_widget_unparent);

  GTK_WIDGET_CLASS (adw_tab_grid_parent_class)->unrealize (widget);
}

static void
adw_tab_grid_unmap (GtkWidget *widget)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);

  force_end_reordering (self);

  if (self->drag_autoscroll_cb_id) {
    gtk_widget_remove_tick_callback (widget, self->drag_autoscroll_cb_id);
    self->drag_autoscroll_cb_id = 0;
  }

  GTK_WIDGET_CLASS (adw_tab_grid_parent_class)->unmap (widget);
}

static void
adw_tab_grid_direction_changed (GtkWidget        *widget,
                                GtkTextDirection  previous_direction)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);

  if (gtk_widget_get_direction (widget) == previous_direction)
    return;

  if (self->context_menu) {
    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      gtk_widget_set_halign (self->context_menu, GTK_ALIGN_END);
    else
      gtk_widget_set_halign (self->context_menu, GTK_ALIGN_START);
  }
}

static void
adw_tab_grid_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
  AdwTabGrid *self = ADW_TAB_GRID (widget);
  GList *l;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    int pos, height;

    if (info == self->reordered_tab)
      continue;

    pos = get_tab_y (self, info, FALSE);
    height = gtk_widget_get_height (info->container);

    if (pos + height < self->visible_lower - self->lower_inset)
      continue;

    if (pos > self->visible_upper + self->upper_inset)
      continue;

    gtk_widget_snapshot_child (widget, info->container, snapshot);
  }

  if (self->reordered_tab)
    gtk_widget_snapshot_child (widget, self->reordered_tab->container, snapshot);
}

static void
adw_tab_grid_dispose (GObject *object)
{
  AdwTabGrid *self = ADW_TAB_GRID (object);

  g_clear_handle_id (&self->drop_switch_timeout_id, g_source_remove);

  self->drag_gesture = NULL;
  self->tab_overview = NULL;
  adw_tab_grid_set_view (self, NULL);

  g_clear_object (&self->filter);
  self->title_filter = NULL;
  self->tooltip_filter = NULL;
  self->keyword_filter = NULL;

  g_clear_object (&self->resize_animation);

  g_clear_pointer (&self->context_menu, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_tab_grid_parent_class)->dispose (object);
}

static void
adw_tab_grid_finalize (GObject *object)
{
  AdwTabGrid *self = (AdwTabGrid *) object;

  g_clear_pointer (&self->extra_drag_types, g_free);

  G_OBJECT_CLASS (adw_tab_grid_parent_class)->finalize (object);
}

static void
adw_tab_grid_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwTabGrid *self = ADW_TAB_GRID (object);

  switch (prop_id) {
  case PROP_PINNED:
    g_value_set_boolean (value, self->pinned);
    break;

  case PROP_TAB_OVERVIEW:
    g_value_set_object (value, self->tab_overview);
    break;

  case PROP_VIEW:
    g_value_set_object (value, self->view);
    break;

  case PROP_RESIZE_FROZEN:
    g_value_set_boolean (value, self->tab_resize_mode != TAB_RESIZE_NORMAL);
    break;

  case PROP_EMPTY:
    g_value_set_boolean (value, adw_tab_grid_get_empty (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_grid_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwTabGrid *self = ADW_TAB_GRID (object);

  switch (prop_id) {
  case PROP_PINNED:
    self->pinned = g_value_get_boolean (value);
    break;

  case PROP_TAB_OVERVIEW:
    self->tab_overview = g_value_get_object (value);
    break;

  case PROP_VIEW:
    adw_tab_grid_set_view (self, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_grid_class_init (AdwTabGridClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_tab_grid_dispose;
  object_class->finalize = adw_tab_grid_finalize;
  object_class->get_property = adw_tab_grid_get_property;
  object_class->set_property = adw_tab_grid_set_property;

  widget_class->measure = adw_tab_grid_measure;
  widget_class->size_allocate = adw_tab_grid_size_allocate;
  widget_class->get_request_mode = adw_tab_grid_get_request_mode;
  widget_class->focus = adw_tab_grid_focus;
  widget_class->grab_focus = adw_tab_grid_grab_focus;
  widget_class->unrealize = adw_tab_grid_unrealize;
  widget_class->unmap = adw_tab_grid_unmap;
  widget_class->direction_changed = adw_tab_grid_direction_changed;
  widget_class->snapshot = adw_tab_grid_snapshot;

  props[PROP_PINNED] =
    g_param_spec_boolean ("pinned", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  props[PROP_TAB_OVERVIEW] =
    g_param_spec_object ("tab-overview", NULL, NULL,
                         ADW_TYPE_TAB_OVERVIEW,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  props[PROP_VIEW] =
    g_param_spec_object ("view", NULL, NULL,
                         ADW_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_RESIZE_FROZEN] =
    g_param_spec_boolean ("resize-frozen", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_EMPTY] =
    g_param_spec_boolean ("empty", NULL, NULL,
                          TRUE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  signals[SIGNAL_SCROLL_RELATIVE] =
    g_signal_new ("scroll-relative",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__DOUBLE_UINT,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_DOUBLE,
                  G_TYPE_UINT);
  g_signal_set_va_marshaller (signals[SIGNAL_SCROLL_RELATIVE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__DOUBLE_UINTv);

  signals[SIGNAL_SCROLL_TO_TAB] =
    g_signal_new ("scroll-to-tab",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__DOUBLE_UINT,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_DOUBLE,
                  G_TYPE_UINT);
  g_signal_set_va_marshaller (signals[SIGNAL_SCROLL_TO_TAB],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__DOUBLE_UINTv);

  signals[SIGNAL_EXTRA_DRAG_DROP] =
    g_signal_new ("extra-drag-drop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  G_TYPE_BOOLEAN,
                  3,
                  ADW_TYPE_TAB_PAGE,
                  G_TYPE_VALUE,
                  GDK_TYPE_DRAG_ACTION);

  signals[SIGNAL_EXTRA_DRAG_VALUE] =
    g_signal_new ("extra-drag-value",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins,
                  NULL, NULL,
                  GDK_TYPE_DRAG_ACTION,
                  2,
                  ADW_TYPE_TAB_PAGE,
                  G_TYPE_VALUE);

  gtk_widget_class_install_action (widget_class, "menu.popup", NULL, popup_menu_cb);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_F10, GDK_SHIFT_MASK, "menu.popup", NULL);
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_Menu, 0, "menu.popup", NULL);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_Return, 0,
                                (GtkShortcutFunc) activate_tab, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_ISO_Enter, 0,
                                (GtkShortcutFunc) activate_tab, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_KP_Enter, 0,
                                (GtkShortcutFunc) activate_tab, NULL);

  add_reorder_bindings (widget_class, GDK_KEY_Left,  GTK_DIR_LEFT);
  add_reorder_bindings (widget_class, GDK_KEY_Right, GTK_DIR_RIGHT);
  add_reorder_bindings (widget_class, GDK_KEY_Up,    GTK_DIR_UP);
  add_reorder_bindings (widget_class, GDK_KEY_Down,  GTK_DIR_DOWN);

  gtk_widget_class_set_css_name (widget_class, "tabgrid");
}

static void
adw_tab_grid_init (AdwTabGrid *self)
{
  GtkEventController *controller;
  AdwAnimationTarget *target;
  GtkExpression *expression;

  self->can_remove_placeholder = TRUE;
  self->initial_max_n_columns = -1;
  self->visible_lower = 0;
  self->visible_upper = 0;
  self->empty = TRUE;

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (controller), 0);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (controller), TRUE);
  g_signal_connect_swapped (controller, "pressed", G_CALLBACK (pressed_cb), self);
  g_signal_connect_swapped (controller, "released", G_CALLBACK (released_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_long_press_new ());
  gtk_gesture_long_press_set_delay_factor (GTK_GESTURE_LONG_PRESS (controller), 2);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (controller), TRUE);
  gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (controller), TRUE);
  g_signal_connect_swapped (controller, "pressed", G_CALLBACK (long_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_drag_new ());
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (controller), GDK_BUTTON_PRIMARY);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (controller), TRUE);
  g_signal_connect_swapped (controller, "drag-begin", G_CALLBACK (reorder_begin_cb), self);
  g_signal_connect_swapped (controller, "drag-update", G_CALLBACK (reorder_update_cb), self);
  g_signal_connect_swapped (controller, "drag-end", G_CALLBACK (reorder_end_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);
  self->drag_gesture = GTK_GESTURE (controller);

  controller = gtk_drop_controller_motion_new ();
  g_signal_connect_swapped (controller, "enter", G_CALLBACK (drag_enter_motion_cb), self);
  g_signal_connect_swapped (controller, "motion", G_CALLBACK (drag_enter_motion_cb), self);
  g_signal_connect_swapped (controller, "leave", G_CALLBACK (drag_leave_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = GTK_EVENT_CONTROLLER (gtk_drop_target_new (ADW_TYPE_TAB_PAGE, GDK_ACTION_MOVE));
  gtk_drop_target_set_preload (GTK_DROP_TARGET (controller), TRUE);
  g_signal_connect_swapped (controller, "enter", G_CALLBACK (tab_drag_enter_motion_cb), self);
  g_signal_connect_swapped (controller, "motion", G_CALLBACK (tab_drag_enter_motion_cb), self);
  g_signal_connect_swapped (controller, "leave", G_CALLBACK (tab_drag_leave_cb), self);
  g_signal_connect_swapped (controller, "drop", G_CALLBACK (tab_drag_drop_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              resize_animation_value_cb,
                                              self, NULL);
  self->resize_animation =
    adw_timed_animation_new (GTK_WIDGET (self), 0, 1,
                             RESIZE_ANIMATION_DURATION, target);

  g_signal_connect_swapped (self->resize_animation, "done",
                            G_CALLBACK (resize_animation_done_cb), self);

  expression = gtk_property_expression_new (ADW_TYPE_TAB_PAGE, NULL, "title");
  self->title_filter = gtk_string_filter_new (expression);

  expression = gtk_property_expression_new (ADW_TYPE_TAB_PAGE, NULL, "tooltip");
  self->tooltip_filter = gtk_string_filter_new (expression);

  expression = gtk_property_expression_new (ADW_TYPE_TAB_PAGE, NULL, "keyword");
  self->keyword_filter = gtk_string_filter_new (expression);

  self->filter = GTK_FILTER (gtk_any_filter_new ());
  gtk_multi_filter_append (GTK_MULTI_FILTER (self->filter),
                           GTK_FILTER (self->title_filter));
  gtk_multi_filter_append (GTK_MULTI_FILTER (self->filter),
                           GTK_FILTER (self->tooltip_filter));
  gtk_multi_filter_append (GTK_MULTI_FILTER (self->filter),
                           GTK_FILTER (self->keyword_filter));

  g_signal_connect_swapped (self->filter, "changed",
                            G_CALLBACK (search_changed_cb), self);
}

void
adw_tab_grid_set_view (AdwTabGrid *self,
                       AdwTabView *view)
{
  g_return_if_fail (ADW_IS_TAB_GRID (self));
  g_return_if_fail (view == NULL || ADW_IS_TAB_VIEW (view));

  if (view == self->view)
    return;

  if (self->view) {
    force_end_reordering (self);
    g_signal_handlers_disconnect_by_func (self->view, page_attached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_detached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_reordered_cb, self);

    if (!self->pinned) {
      gtk_widget_remove_controller (GTK_WIDGET (self->view), self->view_drop_target);
      self->view_drop_target = NULL;
    }

    g_clear_list (&self->tabs, (GDestroyNotify) remove_and_free_tab_info);
    self->n_tabs = 0;
  }

  self->view = view;

  if (self->view) {
    int i, n_pages = adw_tab_view_get_n_pages (self->view);

    for (i = n_pages - 1; i >= 0; i--)
      page_attached_cb (self, adw_tab_view_get_nth_page (self->view, i), 0);

    g_signal_connect_object (self->view, "page-attached", G_CALLBACK (page_attached_cb), self, G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-detached", G_CALLBACK (page_detached_cb), self, G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-reordered", G_CALLBACK (page_reordered_cb), self, G_CONNECT_SWAPPED);

    if (!self->pinned) {
      self->view_drop_target = GTK_EVENT_CONTROLLER (gtk_drop_target_new (ADW_TYPE_TAB_PAGE, GDK_ACTION_MOVE));

      g_signal_connect_object (self->view_drop_target, "drop", G_CALLBACK (view_drag_drop_cb), self, G_CONNECT_SWAPPED);

      gtk_widget_add_controller (GTK_WIDGET (self->view), self->view_drop_target);
    }
  }

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VIEW]);
}

void
adw_tab_grid_attach_page (AdwTabGrid *self,
                          AdwTabPage *page,
                          int         position)
{
  g_return_if_fail (ADW_IS_TAB_GRID (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));

  page_attached_cb (self, page, position);
}

void
adw_tab_grid_detach_page (AdwTabGrid *self,
                          AdwTabPage *page)
{
  g_return_if_fail (ADW_IS_TAB_GRID (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));

  page_detached_cb (self, page);
}

void
adw_tab_grid_select_page (AdwTabGrid *self,
                          AdwTabPage *page)
{
  g_return_if_fail (ADW_IS_TAB_GRID (self));
  g_return_if_fail (page == NULL || ADW_IS_TAB_PAGE (page));

  select_page (self, page);
}

void
adw_tab_grid_try_focus_selected_tab (AdwTabGrid *self,
                                     gboolean    animate)
{
  g_return_if_fail (ADW_IS_TAB_GRID (self));

  if (!self->selected_tab)
    return;

  scroll_to_tab (self, self->selected_tab, animate ? FOCUS_ANIMATION_DURATION : 0);

  gtk_widget_grab_focus (self->selected_tab->container);
}

gboolean
adw_tab_grid_is_page_focused (AdwTabGrid *self,
                              AdwTabPage *page)
{
  TabInfo *info;

  g_return_val_if_fail (ADW_IS_TAB_GRID (self), FALSE);
  g_return_val_if_fail (ADW_IS_TAB_PAGE (page), FALSE);

  info = find_info_for_page (self, page);

  return info && gtk_widget_is_focus (info->container);
}

void
adw_tab_grid_setup_extra_drop_target (AdwTabGrid    *self,
                                      GdkDragAction  actions,
                                      GType         *types,
                                      gsize          n_types)
{
  GList *l;

  g_return_if_fail (ADW_IS_TAB_GRID (self));
  g_return_if_fail (n_types == 0 || types != NULL);

  g_clear_pointer (&self->extra_drag_types, g_free);

  self->extra_drag_actions = actions;
  self->extra_drag_types = g_memdup2 (types, sizeof (GType) * n_types);
  self->extra_drag_n_types = n_types;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    adw_tab_thumbnail_setup_extra_drop_target (info->tab,
                                               self->extra_drag_actions,
                                               self->extra_drag_types,
                                               self->extra_drag_n_types);
  }
}

gboolean
adw_tab_grid_get_inverted (AdwTabGrid *self)
{
  g_return_val_if_fail (ADW_IS_TAB_GRID (self), FALSE);

  return self->inverted;
}

void
adw_tab_grid_set_inverted (AdwTabGrid *self,
                           gboolean    inverted)
{
  GList *l;

  g_return_if_fail (ADW_IS_TAB_GRID (self));

  inverted = !!inverted;

  if (inverted == self->inverted)
    return;

  self->inverted = inverted;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    adw_tab_thumbnail_set_inverted (info->tab, inverted);
  }
}

AdwTabThumbnail *
adw_tab_grid_get_transition_thumbnail (AdwTabGrid *self)
{
  g_return_val_if_fail (ADW_IS_TAB_GRID (self), NULL);

  if (self->selected_tab)
    return self->selected_tab->tab;

  return NULL;
}

void
adw_tab_grid_set_visible_range (AdwTabGrid *self,
                                double      lower,
                                double      upper,
                                double      page_size,
                                double      lower_inset,
                                double      upper_inset)
{
  g_return_if_fail (ADW_IS_TAB_GRID (self));

  self->visible_lower = lower;
  self->visible_upper = upper;
  self->page_size = page_size;
  self->lower_inset = lower_inset;
  self->upper_inset = upper_inset;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

void
adw_tab_grid_adjustment_shifted (AdwTabGrid *self,
                                 double      delta)
{
  if (!self->drop_target_tab)
    return;

  self->drop_target_y += delta;

  set_drop_target_tab (self, find_tab_info_at (self,
                                               self->drop_target_x,
                                               self->drop_target_y));
}

double
adw_tab_grid_get_scrolled_tab_y (AdwTabGrid *self)
{
  if (!self->scroll_animation_tab)
    return NAN;

  return get_tab_y (self, self->scroll_animation_tab, TRUE);
}

void
adw_tab_grid_reset_scrolled_tab (AdwTabGrid *self)
{
  self->scroll_animation_tab = NULL;
}

void
adw_tab_grid_scroll_to_page (AdwTabGrid *self,
                             AdwTabPage *page,
                             gboolean    animate)
{
  TabInfo *info = find_info_for_page (self, page);

  if (!info)
    return;

  scroll_to_tab (self, info, animate ? FOCUS_ANIMATION_DURATION : 0);
}

void
adw_tab_grid_set_hovering (AdwTabGrid *self,
                           gboolean    hovering)
{
  self->hovering = hovering;
  update_hover (self);
}

void
adw_tab_grid_set_search_terms (AdwTabGrid *self,
                               const char *terms)
{
  self->searching = terms && *terms;
  gtk_string_filter_set_search (self->title_filter, terms);
  gtk_string_filter_set_search (self->tooltip_filter, terms);
  gtk_string_filter_set_search (self->keyword_filter, terms);

  if (!self->searching)
    set_empty (self, self->n_tabs == 0);
}

gboolean
adw_tab_grid_get_empty (AdwTabGrid *self)
{
  return self->empty;
}

gboolean
adw_tab_grid_focus_first_row (AdwTabGrid *self,
                              int         column)
{
  TabInfo *info;
  int n_tabs;

  if (!self->tabs)
    return FALSE;

  if (column < 0)
    column = MIN (self->n_tabs, self->n_columns) - 1;

  n_tabs = get_n_visible_tabs (self);
  column = CLAMP (column, 0, MIN (n_tabs, self->n_columns) - 1);

  info = find_nth_visible_tab (self, column)->data;

  scroll_to_tab (self, info, FOCUS_ANIMATION_DURATION);

  return gtk_widget_grab_focus (info->container);
}

gboolean
adw_tab_grid_focus_last_row (AdwTabGrid *self,
                             int         column)
{
  TabInfo *info;
  int last_col, n_tabs;

  if (!self->tabs)
      return FALSE;

  info = g_list_last (self->tabs)->data;

  last_col = (int) round (fmod (info->final_index, self->n_columns));
  n_tabs = get_n_visible_tabs (self);

  if (column < 0)
    column = (int) round (last_col);

  column = CLAMP (column, 0, MIN (n_tabs - 1, last_col));

  info = find_nth_visible_tab (self, n_tabs - 1 - last_col + column)->data;

  scroll_to_tab (self, info, FOCUS_ANIMATION_DURATION);

  return gtk_widget_grab_focus (info->container);
}

void
adw_tab_grid_focus_page (AdwTabGrid *self,
                         AdwTabPage *page)
{
  TabInfo *info = find_info_for_page (self, page);

  if (!info)
    return;

  scroll_to_tab (self, info, FOCUS_ANIMATION_DURATION);

  gtk_widget_grab_focus (info->container);
}

int
adw_tab_grid_measure_height_final (AdwTabGrid *self,
                                   int         for_width)
{
  int minimum;

  measure_tab_grid (self, GTK_ORIENTATION_VERTICAL, for_width, &minimum, NULL, FALSE);

  return minimum;
}

gboolean
adw_tab_grid_get_extra_drag_preload (AdwTabGrid *self)
{
  g_return_val_if_fail (ADW_IS_TAB_GRID (self), FALSE);

  return self->extra_drag_preload;
}

void
adw_tab_grid_set_extra_drag_preload (AdwTabGrid *self,
                                     gboolean    preload)
{
  GList *l;

  g_return_if_fail (ADW_IS_TAB_GRID (self));

  if (preload == self->extra_drag_preload)
    return;

  self->extra_drag_preload = preload;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    adw_tab_thumbnail_set_extra_drag_preload (info->tab, preload);
  }
}
