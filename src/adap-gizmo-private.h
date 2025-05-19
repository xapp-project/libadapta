/*
 * Copyright (C) 2020 Purism SPC
 *
 * Based on gtkgizmoprivate.h
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/5d5625dec839c00fdb572af82fbbe872ea684859/gtk/gtkgizmoprivate.h
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_GIZMO (adw_gizmo_get_type())

G_DECLARE_FINAL_TYPE (AdwGizmo, adw_gizmo, ADW, GIZMO, GtkWidget)

typedef void     (* AdwGizmoMeasureFunc)  (AdwGizmo       *self,
                                           GtkOrientation  orientation,
                                           int             for_size,
                                           int            *minimum,
                                           int            *natural,
                                           int            *minimum_baseline,
                                           int            *natural_baseline);
typedef void     (* AdwGizmoAllocateFunc) (AdwGizmo *self,
                                           int       width,
                                           int       height,
                                           int       baseline);
typedef void     (* AdwGizmoSnapshotFunc) (AdwGizmo    *self,
                                           GtkSnapshot *snapshot);
typedef gboolean (* AdwGizmoContainsFunc) (AdwGizmo *self,
                                           double    x,
                                           double    y);
typedef gboolean (* AdwGizmoFocusFunc)    (AdwGizmo         *self,
                                           GtkDirectionType  direction);
typedef gboolean (* AdwGizmoGrabFocusFunc)(AdwGizmo         *self);

GtkWidget *adw_gizmo_new (const char            *css_name,
                          AdwGizmoMeasureFunc    measure_func,
                          AdwGizmoAllocateFunc   allocate_func,
                          AdwGizmoSnapshotFunc   snapshot_func,
                          AdwGizmoContainsFunc   contains_func,
                          AdwGizmoFocusFunc      focus_func,
                          AdwGizmoGrabFocusFunc  grab_focus_func) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adw_gizmo_new_with_role (const char            *css_name,
                                    GtkAccessibleRole      role,
                                    AdwGizmoMeasureFunc    measure_func,
                                    AdwGizmoAllocateFunc   allocate_func,
                                    AdwGizmoSnapshotFunc   snapshot_func,
                                    AdwGizmoContainsFunc   contains_func,
                                    AdwGizmoFocusFunc      focus_func,
                                    AdwGizmoGrabFocusFunc  grab_focus_func) G_GNUC_WARN_UNUSED_RESULT;

void adw_gizmo_set_measure_func    (AdwGizmo              *self,
                                    AdwGizmoMeasureFunc    measure_func);
void adw_gizmo_set_allocate_func   (AdwGizmo              *self,
                                    AdwGizmoAllocateFunc   allocate_func);
void adw_gizmo_set_snapshot_func   (AdwGizmo              *self,
                                    AdwGizmoSnapshotFunc   snapshot_func);
void adw_gizmo_set_contains_func   (AdwGizmo              *self,
                                    AdwGizmoContainsFunc   contains_func);
void adw_gizmo_set_focus_func      (AdwGizmo              *self,
                                    AdwGizmoFocusFunc      focus_func);
void adw_gizmo_set_grab_focus_func (AdwGizmo              *self,
                                    AdwGizmoGrabFocusFunc  grab_focus_func);

G_END_DECLS
