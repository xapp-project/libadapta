/*
 * Copyright (C) 2020 Purism SPC
 *
 * Based on gtkgizmoprivate.h
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/5d5625dec839c00fdb572af82fbbe872ea684859/gtk/gtkgizmoprivate.h
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADAP_TYPE_GIZMO (adap_gizmo_get_type())

G_DECLARE_FINAL_TYPE (AdapGizmo, adap_gizmo, ADAP, GIZMO, GtkWidget)

typedef void     (* AdapGizmoMeasureFunc)  (AdapGizmo       *self,
                                           GtkOrientation  orientation,
                                           int             for_size,
                                           int            *minimum,
                                           int            *natural,
                                           int            *minimum_baseline,
                                           int            *natural_baseline);
typedef void     (* AdapGizmoAllocateFunc) (AdapGizmo *self,
                                           int       width,
                                           int       height,
                                           int       baseline);
typedef void     (* AdapGizmoSnapshotFunc) (AdapGizmo    *self,
                                           GtkSnapshot *snapshot);
typedef gboolean (* AdapGizmoContainsFunc) (AdapGizmo *self,
                                           double    x,
                                           double    y);
typedef gboolean (* AdapGizmoFocusFunc)    (AdapGizmo         *self,
                                           GtkDirectionType  direction);
typedef gboolean (* AdapGizmoGrabFocusFunc)(AdapGizmo         *self);

GtkWidget *adap_gizmo_new (const char            *css_name,
                          AdapGizmoMeasureFunc    measure_func,
                          AdapGizmoAllocateFunc   allocate_func,
                          AdapGizmoSnapshotFunc   snapshot_func,
                          AdapGizmoContainsFunc   contains_func,
                          AdapGizmoFocusFunc      focus_func,
                          AdapGizmoGrabFocusFunc  grab_focus_func) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adap_gizmo_new_with_role (const char            *css_name,
                                    GtkAccessibleRole      role,
                                    AdapGizmoMeasureFunc    measure_func,
                                    AdapGizmoAllocateFunc   allocate_func,
                                    AdapGizmoSnapshotFunc   snapshot_func,
                                    AdapGizmoContainsFunc   contains_func,
                                    AdapGizmoFocusFunc      focus_func,
                                    AdapGizmoGrabFocusFunc  grab_focus_func) G_GNUC_WARN_UNUSED_RESULT;

void adap_gizmo_set_measure_func    (AdapGizmo              *self,
                                    AdapGizmoMeasureFunc    measure_func);
void adap_gizmo_set_allocate_func   (AdapGizmo              *self,
                                    AdapGizmoAllocateFunc   allocate_func);
void adap_gizmo_set_snapshot_func   (AdapGizmo              *self,
                                    AdapGizmoSnapshotFunc   snapshot_func);
void adap_gizmo_set_contains_func   (AdapGizmo              *self,
                                    AdapGizmoContainsFunc   contains_func);
void adap_gizmo_set_focus_func      (AdapGizmo              *self,
                                    AdapGizmoFocusFunc      focus_func);
void adap_gizmo_set_grab_focus_func (AdapGizmo              *self,
                                    AdapGizmoGrabFocusFunc  grab_focus_func);

G_END_DECLS
