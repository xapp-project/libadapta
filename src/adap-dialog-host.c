/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adap-dialog-host-private.h"

#include "adap-bin.h"
#include "adap-dialog-private.h"
#include "adap-widget-utils-private.h"

struct _AdapDialogHost
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

static void adap_dialog_host_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapDialogHost, adap_dialog_host, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_dialog_host_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  PROP_DIALOGS,
  PROP_VISIBLE_DIALOG,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

#define ADAP_TYPE_DIALOG_MODEL (adap_dialog_model_get_type ())

G_DECLARE_FINAL_TYPE (AdapDialogModel, adap_dialog_model, ADAP, DIALOG_MODEL, GObject)

struct _AdapDialogModel
{
  GObject parent_instance;

  AdapDialogHost *host;
};

static GType
adap_dialog_model_get_item_type (GListModel *model)
{
  return ADAP_TYPE_DIALOG;
}

static guint
adap_dialog_model_get_n_items (GListModel *model)
{
  AdapDialogModel *self = ADAP_DIALOG_MODEL (model);

  if (G_UNLIKELY (!ADAP_IS_DIALOG_HOST (self->host)))
    return 0;

  return self->host->dialogs->len;
}

static gpointer
adap_dialog_model_get_item (GListModel *model,
                           guint       position)
{
  AdapDialogModel *self = ADAP_DIALOG_MODEL (model);
  AdapDialog *dialog;

  if (G_UNLIKELY (!ADAP_IS_DIALOG_HOST (self->host)))
    return NULL;

  dialog = g_ptr_array_index (self->host->dialogs, position);

  if (!dialog)
    return NULL;

  return g_object_ref (dialog);
}

static void
adap_dialog_model_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adap_dialog_model_get_item_type;
  iface->get_n_items = adap_dialog_model_get_n_items;
  iface->get_item = adap_dialog_model_get_item;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (AdapDialogModel, adap_dialog_model, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adap_dialog_model_list_model_init))

static void
adap_dialog_model_init (AdapDialogModel *self)
{
}

static void
adap_dialog_model_dispose (GObject *object)
{
  AdapDialogModel *self = ADAP_DIALOG_MODEL (object);

  g_clear_weak_pointer (&self->host);

  G_OBJECT_CLASS (adap_dialog_model_parent_class)->dispose (object);
}

static void
adap_dialog_model_class_init (AdapDialogModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adap_dialog_model_dispose;
}

static GListModel *
adap_dialog_model_new (AdapDialogHost *host)
{
  AdapDialogModel *model;

  model = g_object_new (ADAP_TYPE_DIALOG_MODEL, NULL);
  g_set_weak_pointer (&model->host, host);

  return G_LIST_MODEL (model);
}

static gboolean
close_request_cb (AdapDialogHost *self)
{
  if (self->dialogs->len > 0) {
    AdapDialog *dialog = adap_dialog_host_get_visible_dialog (self);

    adap_dialog_close (dialog);

    return GDK_EVENT_STOP;
  }

  return GDK_EVENT_PROPAGATE;
}

static void
dialog_closing_cb (AdapDialog     *dialog,
                   AdapDialogHost *self)

{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  guint index;

  g_assert (g_ptr_array_find (self->dialogs, dialog, &index));

  g_ptr_array_remove (self->dialogs, dialog);

  adap_dialog_set_closing (dialog, TRUE);

  if (self->dialogs_model)
    g_list_model_items_changed (self->dialogs_model, index, 1, 0);

  if (self->dialogs->len == 0) {
    gtk_widget_set_can_focus (self->bin, TRUE);
    gtk_widget_set_can_target (self->bin, TRUE);

    if (root && self->last_focus)
      gtk_window_set_focus (GTK_WINDOW (root), self->last_focus);

    g_clear_weak_pointer (&self->last_focus);
  } else {
    AdapDialog *next_dialog = adap_dialog_host_get_visible_dialog (self);

    adap_dialog_set_shadowed (next_dialog, FALSE);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_DIALOG]);
}

static void
dialog_remove_cb (AdapDialog     *dialog,
                  AdapDialogHost *self)
{
  if (!adap_dialog_get_closing (dialog))
    return;

  adap_dialog_set_closing (dialog, FALSE);

  adap_dialog_set_callbacks (dialog, NULL, NULL, NULL);

  if (self->within_unmap)
    g_ptr_array_add (self->dialogs_closed_during_unmap, dialog);
  else
    gtk_widget_unparent (GTK_WIDGET (dialog));
}

static gboolean
key_pressed_cb (AdapDialogHost *self)
{
  if (self->dialogs->len == 0)
    return GDK_EVENT_PROPAGATE;

  return GDK_EVENT_STOP;
}

static void
adap_dialog_host_root (GtkWidget *widget)
{
  AdapDialogHost *self = ADAP_DIALOG_HOST (widget);
  GtkRoot *root;

  GTK_WIDGET_CLASS (adap_dialog_host_parent_class)->root (widget);

  root = gtk_widget_get_root (GTK_WIDGET (widget));

  g_signal_connect_swapped (root, "close-request",
                            G_CALLBACK (close_request_cb), self);
}

static void
adap_dialog_host_unroot (GtkWidget *widget)
{
  AdapDialogHost *self = ADAP_DIALOG_HOST (widget);
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (widget));

  g_signal_handlers_disconnect_by_func (root, close_request_cb, self);

  GTK_WIDGET_CLASS (adap_dialog_host_parent_class)->unroot (widget);
}

static void
adap_dialog_host_unmap (GtkWidget *widget)
{
  AdapDialogHost *self = ADAP_DIALOG_HOST (widget);
  int i;

  self->within_unmap = TRUE;

  GTK_WIDGET_CLASS (adap_dialog_host_parent_class)->unmap (widget);

  self->within_unmap = FALSE;

  for (i = 0; i < self->dialogs_closed_during_unmap->len; i++) {
    AdapDialog *dialog = g_ptr_array_index (self->dialogs_closed_during_unmap, i);

    gtk_widget_unparent (GTK_WIDGET (dialog));
  }

  g_ptr_array_remove_range (self->dialogs_closed_during_unmap, 0,
                            self->dialogs_closed_during_unmap->len);
}

static void
adap_dialog_host_measure (GtkWidget      *widget,
                         GtkOrientation  orientation,
                         int             for_size,
                         int            *minimum,
                         int            *natural,
                         int            *minimum_baseline,
                         int            *natural_baseline)
{
  AdapDialogHost *self = ADAP_DIALOG_HOST (widget);

  gtk_widget_measure (self->bin, orientation, for_size,
                      minimum, natural, minimum_baseline, natural_baseline);
}

static void
adap_dialog_host_size_allocate (GtkWidget *widget,
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
adap_dialog_host_dispose (GObject *object)
{
  AdapDialogHost *self = ADAP_DIALOG_HOST (object);

  if (self->dialogs_model)
    g_list_model_items_changed (self->dialogs_model, 0, self->dialogs->len, 0);

  if (self->dialogs) {
    int i;

    for (i = 0; i < self->dialogs->len; i++) {
      AdapDialog *dialog = g_ptr_array_index (self->dialogs, i);

      adap_dialog_set_callbacks (dialog, NULL, NULL, NULL);

      gtk_widget_unparent (GTK_WIDGET (dialog));
    }
  }

  g_clear_weak_pointer (&self->last_focus);

  g_clear_pointer (&self->dialogs, g_ptr_array_unref);
  g_clear_pointer (&self->dialogs_closed_during_unmap, g_ptr_array_unref);

  g_clear_pointer (&self->bin, gtk_widget_unparent);

  G_OBJECT_CLASS (adap_dialog_host_parent_class)->dispose (object);
}

static void
adap_dialog_host_finalize (GObject *object)
{
  AdapDialogHost *self = ADAP_DIALOG_HOST (object);

  g_clear_weak_pointer (&self->dialogs_model);

  G_OBJECT_CLASS (adap_dialog_host_parent_class)->finalize (object);
}

static void
adap_dialog_host_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdapDialogHost *self = ADAP_DIALOG_HOST (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adap_dialog_host_get_child (self));
    break;
  case PROP_DIALOGS:
    g_value_take_object (value, adap_dialog_host_get_dialogs (self));
    break;
  case PROP_VISIBLE_DIALOG:
    g_value_set_object (value, adap_dialog_host_get_visible_dialog (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_dialog_host_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  AdapDialogHost *self = ADAP_DIALOG_HOST (object);

  switch (prop_id) {
  case PROP_CHILD:
    adap_dialog_host_set_child (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_dialog_host_class_init (AdapDialogHostClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_dialog_host_dispose;
  object_class->finalize = adap_dialog_host_finalize;
  object_class->get_property = adap_dialog_host_get_property;
  object_class->set_property = adap_dialog_host_set_property;

  widget_class->root = adap_dialog_host_root;
  widget_class->unroot = adap_dialog_host_unroot;
  widget_class->unmap = adap_dialog_host_unmap;
  widget_class->measure = adap_dialog_host_measure;
  widget_class->size_allocate = adap_dialog_host_size_allocate;
  widget_class->get_request_mode = adap_widget_get_request_mode;
  widget_class->compute_expand = adap_widget_compute_expand;

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
                         ADAP_TYPE_DIALOG,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "dialog-host");
}

static void
adap_dialog_host_init (AdapDialogHost *self)
{
  GtkEventController *controller;

  self->dialogs = g_ptr_array_new ();

  self->dialogs_closed_during_unmap = g_ptr_array_new ();

  self->bin = adap_bin_new ();
  gtk_widget_set_parent (self->bin, GTK_WIDGET (self));

  controller = gtk_event_controller_key_new ();
  g_signal_connect_swapped (controller, "key-pressed", G_CALLBACK (key_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);
}

static void
adap_dialog_host_buildable_add_child (GtkBuildable *buildable,
                                     GtkBuilder   *builder,
                                     GObject      *child,
                                     const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adap_dialog_host_set_child (ADAP_DIALOG_HOST (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_dialog_host_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_dialog_host_buildable_add_child;
}

GtkWidget *
adap_dialog_host_new (void)
{
  return g_object_new (ADAP_TYPE_DIALOG_HOST, NULL);
}

GtkWidget *
adap_dialog_host_get_child (AdapDialogHost *self)
{
  g_return_val_if_fail (ADAP_IS_DIALOG_HOST (self), NULL);

  return adap_bin_get_child (ADAP_BIN (self->bin));
}

void
adap_dialog_host_set_child (AdapDialogHost *self,
                           GtkWidget     *child)
{
  g_return_if_fail (ADAP_IS_DIALOG_HOST (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (adap_dialog_host_get_child (self) == child)
    return;

  adap_bin_set_child (ADAP_BIN (self->bin), child);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

GListModel *
adap_dialog_host_get_dialogs (AdapDialogHost *self)
{
  g_return_val_if_fail (ADAP_IS_DIALOG_HOST (self), NULL);

  if (self->dialogs_model)
    return g_object_ref (self->dialogs_model);

  g_set_weak_pointer (&self->dialogs_model, adap_dialog_model_new (self));

  return self->dialogs_model;
}

void
adap_dialog_host_present_dialog (AdapDialogHost *self,
                                AdapDialog     *dialog)
{
  GtkRoot *root;
  gboolean closing;
  guint index;

  g_return_if_fail (ADAP_IS_DIALOG_HOST (self));
  g_return_if_fail (ADAP_IS_DIALOG (dialog));

  root = gtk_widget_get_root (GTK_WIDGET (self));

  g_return_if_fail (GTK_IS_WINDOW (root));

  if (g_ptr_array_find (self->dialogs, dialog, &index)) {
    AdapDialog *last_dialog = adap_dialog_host_get_visible_dialog (self);

    if (dialog == last_dialog)
      return;

    /* Raise the dialog to the top */
    gtk_widget_insert_before (GTK_WIDGET (dialog), GTK_WIDGET (self), NULL);

    adap_dialog_set_shadowed (last_dialog, TRUE);
    adap_dialog_set_shadowed (dialog, FALSE);

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

  closing = adap_dialog_get_closing (dialog);
  adap_dialog_set_closing (dialog, FALSE);

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
    AdapDialog *last_dialog = adap_dialog_host_get_visible_dialog (self);

    adap_dialog_set_shadowed (last_dialog, TRUE);
  }

  if (!closing) {
    adap_dialog_set_callbacks (dialog,
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

AdapDialog *
adap_dialog_host_get_visible_dialog (AdapDialogHost *self)
{
  g_return_val_if_fail (ADAP_IS_DIALOG_HOST (self), NULL);

  if (self->dialogs->len == 0)
    return NULL;

  return g_ptr_array_index (self->dialogs, self->dialogs->len - 1);
}

GtkWidget *
adap_dialog_host_get_proxy (AdapDialogHost *self)
{
  g_return_val_if_fail (ADAP_IS_DIALOG_HOST (self), NULL);

  return self->proxy;
}

void
adap_dialog_host_set_proxy (AdapDialogHost *self,
                           GtkWidget     *proxy)
{
  g_return_if_fail (ADAP_IS_DIALOG_HOST (self));
  g_return_if_fail (proxy == NULL || GTK_IS_WIDGET (proxy));
  g_return_if_fail (adap_dialog_host_get_from_proxy (proxy) == NULL);

  if (self->proxy)
    g_object_set_data (G_OBJECT (self->proxy), "-adap-dialog-host-proxy", NULL);

  self->proxy = proxy;

  if (self->proxy)
    g_object_set_data (G_OBJECT (self->proxy), "-adap-dialog-host-proxy", self);
}

AdapDialogHost *
adap_dialog_host_get_from_proxy (GtkWidget *widget)
{
  gpointer data = g_object_get_data (G_OBJECT (widget), "-adap-dialog-host-proxy");

  if (ADAP_IS_DIALOG_HOST (data))
    return ADAP_DIALOG_HOST (data);

  return NULL;
}

