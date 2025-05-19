/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-dialog-host-private.h"

#include "adw-bin.h"
#include "adw-dialog-private.h"
#include "adw-widget-utils-private.h"

struct _AdwDialogHost
{
  GtkWidget parent_instance;

  GtkWidget *bin;

  GPtrArray *dialogs;
  GListModel *dialogs_model;

  gboolean within_unmap;
  GPtrArray *dialogs_closed_during_unmap;

  GtkWidget *last_focus;

  GtkWidget *proxy;
};

static void adw_dialog_host_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwDialogHost, adw_dialog_host, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_dialog_host_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  PROP_DIALOGS,
  PROP_VISIBLE_DIALOG,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

#define ADW_TYPE_DIALOG_MODEL (adw_dialog_model_get_type ())

G_DECLARE_FINAL_TYPE (AdwDialogModel, adw_dialog_model, ADW, DIALOG_MODEL, GObject)

struct _AdwDialogModel
{
  GObject parent_instance;

  AdwDialogHost *host;
};

static GType
adw_dialog_model_get_item_type (GListModel *model)
{
  return ADW_TYPE_DIALOG;
}

static guint
adw_dialog_model_get_n_items (GListModel *model)
{
  AdwDialogModel *self = ADW_DIALOG_MODEL (model);

  if (G_UNLIKELY (!ADW_IS_DIALOG_HOST (self->host)))
    return 0;

  return self->host->dialogs->len;
}

static gpointer
adw_dialog_model_get_item (GListModel *model,
                           guint       position)
{
  AdwDialogModel *self = ADW_DIALOG_MODEL (model);
  AdwDialog *dialog;

  if (G_UNLIKELY (!ADW_IS_DIALOG_HOST (self->host)))
    return NULL;

  dialog = g_ptr_array_index (self->host->dialogs, position);

  if (!dialog)
    return NULL;

  return g_object_ref (dialog);
}

static void
adw_dialog_model_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_dialog_model_get_item_type;
  iface->get_n_items = adw_dialog_model_get_n_items;
  iface->get_item = adw_dialog_model_get_item;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwDialogModel, adw_dialog_model, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_dialog_model_list_model_init))

static void
adw_dialog_model_init (AdwDialogModel *self)
{
}

static void
adw_dialog_model_dispose (GObject *object)
{
  AdwDialogModel *self = ADW_DIALOG_MODEL (object);

  g_clear_weak_pointer (&self->host);

  G_OBJECT_CLASS (adw_dialog_model_parent_class)->dispose (object);
}

static void
adw_dialog_model_class_init (AdwDialogModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_dialog_model_dispose;
}

static GListModel *
adw_dialog_model_new (AdwDialogHost *host)
{
  AdwDialogModel *model;

  model = g_object_new (ADW_TYPE_DIALOG_MODEL, NULL);
  g_set_weak_pointer (&model->host, host);

  return G_LIST_MODEL (model);
}

static gboolean
close_request_cb (AdwDialogHost *self)
{
  if (self->dialogs->len > 0) {
    AdwDialog *dialog = adw_dialog_host_get_visible_dialog (self);

    adw_dialog_close (dialog);

    return GDK_EVENT_STOP;
  }

  return GDK_EVENT_PROPAGATE;
}

static void
dialog_closing_cb (AdwDialog     *dialog,
                   AdwDialogHost *self)

{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  guint index;

  g_assert (g_ptr_array_find (self->dialogs, dialog, &index));

  g_ptr_array_remove (self->dialogs, dialog);

  adw_dialog_set_closing (dialog, TRUE);

  if (self->dialogs_model)
    g_list_model_items_changed (self->dialogs_model, index, 1, 0);

  if (self->dialogs->len == 0) {
    gtk_widget_set_can_focus (self->bin, TRUE);
    gtk_widget_set_can_target (self->bin, TRUE);

    if (root && self->last_focus)
      gtk_window_set_focus (GTK_WINDOW (root), self->last_focus);

    g_clear_weak_pointer (&self->last_focus);
  } else {
    AdwDialog *next_dialog = adw_dialog_host_get_visible_dialog (self);

    adw_dialog_set_shadowed (next_dialog, FALSE);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_DIALOG]);
}

static void
dialog_remove_cb (AdwDialog     *dialog,
                  AdwDialogHost *self)
{
  if (!adw_dialog_get_closing (dialog))
    return;

  adw_dialog_set_closing (dialog, FALSE);

  adw_dialog_set_callbacks (dialog, NULL, NULL, NULL);

  if (self->within_unmap)
    g_ptr_array_add (self->dialogs_closed_during_unmap, dialog);
  else
    gtk_widget_unparent (GTK_WIDGET (dialog));
}

static gboolean
key_pressed_cb (AdwDialogHost *self)
{
  if (self->dialogs->len == 0)
    return GDK_EVENT_PROPAGATE;

  return GDK_EVENT_STOP;
}

static void
adw_dialog_host_root (GtkWidget *widget)
{
  AdwDialogHost *self = ADW_DIALOG_HOST (widget);
  GtkRoot *root;

  GTK_WIDGET_CLASS (adw_dialog_host_parent_class)->root (widget);

  root = gtk_widget_get_root (GTK_WIDGET (widget));

  g_signal_connect_swapped (root, "close-request",
                            G_CALLBACK (close_request_cb), self);
}

static void
adw_dialog_host_unroot (GtkWidget *widget)
{
  AdwDialogHost *self = ADW_DIALOG_HOST (widget);
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (widget));

  g_signal_handlers_disconnect_by_func (root, close_request_cb, self);

  GTK_WIDGET_CLASS (adw_dialog_host_parent_class)->unroot (widget);
}

static void
adw_dialog_host_unmap (GtkWidget *widget)
{
  AdwDialogHost *self = ADW_DIALOG_HOST (widget);
  int i;

  self->within_unmap = TRUE;

  GTK_WIDGET_CLASS (adw_dialog_host_parent_class)->unmap (widget);

  self->within_unmap = FALSE;

  for (i = 0; i < self->dialogs_closed_during_unmap->len; i++) {
    AdwDialog *dialog = g_ptr_array_index (self->dialogs_closed_during_unmap, i);

    gtk_widget_unparent (GTK_WIDGET (dialog));
  }

  g_ptr_array_remove_range (self->dialogs_closed_during_unmap, 0,
                            self->dialogs_closed_during_unmap->len);
}

static void
adw_dialog_host_measure (GtkWidget      *widget,
                         GtkOrientation  orientation,
                         int             for_size,
                         int            *minimum,
                         int            *natural,
                         int            *minimum_baseline,
                         int            *natural_baseline)
{
  AdwDialogHost *self = ADW_DIALOG_HOST (widget);

  gtk_widget_measure (self->bin, orientation, for_size,
                      minimum, natural, minimum_baseline, natural_baseline);
}

static void
adw_dialog_host_size_allocate (GtkWidget *widget,
                               int        width,
                               int        height,
                               int        baseline)
{
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child;
       child = gtk_widget_get_next_sibling (child)) {
    GtkRequisition min;

    gtk_widget_get_preferred_size (child, &min, NULL);

    width = MAX (width, min.width);
    height = MAX (height, min.height);

    gtk_widget_allocate (child, width, height, baseline, NULL);
  }
}

static void
adw_dialog_host_dispose (GObject *object)
{
  AdwDialogHost *self = ADW_DIALOG_HOST (object);

  if (self->dialogs_model)
    g_list_model_items_changed (self->dialogs_model, 0, self->dialogs->len, 0);

  if (self->dialogs) {
    int i;

    for (i = 0; i < self->dialogs->len; i++) {
      AdwDialog *dialog = g_ptr_array_index (self->dialogs, i);

      adw_dialog_set_callbacks (dialog, NULL, NULL, NULL);

      gtk_widget_unparent (GTK_WIDGET (dialog));
    }
  }

  g_clear_weak_pointer (&self->last_focus);

  g_clear_pointer (&self->dialogs, g_ptr_array_unref);
  g_clear_pointer (&self->dialogs_closed_during_unmap, g_ptr_array_unref);

  g_clear_pointer (&self->bin, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_dialog_host_parent_class)->dispose (object);
}

static void
adw_dialog_host_finalize (GObject *object)
{
  AdwDialogHost *self = ADW_DIALOG_HOST (object);

  g_clear_weak_pointer (&self->dialogs_model);

  G_OBJECT_CLASS (adw_dialog_host_parent_class)->finalize (object);
}

static void
adw_dialog_host_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdwDialogHost *self = ADW_DIALOG_HOST (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_dialog_host_get_child (self));
    break;
  case PROP_DIALOGS:
    g_value_take_object (value, adw_dialog_host_get_dialogs (self));
    break;
  case PROP_VISIBLE_DIALOG:
    g_value_set_object (value, adw_dialog_host_get_visible_dialog (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_dialog_host_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  AdwDialogHost *self = ADW_DIALOG_HOST (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_dialog_host_set_child (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_dialog_host_class_init (AdwDialogHostClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_dialog_host_dispose;
  object_class->finalize = adw_dialog_host_finalize;
  object_class->get_property = adw_dialog_host_get_property;
  object_class->set_property = adw_dialog_host_set_property;

  widget_class->root = adw_dialog_host_root;
  widget_class->unroot = adw_dialog_host_unroot;
  widget_class->unmap = adw_dialog_host_unmap;
  widget_class->measure = adw_dialog_host_measure;
  widget_class->size_allocate = adw_dialog_host_size_allocate;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;

  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_DIALOGS] =
    g_param_spec_object ("dialogs", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_VISIBLE_DIALOG] =
    g_param_spec_object ("visible-dialog", NULL, NULL,
                         ADW_TYPE_DIALOG,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "dialog-host");
}

static void
adw_dialog_host_init (AdwDialogHost *self)
{
  GtkEventController *controller;

  self->dialogs = g_ptr_array_new ();

  self->dialogs_closed_during_unmap = g_ptr_array_new ();

  self->bin = adw_bin_new ();
  gtk_widget_set_parent (self->bin, GTK_WIDGET (self));

  controller = gtk_event_controller_key_new ();
  g_signal_connect_swapped (controller, "key-pressed", G_CALLBACK (key_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);
}

static void
adw_dialog_host_buildable_add_child (GtkBuildable *buildable,
                                     GtkBuilder   *builder,
                                     GObject      *child,
                                     const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adw_dialog_host_set_child (ADW_DIALOG_HOST (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_dialog_host_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_dialog_host_buildable_add_child;
}

GtkWidget *
adw_dialog_host_new (void)
{
  return g_object_new (ADW_TYPE_DIALOG_HOST, NULL);
}

GtkWidget *
adw_dialog_host_get_child (AdwDialogHost *self)
{
  g_return_val_if_fail (ADW_IS_DIALOG_HOST (self), NULL);

  return adw_bin_get_child (ADW_BIN (self->bin));
}

void
adw_dialog_host_set_child (AdwDialogHost *self,
                           GtkWidget     *child)
{
  g_return_if_fail (ADW_IS_DIALOG_HOST (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (adw_dialog_host_get_child (self) == child)
    return;

  adw_bin_set_child (ADW_BIN (self->bin), child);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

GListModel *
adw_dialog_host_get_dialogs (AdwDialogHost *self)
{
  g_return_val_if_fail (ADW_IS_DIALOG_HOST (self), NULL);

  if (self->dialogs_model)
    return g_object_ref (self->dialogs_model);

  g_set_weak_pointer (&self->dialogs_model, adw_dialog_model_new (self));

  return self->dialogs_model;
}

void
adw_dialog_host_present_dialog (AdwDialogHost *self,
                                AdwDialog     *dialog)
{
  GtkRoot *root;
  gboolean closing;
  guint index;

  g_return_if_fail (ADW_IS_DIALOG_HOST (self));
  g_return_if_fail (ADW_IS_DIALOG (dialog));

  root = gtk_widget_get_root (GTK_WIDGET (self));

  g_return_if_fail (GTK_IS_WINDOW (root));

  if (g_ptr_array_find (self->dialogs, dialog, &index)) {
    AdwDialog *last_dialog = adw_dialog_host_get_visible_dialog (self);

    if (dialog == last_dialog)
      return;

    /* Raise the dialog to the top */
    gtk_widget_insert_before (GTK_WIDGET (dialog), GTK_WIDGET (self), NULL);

    adw_dialog_set_shadowed (last_dialog, TRUE);
    adw_dialog_set_shadowed (dialog, FALSE);

    g_ptr_array_remove (self->dialogs, dialog);
    g_ptr_array_add (self->dialogs, dialog);

    if (self->dialogs_model) {
      g_list_model_items_changed (self->dialogs_model, index,
                                  self->dialogs->len - index,
                                  self->dialogs->len - index);
    }

    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_DIALOG]);

    return;
  }

  closing = adw_dialog_get_closing (dialog);
  adw_dialog_set_closing (dialog, FALSE);

  if (self->dialogs->len == 0) {
    GtkWidget *focus = gtk_window_get_focus (GTK_WINDOW (root));

    while (focus && !gtk_widget_get_mapped (focus))
      focus = gtk_widget_get_parent (focus);

    if (focus && gtk_widget_is_ancestor (focus, self->bin))
      g_set_weak_pointer (&self->last_focus, focus);

    gtk_widget_set_can_focus (self->bin, FALSE);
    gtk_widget_set_can_target (self->bin, FALSE);
    gtk_window_set_focus (GTK_WINDOW (root), NULL);
  } else {
    AdwDialog *last_dialog = adw_dialog_host_get_visible_dialog (self);

    adw_dialog_set_shadowed (last_dialog, TRUE);
  }

  if (!closing) {
    adw_dialog_set_callbacks (dialog,
                              (GFunc) dialog_closing_cb,
                              (GFunc) dialog_remove_cb,
                              self);

    gtk_widget_insert_before (GTK_WIDGET (dialog), GTK_WIDGET (self), NULL);
  }

  g_ptr_array_add (self->dialogs, dialog);

  if (self->dialogs_model)
    g_list_model_items_changed (self->dialogs_model, self->dialogs->len - 1, 0, 1);

  if (gtk_window_get_focus_visible (GTK_WINDOW (root)))
    gtk_window_set_focus_visible (GTK_WINDOW (root), TRUE);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_DIALOG]);
}

AdwDialog *
adw_dialog_host_get_visible_dialog (AdwDialogHost *self)
{
  g_return_val_if_fail (ADW_IS_DIALOG_HOST (self), NULL);

  if (self->dialogs->len == 0)
    return NULL;

  return g_ptr_array_index (self->dialogs, self->dialogs->len - 1);
}

GtkWidget *
adw_dialog_host_get_proxy (AdwDialogHost *self)
{
  g_return_val_if_fail (ADW_IS_DIALOG_HOST (self), NULL);

  return self->proxy;
}

void
adw_dialog_host_set_proxy (AdwDialogHost *self,
                           GtkWidget     *proxy)
{
  g_return_if_fail (ADW_IS_DIALOG_HOST (self));
  g_return_if_fail (proxy == NULL || GTK_IS_WIDGET (proxy));
  g_return_if_fail (adw_dialog_host_get_from_proxy (proxy) == NULL);

  if (self->proxy)
    g_object_set_data (G_OBJECT (self->proxy), "-adw-dialog-host-proxy", NULL);

  self->proxy = proxy;

  if (self->proxy)
    g_object_set_data (G_OBJECT (self->proxy), "-adw-dialog-host-proxy", self);
}

AdwDialogHost *
adw_dialog_host_get_from_proxy (GtkWidget *widget)
{
  gpointer data = g_object_get_data (G_OBJECT (widget), "-adw-dialog-host-proxy");

  if (ADW_IS_DIALOG_HOST (data))
    return ADW_DIALOG_HOST (data);

  return NULL;
}

