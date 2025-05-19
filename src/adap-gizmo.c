/*
 * Copyright (C) 2020 Purism SPC
 *
 * Based on gtkgizmo.c
 * https://gitlab.gnome.org/GNOME/gtk/-/blob/5d5625dec839c00fdb572af82fbbe872ea684859/gtk/gtkgizmo.c
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "adap-gizmo-private.h"

#include "adap-widget-utils-private.h"

struct _AdapGizmo
{
  GtkWidget parent_instance;

  AdapGizmoMeasureFunc   measure_func;
  AdapGizmoAllocateFunc  allocate_func;
  AdapGizmoSnapshotFunc  snapshot_func;
  AdapGizmoContainsFunc  contains_func;
  AdapGizmoFocusFunc     focus_func;
  AdapGizmoGrabFocusFunc grab_focus_func;
};

G_DEFINE_FINAL_TYPE (AdapGizmo, adap_gizmo, GTK_TYPE_WIDGET)

static void
adap_gizmo_measure (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  AdapGizmo *self = ADAP_GIZMO (widget);

  if (self->measure_func)
    self->measure_func (self, orientation, for_size,
                        minimum, natural,
                        minimum_baseline, natural_baseline);
}

static void
adap_gizmo_size_allocate (GtkWidget *widget,
                         int        width,
                         int        height,
                         int        baseline)
{
  AdapGizmo *self = ADAP_GIZMO (widget);

  if (self->allocate_func)
    self->allocate_func (self, width, height, baseline);
}

static void
adap_gizmo_snapshot (GtkWidget   *widget,
                    GtkSnapshot *snapshot)
{
  AdapGizmo *self = ADAP_GIZMO (widget);

  if (self->snapshot_func)
    self->snapshot_func (self, snapshot);
  else
    GTK_WIDGET_CLASS (adap_gizmo_parent_class)->snapshot (widget, snapshot);
}

static gboolean
adap_gizmo_contains (GtkWidget *widget,
                    double     x,
                    double     y)
{
  AdapGizmo *self = ADAP_GIZMO (widget);

  if (self->contains_func)
    return self->contains_func (self, x, y);
  else
    return GTK_WIDGET_CLASS (adap_gizmo_parent_class)->contains (widget, x, y);
}

static gboolean
adap_gizmo_focus (GtkWidget        *widget,
                 GtkDirectionType  direction)
{
  AdapGizmo *self = ADAP_GIZMO (widget);

  if (self->focus_func)
    return self->focus_func (self, direction);

  return FALSE;
}

static gboolean
adap_gizmo_grab_focus (GtkWidget *widget)
{
  AdapGizmo *self = ADAP_GIZMO (widget);

  if (self->grab_focus_func)
    return self->grab_focus_func (self);

  return FALSE;
}

static void
adap_gizmo_dispose (GObject *object)
{
  AdapGizmo *self = ADAP_GIZMO (object);
  GtkWidget *widget = gtk_widget_get_first_child (GTK_WIDGET (self));

  while (widget) {
    GtkWidget *next = gtk_widget_get_next_sibling (widget);

    gtk_widget_unparent (widget);

    widget = next;
  }

  G_OBJECT_CLASS (adap_gizmo_parent_class)->dispose (object);
}

static void
adap_gizmo_class_init (AdapGizmoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_gizmo_dispose;

  widget_class->measure = adap_gizmo_measure;
  widget_class->size_allocate = adap_gizmo_size_allocate;
  widget_class->snapshot = adap_gizmo_snapshot;
  widget_class->contains = adap_gizmo_contains;
  widget_class->grab_focus = adap_gizmo_grab_focus;
  widget_class->focus = adap_gizmo_focus;
  widget_class->compute_expand = adap_widget_compute_expand;
}

static void
adap_gizmo_init (AdapGizmo *self)
{
}

GtkWidget *
adap_gizmo_new (const char            *css_name,
               AdapGizmoMeasureFunc    measure_func,
               AdapGizmoAllocateFunc   allocate_func,
               AdapGizmoSnapshotFunc   snapshot_func,
               AdapGizmoContainsFunc   contains_func,
               AdapGizmoFocusFunc      focus_func,
               AdapGizmoGrabFocusFunc  grab_focus_func)
{
  AdapGizmo *gizmo = g_object_new (ADAP_TYPE_GIZMO,
                                  "css-name", css_name,
                                  NULL);

  gizmo->measure_func  = measure_func;
  gizmo->allocate_func = allocate_func;
  gizmo->snapshot_func = snapshot_func;
  gizmo->contains_func = contains_func;
  gizmo->focus_func = focus_func;
  gizmo->grab_focus_func = grab_focus_func;

  return GTK_WIDGET (gizmo);
}

GtkWidget *
adap_gizmo_new_with_role (const char            *css_name,
                         GtkAccessibleRole      role,
                         AdapGizmoMeasureFunc    measure_func,
                         AdapGizmoAllocateFunc   allocate_func,
                         AdapGizmoSnapshotFunc   snapshot_func,
                         AdapGizmoContainsFunc   contains_func,
                         AdapGizmoFocusFunc      focus_func,
                         AdapGizmoGrabFocusFunc  grab_focus_func)
{
  AdapGizmo *gizmo = ADAP_GIZMO (g_object_new (ADAP_TYPE_GIZMO,
                                             "css-name", css_name,
                                             "accessible-role", role,
                                             NULL));

  gizmo->measure_func  = measure_func;
  gizmo->allocate_func = allocate_func;
  gizmo->snapshot_func = snapshot_func;
  gizmo->contains_func = contains_func;
  gizmo->focus_func = focus_func;
  gizmo->grab_focus_func = grab_focus_func;

  return GTK_WIDGET (gizmo);
}

void
adap_gizmo_set_measure_func (AdapGizmo            *self,
                            AdapGizmoMeasureFunc  measure_func)
{
  self->measure_func = measure_func;

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

void
adap_gizmo_set_allocate_func (AdapGizmo             *self,
                             AdapGizmoAllocateFunc  allocate_func)
{
  self->allocate_func = allocate_func;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

void
adap_gizmo_set_snapshot_func (AdapGizmo             *self,
                             AdapGizmoSnapshotFunc  snapshot_func)
{
  self->snapshot_func = snapshot_func;

  gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
adap_gizmo_set_contains_func (AdapGizmo             *self,
                             AdapGizmoContainsFunc  contains_func)
{
  self->contains_func = contains_func;

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

void
adap_gizmo_set_focus_func (AdapGizmo          *self,
                          AdapGizmoFocusFunc  focus_func)
{
  self->focus_func = focus_func;
}

void
adap_gizmo_set_grab_focus_func (AdapGizmo              *self,
                               AdapGizmoGrabFocusFunc  grab_focus_func)
{
  self->grab_focus_func = grab_focus_func;
}
