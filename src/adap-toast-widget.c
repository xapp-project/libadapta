/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-toast-widget-private.h"

#include "adap-bin.h"

struct _AdapToastWidget {
  GtkWidget parent_instance;

  AdapBin *title_bin;
  GtkWidget *action_button;
  GtkWidget *close_button;

  AdapToast *toast;

  guint hide_timeout_id;
  gint inhibit_count;
};

enum {
  PROP_0,
  PROP_TOAST,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdapToastWidget, adap_toast_widget, GTK_TYPE_WIDGET)

static gboolean
string_is_not_empty (gpointer    user_data,
                     const char *string)
{
  return string && string[0];
}

static void
timeout_cb (AdapToastWidget *self)
{
  self->hide_timeout_id = 0;

  adap_toast_dismiss (self->toast);
}

static void
start_timeout (AdapToastWidget *self)
{
  guint timeout = adap_toast_get_timeout (self->toast);

  if (!self->hide_timeout_id && timeout)
    self->hide_timeout_id =
      g_timeout_add_once (timeout * 1000,
                          (GSourceOnceFunc) (timeout_cb),
                          self);
}

static void
end_timeout (AdapToastWidget *self)
{
  g_clear_handle_id (&self->hide_timeout_id, g_source_remove);
}

static void
inhibit_hide (AdapToastWidget *self)
{
  if (self->inhibit_count++ == 0)
    end_timeout (self);
}

static void
uninhibit_hide (AdapToastWidget  *self)
{
  g_assert (self->inhibit_count);

  if (--self->inhibit_count == 0)
    start_timeout (self);
}

static void
dismiss (AdapToastWidget *self)
{
  end_timeout (self);

  adap_toast_dismiss (self->toast);
}

static void
close_idle_cb (AdapToastWidget *self)
{
  dismiss (self);
  g_object_unref (self);
}

static void
action_clicked_cb (AdapToastWidget *self)
{
  end_timeout (self);

  g_signal_emit_by_name (self->toast, "button-clicked");

  /* Keep the widget alive through the idle. Otherwise it may be immediately
   * destroyed if animations are disabled */
  g_idle_add_once ((GSourceOnceFunc) close_idle_cb, g_object_ref (self));
}

static void
update_title_widget (AdapToastWidget *self)
{
  GtkWidget *custom_title;

  if (!self->toast) {
    adap_bin_set_child (self->title_bin, NULL);
    return;
  }

  custom_title = adap_toast_get_custom_title (self->toast);

  if (custom_title) {
    adap_bin_set_child (self->title_bin, custom_title);
  } else {
    GtkWidget *title = gtk_label_new (NULL);

    gtk_label_set_ellipsize (GTK_LABEL (title), PANGO_ELLIPSIZE_END);
    gtk_label_set_xalign (GTK_LABEL (title), 0.0);
    gtk_label_set_single_line_mode (GTK_LABEL (title), TRUE);
    gtk_widget_add_css_class (title, "heading");

    g_object_bind_property (self->toast, "use-markup",
                            title, "use-markup",
                            G_BINDING_SYNC_CREATE);

    g_object_bind_property (self->toast, "title",
                            title, "label",
                            G_BINDING_SYNC_CREATE);

    adap_bin_set_child (self->title_bin, title);
  }
}

static void
set_toast (AdapToastWidget *self,
           AdapToast       *toast)
{
  g_assert (ADAP_IS_TOAST_WIDGET (self));
  g_assert (toast == NULL || ADAP_IS_TOAST (toast));

  if (self->toast) {
    end_timeout (self);

    g_signal_handlers_disconnect_by_func (self->toast,
                                          update_title_widget,
                                          self);
  }

  g_set_object (&self->toast, toast);
  update_title_widget (self);

  if (self->toast) {
    g_signal_connect_swapped (toast,
                              "notify::custom-title",
                              G_CALLBACK (update_title_widget),
                              self);

    start_timeout (self);
  }
}

static void
adap_toast_widget_dispose (GObject *object)
{
  AdapToastWidget *self = ADAP_TOAST_WIDGET (object);

  end_timeout (self);

  set_toast (self, NULL);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADAP_TYPE_TOAST_WIDGET);

  G_OBJECT_CLASS (adap_toast_widget_parent_class)->dispose (object);
}

static void
adap_toast_widget_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdapToastWidget *self = ADAP_TOAST_WIDGET (object);

  switch (prop_id) {
  case PROP_TOAST:
    g_value_set_object (value, self->toast);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_toast_widget_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdapToastWidget *self = ADAP_TOAST_WIDGET (object);

  switch (prop_id) {
  case PROP_TOAST:
    set_toast (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_toast_widget_class_init (AdapToastWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adap_toast_widget_dispose;
  object_class->get_property = adap_toast_widget_get_property;
  object_class->set_property = adap_toast_widget_set_property;

  props[PROP_TOAST] =
    g_param_spec_object ("toast", NULL, NULL,
                         ADAP_TYPE_TOAST,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-toast-widget.ui");

  gtk_widget_class_bind_template_child (widget_class, AdapToastWidget, title_bin);
  gtk_widget_class_bind_template_child (widget_class, AdapToastWidget, action_button);
  gtk_widget_class_bind_template_child (widget_class, AdapToastWidget, close_button);

  gtk_widget_class_bind_template_callback (widget_class, string_is_not_empty);
  gtk_widget_class_bind_template_callback (widget_class, action_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, dismiss);
  gtk_widget_class_bind_template_callback (widget_class, inhibit_hide);
  gtk_widget_class_bind_template_callback (widget_class, uninhibit_hide);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "toast");
}

static void
adap_toast_widget_init (AdapToastWidget *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
adap_toast_widget_new (AdapToast *toast)
{
  g_assert (ADAP_IS_TOAST (toast));

  return g_object_new (ADAP_TYPE_TOAST_WIDGET,
                       "toast", toast,
                       NULL);
}

void
adap_toast_widget_reset_timeout (AdapToastWidget *self)
{
  g_assert (ADAP_IS_TOAST_WIDGET (self));

  end_timeout (self);
  start_timeout (self);
}

gboolean
adap_toast_widget_get_button_visible (AdapToastWidget *self)
{
  g_assert (ADAP_IS_TOAST_WIDGET (self));

  return gtk_widget_get_visible (self->action_button);
}
