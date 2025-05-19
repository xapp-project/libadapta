/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-toast-widget-private.h"

#include "adw-bin.h"

struct _AdwToastWidget {
  GtkWidget parent_instance;

  AdwBin *title_bin;
  GtkWidget *action_button;
  GtkWidget *close_button;

  AdwToast *toast;

  guint hide_timeout_id;
  gint inhibit_count;
};

enum {
  PROP_0,
  PROP_TOAST,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdwToastWidget, adw_toast_widget, GTK_TYPE_WIDGET)

static gboolean
string_is_not_empty (gpointer    user_data,
                     const char *string)
{
  return string && string[0];
}

static void
timeout_cb (AdwToastWidget *self)
{
  self->hide_timeout_id = 0;

  adw_toast_dismiss (self->toast);
}

static void
start_timeout (AdwToastWidget *self)
{
  guint timeout = adw_toast_get_timeout (self->toast);

  if (!self->hide_timeout_id && timeout)
    self->hide_timeout_id =
      g_timeout_add_once (timeout * 1000,
                          (GSourceOnceFunc) (timeout_cb),
                          self);
}

static void
end_timeout (AdwToastWidget *self)
{
  g_clear_handle_id (&self->hide_timeout_id, g_source_remove);
}

static void
inhibit_hide (AdwToastWidget *self)
{
  if (self->inhibit_count++ == 0)
    end_timeout (self);
}

static void
uninhibit_hide (AdwToastWidget  *self)
{
  g_assert (self->inhibit_count);

  if (--self->inhibit_count == 0)
    start_timeout (self);
}

static void
dismiss (AdwToastWidget *self)
{
  end_timeout (self);

  adw_toast_dismiss (self->toast);
}

static void
close_idle_cb (AdwToastWidget *self)
{
  dismiss (self);
  g_object_unref (self);
}

static void
action_clicked_cb (AdwToastWidget *self)
{
  end_timeout (self);

  g_signal_emit_by_name (self->toast, "button-clicked");

  /* Keep the widget alive through the idle. Otherwise it may be immediately
   * destroyed if animations are disabled */
  g_idle_add_once ((GSourceOnceFunc) close_idle_cb, g_object_ref (self));
}

static void
update_title_widget (AdwToastWidget *self)
{
  GtkWidget *custom_title;

  if (!self->toast) {
    adw_bin_set_child (self->title_bin, NULL);
    return;
  }

  custom_title = adw_toast_get_custom_title (self->toast);

  if (custom_title) {
    adw_bin_set_child (self->title_bin, custom_title);
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

    adw_bin_set_child (self->title_bin, title);
  }
}

static void
set_toast (AdwToastWidget *self,
           AdwToast       *toast)
{
  g_assert (ADW_IS_TOAST_WIDGET (self));
  g_assert (toast == NULL || ADW_IS_TOAST (toast));

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
adw_toast_widget_dispose (GObject *object)
{
  AdwToastWidget *self = ADW_TOAST_WIDGET (object);

  end_timeout (self);

  set_toast (self, NULL);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADW_TYPE_TOAST_WIDGET);

  G_OBJECT_CLASS (adw_toast_widget_parent_class)->dispose (object);
}

static void
adw_toast_widget_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwToastWidget *self = ADW_TOAST_WIDGET (object);

  switch (prop_id) {
  case PROP_TOAST:
    g_value_set_object (value, self->toast);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toast_widget_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwToastWidget *self = ADW_TOAST_WIDGET (object);

  switch (prop_id) {
  case PROP_TOAST:
    set_toast (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toast_widget_class_init (AdwToastWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_toast_widget_dispose;
  object_class->get_property = adw_toast_widget_get_property;
  object_class->set_property = adw_toast_widget_set_property;

  props[PROP_TOAST] =
    g_param_spec_object ("toast", NULL, NULL,
                         ADW_TYPE_TOAST,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-toast-widget.ui");

  gtk_widget_class_bind_template_child (widget_class, AdwToastWidget, title_bin);
  gtk_widget_class_bind_template_child (widget_class, AdwToastWidget, action_button);
  gtk_widget_class_bind_template_child (widget_class, AdwToastWidget, close_button);

  gtk_widget_class_bind_template_callback (widget_class, string_is_not_empty);
  gtk_widget_class_bind_template_callback (widget_class, action_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, dismiss);
  gtk_widget_class_bind_template_callback (widget_class, inhibit_hide);
  gtk_widget_class_bind_template_callback (widget_class, uninhibit_hide);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "toast");
}

static void
adw_toast_widget_init (AdwToastWidget *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
adw_toast_widget_new (AdwToast *toast)
{
  g_assert (ADW_IS_TOAST (toast));

  return g_object_new (ADW_TYPE_TOAST_WIDGET,
                       "toast", toast,
                       NULL);
}

void
adw_toast_widget_reset_timeout (AdwToastWidget *self)
{
  g_assert (ADW_IS_TOAST_WIDGET (self));

  end_timeout (self);
  start_timeout (self);
}

gboolean
adw_toast_widget_get_button_visible (AdwToastWidget *self)
{
  g_assert (ADW_IS_TOAST_WIDGET (self));

  return gtk_widget_get_visible (self->action_button);
}
