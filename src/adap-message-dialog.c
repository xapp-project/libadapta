/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adap-message-dialog.h"

#include "adap-gizmo-private.h"
#include "adap-gtkbuilder-utils-private.h"
#include "adap-marshalers.h"
#include "adap-widget-utils-private.h"

/**
 * AdapMessageDialog:
 *
 * A dialog presenting a message or a question.
 *
 * <picture>
 *   <source srcset="message-dialog-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="message-dialog.png" alt="message-dialog">
 * </picture>
 *
 * Message dialogs have a heading, a body, an optional child widget, and one or
 * multiple responses, each presented as a button.
 *
 * Each response has a unique string ID, and a button label. Additionally, each
 * response can be enabled or disabled, and can have a suggested or destructive
 * appearance.
 *
 * When one of the responses is activated, or the dialog is closed, the
 * [signal@MessageDialog::response] signal will be emitted. This signal is
 * detailed, and the detail, as well as the `response` parameter will be set to
 * the ID of the activated response, or to the value of the
 * [property@MessageDialog:close-response] property if the dialog had been
 * closed without activating any of the responses.
 *
 * Response buttons can be presented horizontally or vertically depending on
 * available space.
 *
 * When a response is activated, `AdapMessageDialog` is closed automatically.
 *
 * An example of using a message dialog:
 *
 * ```c
 * GtkWidget *dialog;
 *
 * dialog = adap_message_dialog_new (parent, _("Replace File?"), NULL);
 *
 * adap_message_dialog_format_body (ADAP_MESSAGE_DIALOG (dialog),
 *                                 _("A file named “%s” already exists. Do you want to replace it?"),
 *                                 filename);
 *
 * adap_message_dialog_add_responses (ADAP_MESSAGE_DIALOG (dialog),
 *                                   "cancel",  _("_Cancel"),
 *                                   "replace", _("_Replace"),
 *                                   NULL);
 *
 * adap_message_dialog_set_response_appearance (ADAP_MESSAGE_DIALOG (dialog), "replace", ADAP_RESPONSE_DESTRUCTIVE);
 *
 * adap_message_dialog_set_default_response (ADAP_MESSAGE_DIALOG (dialog), "cancel");
 * adap_message_dialog_set_close_response (ADAP_MESSAGE_DIALOG (dialog), "cancel");
 *
 * g_signal_connect (dialog, "response", G_CALLBACK (response_cb), self);
 *
 * gtk_window_present (GTK_WINDOW (dialog));
 * ```
 *
 * ## Async API
 *
 * `AdapMessageDialog` can also be used via the [method@MessageDialog.choose]
 * method. This API follows the GIO async pattern, and the result can be
 * obtained by calling [method@MessageDialog.choose_finish], for example:
 *
 * ```c
 * static void
 * dialog_cb (AdapMessageDialog *dialog,
 *            GAsyncResult     *result,
 *            MyWindow         *self)
 * {
 *   const char *response = adap_message_dialog_choose_finish (dialog, result);
 *
 *   // ...
 * }
 *
 * static void
 * show_dialog (MyWindow *self)
 * {
 *   GtkWidget *dialog;
 *
 *   dialog = adap_message_dialog_new (GTK_WINDOW (self), _("Replace File?"), NULL);
 *
 *   adap_message_dialog_format_body (ADAP_MESSAGE_DIALOG (dialog),
 *                                   _("A file named “%s” already exists. Do you want to replace it?"),
 *                                   filename);
 *
 *   adap_message_dialog_add_responses (ADAP_MESSAGE_DIALOG (dialog),
 *                                     "cancel",  _("_Cancel"),
 *                                     "replace", _("_Replace"),
 *                                     NULL);
 *
 *   adap_message_dialog_set_response_appearance (ADAP_MESSAGE_DIALOG (dialog), "replace", ADAP_RESPONSE_DESTRUCTIVE);
 *
 *   adap_message_dialog_set_default_response (ADAP_MESSAGE_DIALOG (dialog), "cancel");
 *   adap_message_dialog_set_close_response (ADAP_MESSAGE_DIALOG (dialog), "cancel");
 *
 *   adap_message_dialog_choose (ADAP_MESSAGE_DIALOG (dialog), NULL, (GAsyncReadyCallback) dialog_cb, self);
 * }
 * ```
 *
 * ## AdapMessageDialog as GtkBuildable
 *
 * `AdapMessageDialog` supports adding responses in UI definitions by via the
 * `<responses>` element that may contain multiple `<response>` elements, each
 * respresenting a response.
 *
 * Each of the `<response>` elements must have the `id` attribute specifying the
 * response ID. The contents of the element are used as the response label.
 *
 * Response labels can be translated with the usual `translatable`, `context`
 * and `comments` attributes.
 *
 * The `<response>` elements can also have `enabled` and/or `appearance`
 * attributes. See [method@MessageDialog.set_response_enabled] and
 * [method@MessageDialog.set_response_appearance] for details.
 *
 * Example of an `AdapMessageDialog` UI definition:
 *
 * ```xml
 * <object class="AdapMessageDialog" id="dialog">
 *   <property name="heading" translatable="yes">Save Changes?</property>
 *   <property name="body" translatable="yes">Open documents contain unsaved changes. Changes which are not saved will be permanently lost.</property>
 *   <property name="default-response">save</property>
 *   <property name="close-response">cancel</property>
 *   <signal name="response" handler="response_cb"/>
 *   <responses>
 *     <response id="cancel" translatable="yes">_Cancel</response>
 *     <response id="discard" translatable="yes" appearance="destructive">_Discard</response>
 *     <response id="save" translatable="yes" appearance="suggested" enabled="false">_Save</response>
 *   </responses>
 * </object>
 * ```
 *
 * ## Accessibility
 *
 * `AdapMessageDialog` uses the `GTK_ACCESSIBLE_ROLE_DIALOG` role.
 *
 * Since: 1.2
 */

#define DIALOG_MARGIN 30
#define DIALOG_MAX_WIDTH 550
#define DIALOG_MIN_WIDTH 300

typedef struct {
  AdapMessageDialog *dialog;
  GQuark id;
  char *label;
  AdapResponseAppearance appearance;
  gboolean enabled;

  GtkWidget *button;
  GtkWidget *separator;
} ResponseInfo;

typedef struct
{
  GtkWidget *heading_label;
  GtkWidget *body_label;
  GtkBox *message_area;
  GtkWidget *response_area;

  char *heading;
  gboolean heading_use_markup;
  char *body;
  gboolean body_use_markup;
  GtkWidget *child;

  GList *responses;
  GHashTable *id_to_response;
  GQuark default_response;
  GQuark close_response;

  gboolean block_close_response;

  GtkWindow *parent_window;
  int parent_width;
  int parent_height;

  guint parent_state_idle_id;
} AdapMessageDialogPrivate;

static void adap_message_dialog_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdapMessageDialog, adap_message_dialog, GTK_TYPE_WINDOW,
                         G_ADD_PRIVATE (AdapMessageDialog)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adap_message_dialog_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_HEADING,
  PROP_HEADING_USE_MARKUP,
  PROP_BODY,
  PROP_BODY_USE_MARKUP,
  PROP_EXTRA_CHILD,
  PROP_DEFAULT_RESPONSE,
  PROP_CLOSE_RESPONSE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_RESPONSE,
  SIGNAL_LAST_SIGNAL,
};
static guint signals[SIGNAL_LAST_SIGNAL];

static void
response_info_free (ResponseInfo *info)
{
  g_free (info->label);
  g_free (info);
}

static inline ResponseInfo *
find_response (AdapMessageDialog *self,
               const char       *id)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  return g_hash_table_lookup (priv->id_to_response, id);
}

static void
parent_size_cb (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);
  int w = gtk_widget_get_width (GTK_WIDGET (priv->parent_window));
  int h = gtk_widget_get_height (GTK_WIDGET (priv->parent_window));

  if (w == priv->parent_width && h == priv->parent_height)
      return;

  priv->parent_width = w;
  priv->parent_height = h;
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
parent_state_idle_cb (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  parent_size_cb (self);

  priv->parent_state_idle_id = 0;
}

static void
parent_state_cb (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  g_clear_handle_id (&priv->parent_state_idle_id, g_source_remove);

  priv->parent_state_idle_id =
    g_idle_add_once ((GSourceOnceFunc) parent_state_idle_cb, self);
}

static void
parent_realize_cb (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);
  GdkSurface *surface;

  g_assert (GTK_IS_NATIVE (priv->parent_window));

  surface = gtk_native_get_surface (GTK_NATIVE (priv->parent_window));

  g_signal_connect_swapped (surface, "compute-size",
                            G_CALLBACK (parent_size_cb), self);
  g_signal_connect_swapped (surface, "notify::width",
                            G_CALLBACK (parent_size_cb), self);
  g_signal_connect_swapped (surface, "notify::height",
                            G_CALLBACK (parent_size_cb), self);
  g_signal_connect_swapped (surface, "notify::state",
                            G_CALLBACK (parent_state_cb), self);

  parent_size_cb (self);
}

static void
parent_unrealize_cb (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);
  GdkSurface *surface;

  g_assert (GTK_IS_NATIVE (priv->parent_window));

  surface = gtk_native_get_surface (GTK_NATIVE (priv->parent_window));

  g_signal_handlers_disconnect_by_func (surface,
                                        G_CALLBACK (parent_size_cb),
                                        self);
  g_signal_handlers_disconnect_by_func (surface,
                                        G_CALLBACK (parent_state_cb),
                                        self);

  priv->parent_width = -1;
  priv->parent_height = -1;
}

static void
parent_window_notify_cb (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  g_clear_handle_id (&priv->parent_state_idle_id, g_source_remove);

  priv->parent_window = NULL;
  priv->parent_width = -1;
  priv->parent_height = -1;
}

static void
set_parent (AdapMessageDialog *self,
            GtkWindow        *parent)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  if (priv->parent_window == parent)
    return;

  if (priv->parent_window) {
    g_clear_handle_id (&priv->parent_state_idle_id, g_source_remove);

    g_signal_handlers_disconnect_by_func (priv->parent_window,
                                          G_CALLBACK (parent_realize_cb),
                                          self);
    g_signal_handlers_disconnect_by_func (priv->parent_window,
                                          G_CALLBACK (parent_unrealize_cb),
                                          self);

    if (gtk_widget_get_realized (GTK_WIDGET (priv->parent_window)))
      parent_unrealize_cb (self);

    g_object_weak_unref (G_OBJECT (priv->parent_window),
                         (GWeakNotify) parent_window_notify_cb,
                         self);
  }

  priv->parent_window = parent;

  if (priv->parent_window) {
    g_object_weak_ref (G_OBJECT (priv->parent_window),
                       (GWeakNotify) parent_window_notify_cb,
                       self);

    if (gtk_widget_get_realized (GTK_WIDGET (priv->parent_window)))
      parent_realize_cb (self);

    g_signal_connect_swapped (priv->parent_window, "realize",
                              G_CALLBACK (parent_realize_cb), self);
    g_signal_connect_swapped (priv->parent_window, "unrealize",
                              G_CALLBACK (parent_unrealize_cb), self);
  }
}

static void
parent_changed_cb (AdapMessageDialog *self)
{
  GtkWindow *transient_for = gtk_window_get_transient_for (GTK_WINDOW (self));

  set_parent (self, transient_for);
}

static void
button_clicked_cb (ResponseInfo *info)
{
  AdapMessageDialog *self = info->dialog;
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  g_object_ref (self);
  priv->block_close_response = TRUE;

  gtk_window_close (GTK_WINDOW (self));
  g_signal_emit (self, signals[SIGNAL_RESPONSE], info->id, g_quark_to_string (info->id));

  priv->block_close_response = FALSE;
  g_object_unref (self);
}

static GtkWidget *
create_response_button (AdapMessageDialog *self,
                        ResponseInfo     *info)
{
  GtkWidget *button = gtk_button_new_with_mnemonic (info->label);

  gtk_widget_add_css_class (button, "flat");
  gtk_button_set_can_shrink (GTK_BUTTON (button), TRUE);

  switch (info->appearance) {
  case ADAP_RESPONSE_SUGGESTED:
    gtk_widget_add_css_class (button, "suggested");
    break;
  case ADAP_RESPONSE_DESTRUCTIVE:
    gtk_widget_add_css_class (button, "destructive");
    break;
  case ADAP_RESPONSE_DEFAULT:
  default:
    break;
  }

  gtk_widget_set_sensitive (button, info->enabled);

  g_signal_connect_swapped (button, "clicked", G_CALLBACK (button_clicked_cb), info);

  return button;
}

static void
update_window_title (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  if (priv->heading_use_markup) {
    char *heading = NULL;
    GError *error = NULL;

    pango_parse_markup (priv->heading, -1, 0, NULL, &heading, NULL, &error);

    if (error) {
      g_critical ("Couldn't parse markup: %s", error->message);
      g_clear_error (&error);

      heading = g_strdup (priv->heading);
    }

    gtk_window_set_title (GTK_WINDOW (self), heading);

    g_free (heading);
  } else {
    gtk_window_set_title (GTK_WINDOW (self), priv->heading);
  }
}

static gboolean
adap_message_dialog_close_request (GtkWindow *window)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (window);
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  if (!priv->block_close_response)
    g_signal_emit (self, signals[SIGNAL_RESPONSE],
                   priv->close_response,
                   g_quark_to_string (priv->close_response));

  return GTK_WINDOW_CLASS (adap_message_dialog_parent_class)->close_request (window);
}

static void
adap_message_dialog_map (GtkWidget *widget)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (widget);
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);
  GtkWidget *focus, *default_widget;

  if (gtk_window_get_transient_for (GTK_WINDOW (self)) == NULL)
    g_message ("AdapMessageDialog mapped without a transient parent. This is discouraged.");

  GTK_WIDGET_CLASS (adap_message_dialog_parent_class)->map (widget);

  /* The rest of the function was copied from gtkdialog.c */
  focus = gtk_window_get_focus (GTK_WINDOW (self));
  if (!focus) {
    GtkWidget *first_focus = NULL;
    GList *l;

    do {
      g_signal_emit_by_name (self, "move_focus", GTK_DIR_TAB_FORWARD);

      focus = gtk_window_get_focus (GTK_WINDOW (self));
      if (GTK_IS_LABEL (focus) &&
          !gtk_label_get_current_uri (GTK_LABEL (focus)))
        gtk_label_select_region (GTK_LABEL (focus), 0, 0);

      if (first_focus == NULL)
        first_focus = focus;
      else if (first_focus == focus)
        break;

      if (!GTK_IS_LABEL (focus))
        break;
    } while (TRUE);

    default_widget = gtk_window_get_default_widget (GTK_WINDOW (self));
    for (l = priv->responses; l; l = l->next) {
      ResponseInfo *response = l->data;

      if ((focus == NULL || response->button == focus) &&
           response->button != default_widget &&
           default_widget) {
        gtk_widget_grab_focus (default_widget);
        break;
      }
    }
  }
}

static void
measure_responses_do (AdapMessageDialog *self,
                      gboolean          compact,
                      GtkOrientation    orientation,
                      int              *minimum,
                      int              *natural)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);
  GList *l;
  int min = 0, nat = 0;
  int button_min = 0, button_nat = 0;
  int n_buttons = 0;
  gboolean horiz = (orientation == GTK_ORIENTATION_HORIZONTAL);

  for (l = priv->responses; l; l = l->next) {
    ResponseInfo *response = l->data;
    int child_min, child_nat;

    gtk_widget_measure (response->button, orientation, -1,
                        &child_min, &child_nat, NULL, NULL);

    if (horiz == compact) {
      min = MAX (min, child_min);
      nat = MAX (nat, child_nat);
    } else if (horiz) {
      button_min = MAX (button_min, child_min);
      button_nat = MAX (button_nat, child_nat);
      n_buttons++;
    } else {
      min += child_min;
      nat += child_nat;
    }

    if (response->separator) {
      gtk_widget_measure (response->separator, orientation, -1,
                          &child_min, &child_nat, NULL, NULL);

    if (horiz == compact) {
        min = MAX (min, child_min);
        nat = MAX (nat, child_nat);
      } else {
        min += child_min;
        nat += child_nat;
      }
    }
  }

  if (horiz && !compact) {
    min += button_min * n_buttons;
    nat += button_nat * n_buttons;
  }

  if (minimum)
    *minimum = min;
  if (natural)
    *natural = nat;
}

static GtkSizeRequestMode
get_responses_request_mode (GtkWidget *widget)
{
  return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
measure_responses (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (gtk_widget_get_root (widget));

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    measure_responses_do (self, TRUE, orientation, minimum, NULL);
    measure_responses_do (self, FALSE, orientation, NULL, natural);
  } else {
    int wide_nat = 0;

    if (for_size >= 0)
      measure_responses_do (self, FALSE, GTK_ORIENTATION_HORIZONTAL, NULL, &wide_nat);

    measure_responses_do (self, for_size >= 0 && for_size < wide_nat,
                          orientation, minimum, natural);
  }

  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_responses (GtkWidget *widget,
                    int        width,
                    int        height,
                    int        baseline)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (gtk_widget_get_root (widget));
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);
  gboolean compact;
  int wide_nat;

  measure_responses_do (self, FALSE, GTK_ORIENTATION_HORIZONTAL, NULL, &wide_nat);

  compact = wide_nat > width;

  if (compact)
    gtk_widget_add_css_class (widget, "compact");
  else
    gtk_widget_remove_css_class (widget, "compact");

  if (compact) {
    int pos = height;
    GList *l;

    for (l = priv->responses; l; l = l->next) {
      ResponseInfo *response = l->data;
      int child_height;

      if (response->separator) {
        gtk_widget_measure (response->separator, GTK_ORIENTATION_VERTICAL, -1,
                            &child_height, NULL, NULL, NULL);

        pos -= child_height;

        gtk_widget_allocate (response->separator, width, child_height, -1,
                             gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, pos)));
      }

      gtk_widget_measure (response->button, GTK_ORIENTATION_VERTICAL, -1,
                          &child_height, NULL, NULL, NULL);

      pos -= child_height;

      gtk_widget_allocate (response->button, width, child_height, -1,
                           gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, pos)));
    }
  } else {
    gboolean is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
    int pos = is_rtl ? width : 0;
    int n_buttons = g_list_length (priv->responses);
    int total_width = width;
    int button_width;
    GList *l;

    for (l = priv->responses; l; l = l->next) {
      ResponseInfo *response = l->data;
      int separator_width;

      if (!response->separator)
        continue;

      gtk_widget_measure (response->separator, GTK_ORIENTATION_HORIZONTAL, -1,
                          &separator_width, NULL, NULL, NULL);

      total_width -= separator_width;
    }

    button_width = (int) ceil ((double) total_width / n_buttons);

    for (l = priv->responses; l; l = l->next) {
      ResponseInfo *response = l->data;

      if (response->separator) {
        int separator_width;

        gtk_widget_measure (response->separator, GTK_ORIENTATION_HORIZONTAL, -1,
                            &separator_width, NULL, NULL, NULL);

        if (is_rtl)
          pos -= separator_width;

        gtk_widget_allocate (response->separator, separator_width, height, -1,
                             gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (pos, 0)));

        if (!is_rtl)
          pos += separator_width;
      }

      button_width = MIN (button_width, total_width);

      total_width -= button_width;

      if (is_rtl)
        pos -= button_width;

      gtk_widget_allocate (response->button, button_width, height, -1,
                           gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (pos, 0)));

      if (!is_rtl)
        pos += button_width;
    }
  }
}

static void
adap_message_dialog_measure (GtkWidget      *widget,
                            GtkOrientation  orientation,
                            int             for_size,
                            int            *min,
                            int            *nat,
                            int            *min_baseline,
                            int            *nat_baseline)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (widget);
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);
  int max_size, min_size, base_nat;

  GTK_WIDGET_CLASS (adap_message_dialog_parent_class)->measure (widget,
                                                               orientation,
                                                               for_size,
                                                               &min_size,
                                                               &base_nat,
                                                               NULL, NULL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    int wide_nat, narrow_nat;

    min_size = MAX (min_size, DIALOG_MIN_WIDTH);

    if (priv->parent_window) {
      max_size = priv->parent_width - DIALOG_MARGIN * 2;
      max_size = MIN (max_size, DIALOG_MAX_WIDTH);
    } else {
      max_size = DIALOG_MAX_WIDTH;
    }

    measure_responses_do (self, FALSE, GTK_ORIENTATION_HORIZONTAL, NULL, &wide_nat);
    measure_responses_do (self, TRUE, GTK_ORIENTATION_HORIZONTAL, NULL, &narrow_nat);

    narrow_nat = MAX (narrow_nat, DIALOG_MIN_WIDTH);

    if (max_size < wide_nat)
      max_size = MIN (max_size, narrow_nat);
  } else {
    max_size = priv->parent_height - DIALOG_MARGIN * 2;
  }

  max_size = MAX (min_size, max_size);

  if (min)
    *min = min_size;
  if (nat)
    *nat = CLAMP (base_nat, min_size, max_size);
  if (min_baseline)
    *min_baseline = -1;
  if (nat_baseline)
    *nat_baseline = -1;
}

static void
adap_message_dialog_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (object);

  switch (prop_id) {
  case PROP_HEADING:
    g_value_set_string (value, adap_message_dialog_get_heading (self));
    break;
  case PROP_HEADING_USE_MARKUP:
    g_value_set_boolean (value, adap_message_dialog_get_heading_use_markup (self));
    break;
  case PROP_BODY:
    g_value_set_string (value, adap_message_dialog_get_body (self));
    break;
  case PROP_BODY_USE_MARKUP:
    g_value_set_boolean (value, adap_message_dialog_get_body_use_markup (self));
    break;
  case PROP_EXTRA_CHILD:
    g_value_set_object (value, adap_message_dialog_get_extra_child (self));
    break;
  case PROP_DEFAULT_RESPONSE:
    g_value_set_string (value, adap_message_dialog_get_default_response (self));
    break;
  case PROP_CLOSE_RESPONSE:
    g_value_set_string (value, adap_message_dialog_get_close_response (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_message_dialog_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (object);

  switch (prop_id) {
  case PROP_HEADING:
    adap_message_dialog_set_heading (self, g_value_get_string (value));
    break;
  case PROP_HEADING_USE_MARKUP:
    adap_message_dialog_set_heading_use_markup (self, g_value_get_boolean (value));
    break;
  case PROP_BODY:
    adap_message_dialog_set_body (self, g_value_get_string (value));
    break;
  case PROP_BODY_USE_MARKUP:
    adap_message_dialog_set_body_use_markup (self, g_value_get_boolean (value));
    break;
  case PROP_EXTRA_CHILD:
    adap_message_dialog_set_extra_child (self, g_value_get_object (value));
    break;
  case PROP_DEFAULT_RESPONSE:
    adap_message_dialog_set_default_response (self, g_value_get_string (value));
    break;
  case PROP_CLOSE_RESPONSE:
    adap_message_dialog_set_close_response (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_message_dialog_dispose (GObject *object)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (object);
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  set_parent (self, NULL);

  priv->child = NULL;

  if (priv->responses) {
    g_list_free_full (priv->responses, (GDestroyNotify) response_info_free);
    priv->responses = NULL;
  }

  g_clear_pointer (&priv->id_to_response, g_hash_table_unref);

  G_OBJECT_CLASS (adap_message_dialog_parent_class)->dispose (object);
}

static void
adap_message_dialog_finalize (GObject *object)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (object);
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  g_clear_pointer (&priv->heading, g_free);
  g_clear_pointer (&priv->body, g_free);

  G_OBJECT_CLASS (adap_message_dialog_parent_class)->finalize (object);
}

static void
adap_message_dialog_class_init (AdapMessageDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkWindowClass *window_class = GTK_WINDOW_CLASS (klass);

  object_class->get_property = adap_message_dialog_get_property;
  object_class->set_property = adap_message_dialog_set_property;
  object_class->dispose = adap_message_dialog_dispose;
  object_class->finalize = adap_message_dialog_finalize;

  widget_class->map = adap_message_dialog_map;
  widget_class->measure = adap_message_dialog_measure;

  window_class->close_request = adap_message_dialog_close_request;

  /**
   * AdapMessageDialog:heading: (attributes org.gtk.Property.get=adap_message_dialog_get_heading org.gtk.Property.set=adap_message_dialog_set_heading)
   *
   * The heading of the dialog.
   *
   * Since: 1.2
   */
  props[PROP_HEADING] =
    g_param_spec_string ("heading", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapMessageDialog:heading-use-markup: (attributes org.gtk.Property.get=adap_message_dialog_get_heading_use_markup org.gtk.Property.set=adap_message_dialog_set_heading_use_markup)
   *
   * Whether the heading includes Pango markup.
   *
   * See [func@Pango.parse_markup].
   *
   * Since: 1.2
   */
  props[PROP_HEADING_USE_MARKUP] =
    g_param_spec_boolean ("heading-use-markup", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapMessageDialog:body: (attributes org.gtk.Property.get=adap_message_dialog_get_body org.gtk.Property.set=adap_message_dialog_set_body)
   *
   * The body text of the dialog.
   *
   * Since: 1.2
   */
  props[PROP_BODY] =
    g_param_spec_string ("body", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapMessageDialog:body-use-markup: (attributes org.gtk.Property.get=adap_message_dialog_get_body_use_markup org.gtk.Property.set=adap_message_dialog_set_body_use_markup)
   *
   * Whether the body text includes Pango markup.
   *
   * See [func@Pango.parse_markup].
   *
   * Since: 1.2
   */
  props[PROP_BODY_USE_MARKUP] =
    g_param_spec_boolean ("body-use-markup", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapMessageDialog:extra-child: (attributes org.gtk.Property.get=adap_message_dialog_get_extra_child org.gtk.Property.set=adap_message_dialog_set_extra_child)
   *
   * The child widget.
   *
   * Displayed below the heading and body.
   *
   * Since: 1.2
   */
  props[PROP_EXTRA_CHILD] =
    g_param_spec_object ("extra-child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapMessageDialog:default-response: (attributes org.gtk.Property.get=adap_message_dialog_get_default_response org.gtk.Property.set=adap_message_dialog_set_default_response)
   *
   * The response ID of the default response.
   *
   * If set, pressing <kbd>Enter</kbd> will activate the corresponding button.
   *
   * If set to `NULL` or a non-existent response ID, pressing <kbd>Enter</kbd>
   * will do nothing.
   *
   * Since: 1.2
   */
  props[PROP_DEFAULT_RESPONSE] =
    g_param_spec_string ("default-response", NULL, NULL,
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdapMessageDialog:close-response: (attributes org.gtk.Property.get=adap_message_dialog_get_close_response org.gtk.Property.set=adap_message_dialog_set_close_response)
   *
   * The ID of the close response.
   *
   * It will be passed to [signal@MessageDialog::response] if the window is
   * closed by pressing <kbd>Escape</kbd> or with a system action.
   *
   * It doesn't have to correspond to any of the responses in the dialog.
   *
   * The default close response is `close`.
   *
   * Since: 1.2
   */
  props[PROP_CLOSE_RESPONSE] =
    g_param_spec_string ("close-response", NULL, NULL,
                          "close",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdapMessageDialog::response:
   * @self: a message dialog
   * @response: the response ID
   *
   * This signal is emitted when the dialog is closed.
   *
   * @response will be set to the response ID of the button that had been
   * activated.
   *
   * if the dialog was closed by pressing <kbd>Escape</kbd> or with a system
   * action, @response will be set to the value of
   * [property@MessageDialog:close-response].
   *
   * Since: 1.2
   */
  signals[SIGNAL_RESPONSE] =
    g_signal_new ("response",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  G_STRUCT_OFFSET (AdapMessageDialogClass, response),
                  NULL, NULL,
                  adap_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);
  g_signal_set_va_marshaller (signals[SIGNAL_RESPONSE],
                              G_TYPE_FROM_CLASS (klass),
                              adap_marshal_VOID__STRINGv);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adapta/ui/adap-message-dialog.ui");

  gtk_widget_class_bind_template_child_private (widget_class, AdapMessageDialog, heading_label);
  gtk_widget_class_bind_template_child_private (widget_class, AdapMessageDialog, body_label);
  gtk_widget_class_bind_template_child_private (widget_class, AdapMessageDialog, message_area);
  gtk_widget_class_bind_template_child_private (widget_class, AdapMessageDialog, response_area);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_Escape, 0, "window.close", NULL);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_DIALOG);

  g_type_ensure (ADAP_TYPE_GIZMO);
}

static void
adap_message_dialog_init (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv = adap_message_dialog_get_instance_private (self);

  gtk_window_set_resizable (GTK_WINDOW (self), FALSE);
  gtk_window_set_modal (GTK_WINDOW (self), TRUE);
  gtk_window_set_destroy_with_parent (GTK_WINDOW (self), TRUE);

  priv->close_response = g_quark_from_string ("close");

  priv->heading = g_strdup ("");
  priv->body = g_strdup ("");
  priv->parent_width = -1;
  priv->parent_height = -1;
  priv->id_to_response = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_set_layout_manager (priv->response_area,
                                 gtk_custom_layout_new (get_responses_request_mode,
                                                        measure_responses,
                                                        allocate_responses));

  adap_gizmo_set_focus_func (ADAP_GIZMO (priv->response_area),
                            (AdapGizmoFocusFunc) adap_widget_focus_child);
  adap_gizmo_set_grab_focus_func (ADAP_GIZMO (priv->response_area),
                            (AdapGizmoGrabFocusFunc) adap_widget_grab_focus_child);

  parent_changed_cb (self);
  g_signal_connect (self, "notify::transient-for",
                    G_CALLBACK (parent_changed_cb), self);
}

/* Custom tag handling was copied and modified
 * from gtk-size-group.c and gtk-scale.c */

typedef struct {
  GObject *object;
  GtkBuilder *builder;
  GSList *responses;
} ResponseParserData;

typedef struct {
  char *id;

  GString *label;
  char *context;
  gboolean translatable;

  AdapResponseAppearance appearance;
  gboolean enabled;

  int line;
  int col;
} ResponseData;

static void
response_data_free (gpointer data)
{
  ResponseData *response = data;

  g_free (response->id);
  g_string_free (response->label, TRUE);
  g_free (response->context);
  g_free (response);
}

static void
response_start_element (GtkBuildableParseContext  *context,
                        const char                *element_name,
                        const char               **names,
                        const char               **values,
                        gpointer                   user_data,
                        GError                   **error)
{
  ResponseParserData *data = user_data;

  if (strcmp (element_name, "response") == 0) {
    const char *id;
    const char *msg_context = NULL;
    gboolean translatable = FALSE;
    const char *appearance_str = NULL;
    AdapResponseAppearance appearance = ADAP_RESPONSE_DEFAULT;
    gboolean enabled = TRUE;
    ResponseData *response;

    if (!_gtk_builder_check_parent (data->builder, context, "responses", error))
      return;

    if (!g_markup_collect_attributes (element_name, names, values, error,
                                      G_MARKUP_COLLECT_STRING, "id", &id,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "appearance", &appearance_str,
                                      G_MARKUP_COLLECT_TRISTATE | G_MARKUP_COLLECT_OPTIONAL, "enabled", &enabled,
                                      G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "translatable", &translatable,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "comments", NULL,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "context", &msg_context,
                                      G_MARKUP_COLLECT_INVALID)) {
      _gtk_builder_prefix_error (data->builder, context, error);
      return;
    }

    if (appearance_str) {
      GValue gvalue = G_VALUE_INIT;

      if (!gtk_builder_value_from_string_type (data->builder, ADAP_TYPE_RESPONSE_APPEARANCE, appearance_str, &gvalue, error)) {
        _gtk_builder_prefix_error (data->builder, context, error);
        return;
      }

      appearance = g_value_get_enum (&gvalue);
    }

    /* Normalize a tri-state value */
    enabled = enabled != FALSE;

    response = g_new (ResponseData, 1);
    response->id = g_strdup (id);
    response->context = g_strdup (msg_context);
    response->translatable = translatable;
    response->label = g_string_new ("");
    response->appearance = appearance;
    response->enabled = enabled;

    gtk_buildable_parse_context_get_position (context, &response->line, &response->col);
    data->responses = g_slist_prepend (data->responses, response);
  } else if (strcmp (element_name, "responses") == 0) {
    if (!_gtk_builder_check_parent (data->builder, context, "object", error))
      return;

    if (!g_markup_collect_attributes (element_name, names, values, error,
                                      G_MARKUP_COLLECT_INVALID, NULL, NULL,
                                      G_MARKUP_COLLECT_INVALID))
      _gtk_builder_prefix_error (data->builder, context, error);
  } else {
    _gtk_builder_error_unhandled_tag (data->builder, context,
                                      "AdapMessageDialog", element_name,
                                      error);
  }
}

static void
response_text (GtkBuildableParseContext  *context,
               const char                *text,
               gsize                      text_len,
               gpointer                   user_data,
               GError                   **error)
{
  ResponseParserData *data = user_data;

  if (strcmp (gtk_buildable_parse_context_get_element (context), "response") == 0) {
    ResponseData *response = data->responses->data;

    g_string_append_len (response->label, text, text_len);
  }
}

static const GtkBuildableParser response_parser = {
  response_start_element,
  NULL,
  response_text,
  NULL
};

static gboolean
adap_message_dialog_buildable_custom_tag_start (GtkBuildable       *buildable,
                                               GtkBuilder         *builder,
                                               GObject            *child,
                                               const char         *tagname,
                                               GtkBuildableParser *parser,
                                               gpointer           *parser_data)
{
  ResponseParserData *data;

  if (child)
    return FALSE;

  if (strcmp (tagname, "responses") == 0) {
    data = g_new0 (ResponseParserData, 1);
    data->responses = NULL;
    data->object = G_OBJECT (buildable);
    data->builder = builder;

    *parser = response_parser;
    *parser_data = data;

    return TRUE;
  }

  return parent_buildable_iface->custom_tag_start (buildable, builder, child,
                                                   tagname, parser, parser_data);
}

static void
adap_message_dialog_buildable_custom_finished (GtkBuildable *buildable,
                                              GtkBuilder   *builder,
                                              GObject      *child,
                                              const char   *tagname,
                                              gpointer      user_data)
{
  GSList *l;
  ResponseParserData *data;

  if (strcmp (tagname, "responses") != 0) {
    parent_buildable_iface->custom_finished (buildable, builder, child,
                                             tagname, user_data);
    return;
  }

  data = (ResponseParserData*)user_data;
  data->responses = g_slist_reverse (data->responses);

  for (l = data->responses; l; l = l->next) {
    ResponseData *response = l->data;
    const char *label;

    if (response->translatable && response->label->len)
      label = _gtk_builder_parser_translate (gtk_builder_get_translation_domain (builder),
                                             response->context,
                                             response->label->str);
    else
      label = response->label->str;

    adap_message_dialog_add_response (ADAP_MESSAGE_DIALOG (data->object),
                                     response->id, label);

    if (response->appearance != ADAP_RESPONSE_DEFAULT)
      adap_message_dialog_set_response_appearance (ADAP_MESSAGE_DIALOG (data->object),
                                                  response->id, response->appearance);

    if (!response->enabled)
      adap_message_dialog_set_response_enabled (ADAP_MESSAGE_DIALOG (data->object),
                                               response->id, FALSE);
  }

  g_slist_free_full (data->responses, response_data_free);
  g_free (data);
}

static void
adap_message_dialog_buildable_add_child (GtkBuildable *buildable,
                                        GtkBuilder   *builder,
                                        GObject      *child,
                                        const char   *type)
{
  AdapMessageDialog *self = ADAP_MESSAGE_DIALOG (buildable);

  if (GTK_IS_WIDGET (child))
    adap_message_dialog_set_extra_child (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adap_message_dialog_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adap_message_dialog_buildable_add_child;
  iface->custom_tag_start = adap_message_dialog_buildable_custom_tag_start;
  iface->custom_finished = adap_message_dialog_buildable_custom_finished;
}

/**
 * adap_message_dialog_new:
 * @parent: (nullable): transient parent
 * @heading: (nullable): the heading
 * @body: (nullable): the body text
 *
 * Creates a new `AdapMessageDialog`.
 *
 * @heading and @body can be set to `NULL`. This can be useful if they need to
 * be formatted or use markup. In that case, set them to `NULL` and call
 * [method@MessageDialog.format_body] or similar methods afterwards:
 *
 * ```c
 * GtkWidget *dialog;
 *
 * dialog = adap_message_dialog_new (parent, _("Replace File?"), NULL);
 * adap_message_dialog_format_body (ADAP_MESSAGE_DIALOG (dialog),
 *                                 _("A file named “%s” already exists.  Do you want to replace it?"),
 *                                 filename);
 * ```
 *
 * Returns: the newly created `AdapMessageDialog`
 *
 * Since: 1.2
 */
GtkWidget *
adap_message_dialog_new (GtkWindow  *parent,
                        const char *heading,
                        const char *body)
{
  GtkWidget *dialog;

  g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);

  dialog = g_object_new (ADAP_TYPE_MESSAGE_DIALOG,
                         "transient-for", parent,
                         NULL);

  if (heading)
    adap_message_dialog_set_heading (ADAP_MESSAGE_DIALOG (dialog), heading);

  if (body)
    adap_message_dialog_set_body (ADAP_MESSAGE_DIALOG (dialog), body);

  return dialog;
}

/**
 * adap_message_dialog_get_heading: (attributes org.gtk.Method.get_property=heading)
 * @self: a message dialog
 *
 * Gets the heading of @self.
 *
 * Returns: (nullable): the heading of @self.
 *
 * Since: 1.2
 */
const char *
adap_message_dialog_get_heading (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), NULL);

  priv = adap_message_dialog_get_instance_private (self);

  return priv->heading;
}

/**
 * adap_message_dialog_set_heading: (attributes org.gtk.Method.set_property=heading)
 * @self: a message dialog
 * @heading: (nullable): the heading of @self
 *
 * Sets the heading of @self.
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_heading (AdapMessageDialog *self,
                                const char       *heading)
{
  AdapMessageDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (heading != NULL);

  priv = adap_message_dialog_get_instance_private (self);

  if (!g_set_str (&priv->heading, heading))
    return;

  gtk_label_set_label (GTK_LABEL (priv->heading_label), heading);
  gtk_widget_set_visible (priv->heading_label, heading && *heading);

  if (heading && *heading)
    gtk_widget_add_css_class (GTK_WIDGET (priv->message_area), "has-heading");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (priv->message_area), "has-heading");

  update_window_title (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HEADING]);
}

/**
 * adap_message_dialog_get_heading_use_markup: (attributes org.gtk.Method.get_property=heading-use-markup)
 * @self: a message dialog
 *
 * Gets whether the heading of @self includes Pango markup.
 *
 * Returns: whether @self uses markup for heading
 *
 * Since: 1.2
 */
gboolean
adap_message_dialog_get_heading_use_markup (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), FALSE);

  priv = adap_message_dialog_get_instance_private (self);

  return priv->heading_use_markup;
}

/**
 * adap_message_dialog_set_heading_use_markup: (attributes org.gtk.Method.set_property=heading-use-markup)
 * @self: a message dialog
 * @use_markup: whether to use markup for heading
 *
 * Sets whether the heading of @self includes Pango markup.
 *
 * See [func@Pango.parse_markup].
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_heading_use_markup (AdapMessageDialog *self,
                                           gboolean          use_markup)
{
  AdapMessageDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));

  priv = adap_message_dialog_get_instance_private (self);

  use_markup = !!use_markup;

  if (use_markup == priv->heading_use_markup)
    return;

  priv->heading_use_markup = use_markup;

  gtk_label_set_use_markup (GTK_LABEL (priv->heading_label), use_markup);

  update_window_title (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HEADING_USE_MARKUP]);
}


/**
 * adap_message_dialog_format_heading:
 * @self: a message dialog
 * @format: the formatted string for the heading
 * @...: the parameters to insert into @format
 *
 * Sets the formatted heading of @self.
 *
 * See [property@MessageDialog:heading].
 *
 * Since: 1.2
 */
void
adap_message_dialog_format_heading (AdapMessageDialog *self,
                                   const char       *format,
                                   ...)
{
  va_list args;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (format != NULL);

  g_object_freeze_notify (G_OBJECT (self));

  adap_message_dialog_set_heading_use_markup (self, FALSE);

  if (format) {
    char *heading;

    va_start (args, format);
    heading = g_strdup_vprintf (format, args);
    va_end (args);

    adap_message_dialog_set_heading (self, heading);

    g_free (heading);
  } else {
    adap_message_dialog_set_heading (self, NULL);
  }

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_message_dialog_format_heading_markup:
 * @self: a message dialog
 * @format: the formatted string for the heading with Pango markup
 * @...: the parameters to insert into @format
 *
 * Sets the formatted heading of @self with Pango markup.
 *
 * The @format is assumed to contain Pango markup.
 *
 * Special XML characters in the `printf()` arguments passed to this function
 * will automatically be escaped as necessary, see
 * [func@GLib.markup_printf_escaped].
 *
 * See [property@MessageDialog:heading].
 *
 * Since: 1.2
 */
void
adap_message_dialog_format_heading_markup (AdapMessageDialog *self,
                                          const char       *format,
                                          ...)
{
  va_list args;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (format != NULL);

  g_object_freeze_notify (G_OBJECT (self));

  adap_message_dialog_set_heading_use_markup (self, TRUE);

  if (format) {
    char *heading;

    va_start (args, format);
    heading = g_markup_vprintf_escaped (format, args);
    va_end (args);

    adap_message_dialog_set_heading (self, heading);

    g_free (heading);
  } else {
    adap_message_dialog_set_heading (self, "");
  }

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_message_dialog_get_body: (attributes org.gtk.Method.get_property=body)
 * @self: a message dialog
 *
 * Gets the body text of @self.
 *
 * Returns: the body of @self.
 *
 * Since: 1.2
 */
const char *
adap_message_dialog_get_body (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), NULL);

  priv = adap_message_dialog_get_instance_private (self);

  return priv->body;
}

/**
 * adap_message_dialog_set_body: (attributes org.gtk.Method.set_property=body)
 * @self: a message dialog
 * @body: the body of @self
 *
 * Sets the body text of @self.
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_body (AdapMessageDialog *self,
                             const char       *body)
{
  AdapMessageDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (body != NULL);

  priv = adap_message_dialog_get_instance_private (self);

  if (!g_set_str (&priv->body, body))
    return;

  gtk_label_set_label (GTK_LABEL (priv->body_label), body);
  gtk_widget_set_visible (priv->body_label, body && *body);

  if (body && *body)
    gtk_widget_add_css_class (GTK_WIDGET (priv->message_area), "has-body");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (priv->message_area), "has-body");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BODY]);
}

/**
 * adap_message_dialog_get_body_use_markup: (attributes org.gtk.Method.get_property=body-use-markup)
 * @self: a message dialog
 *
 * Gets whether the body text of @self includes Pango markup.
 *
 * Returns: whether @self uses markup for body text
 *
 * Since: 1.2
 */
gboolean
adap_message_dialog_get_body_use_markup (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), FALSE);

  priv = adap_message_dialog_get_instance_private (self);

  return priv->body_use_markup;
}

/**
 * adap_message_dialog_set_body_use_markup: (attributes org.gtk.Method.set_property=body-use-markup)
 * @self: a message dialog
 * @use_markup: whether to use markup for body text
 *
 * Sets whether the body text of @self includes Pango markup.
 *
 * See [func@Pango.parse_markup].
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_body_use_markup (AdapMessageDialog *self,
                                        gboolean          use_markup)
{
  AdapMessageDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));

  priv = adap_message_dialog_get_instance_private (self);

  use_markup = !!use_markup;

  if (use_markup == priv->body_use_markup)
    return;

  priv->body_use_markup = use_markup;

  gtk_label_set_use_markup (GTK_LABEL (priv->body_label), use_markup);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BODY_USE_MARKUP]);
}

/**
 * adap_message_dialog_format_body:
 * @self: a message dialog
 * @format: the formatted string for the body text
 * @...: the parameters to insert into @format
 *
 * Sets the formatted body text of @self.
 *
 * See [property@MessageDialog:body].
 *
 * Since: 1.2
 */
void
adap_message_dialog_format_body (AdapMessageDialog *self,
                                const char       *format,
                                ...)
{
  va_list args;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (format != NULL);

  g_object_freeze_notify (G_OBJECT (self));

  adap_message_dialog_set_body_use_markup (self, FALSE);

  if (format) {
    char *body;

    va_start (args, format);
    body = g_strdup_vprintf (format, args);
    va_end (args);

    adap_message_dialog_set_body (self, body);

    g_free (body);
  } else {
    adap_message_dialog_set_body (self, "");
  }

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_message_dialog_format_body_markup:
 * @self: a message dialog
 * @format: the formatted string for the body text with Pango markup
 * @...: the parameters to insert into @format
 *
 * Sets the formatted body text of @self with Pango markup.
 *
 * The @format is assumed to contain Pango markup.
 *
 * Special XML characters in the `printf()` arguments passed to this function
 * will  automatically be escaped as necessary, see
 * [func@GLib.markup_printf_escaped].
 *
 * See [property@MessageDialog:body].
 *
 * Since: 1.2
 */
void
adap_message_dialog_format_body_markup (AdapMessageDialog *self,
                                       const char       *format,
                                       ...)
{
  va_list args;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (format != NULL);

  g_object_freeze_notify (G_OBJECT (self));

  adap_message_dialog_set_body_use_markup (self, TRUE);

  if (format) {
    char *body;

    va_start (args, format);
    body = g_markup_vprintf_escaped (format, args);
    va_end (args);

    adap_message_dialog_set_body (self, body);

    g_free (body);
  } else {
    adap_message_dialog_set_body (self, NULL);
  }

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adap_message_dialog_get_extra_child: (attributes org.gtk.Method.get_property=extra-child)
 * @self: a message dialog
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self.
 *
 * Since: 1.2
 */
GtkWidget *
adap_message_dialog_get_extra_child (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), NULL);

  priv = adap_message_dialog_get_instance_private (self);

  return priv->child;
}

/**
 * adap_message_dialog_set_extra_child: (attributes org.gtk.Method.set_property=extra-child)
 * @self: a message dialog
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * The child widget is displayed below the heading and body.
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_extra_child (AdapMessageDialog *self,
                                    GtkWidget        *child)
{
  AdapMessageDialogPrivate *priv;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = adap_message_dialog_get_instance_private (self);

  if (child == priv->child)
    return;

  if (priv->child)
    gtk_box_remove (priv->message_area, priv->child);

  priv->child = child;

  if (priv->child)
    gtk_box_append (priv->message_area, priv->child);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTRA_CHILD]);
}

/**
 * adap_message_dialog_add_response:
 * @self: a message dialog
 * @id: the response ID
 * @label: the response label
 *
 * Adds a response with @id and @label to @self.
 *
 * Responses are represented as buttons in the dialog.
 *
 * Response ID must be unique. It will be used in
 * [signal@MessageDialog::response] to tell which response had been activated,
 * as well as to inspect and modify the response later.
 *
 * An embedded underline in @label indicates a mnemonic.
 *
 * [method@MessageDialog.set_response_label] can be used to change the response
 * label after it had been added.
 *
 * [method@MessageDialog.set_response_enabled] and
 * [method@MessageDialog.set_response_appearance] can be used to customize the
 * responses further.
 *
 * Since: 1.2
 */
void
adap_message_dialog_add_response (AdapMessageDialog *self,
                                 const char       *id,
                                 const char       *label)
{
  AdapMessageDialogPrivate *priv;
  ResponseInfo *info;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (id != NULL);
  g_return_if_fail (label != NULL);

  priv = adap_message_dialog_get_instance_private (self);

  if (find_response (self, id)) {
    g_critical ("Trying to add a response with id '%s' to an "
                "AdapMessageDialog, but such a response already exists", id);
    return;
  }

  info = g_new0 (ResponseInfo, 1);

  info->dialog = self;
  info->id = g_quark_from_string (id);
  info->label = g_strdup (label);
  info->appearance = ADAP_RESPONSE_DEFAULT;
  info->enabled = TRUE;

  if (priv->responses) {
    info->separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_parent (info->separator, priv->response_area);
  }

  info->button = create_response_button (self, info);
  gtk_widget_set_parent (info->button, priv->response_area);

  priv->responses = g_list_append (priv->responses, info);
  g_hash_table_insert (priv->id_to_response, g_strdup (id), info);

  if (priv->default_response == info->id)
    gtk_window_set_default_widget (GTK_WINDOW (self), info->button);
}

/**
 * adap_message_dialog_add_responses: (skip)
 * @self: a message dialog
 * @first_id: response id
 * @...: label for first response, then more id-label pairs
 *
 * Adds multiple responses to @self.
 *
 * This is the same as calling [method@MessageDialog.add_response] repeatedly.
 * The variable argument list should be `NULL`-terminated list of response IDs
 * and labels.
 *
 * Example:
 *
 * ```c
 * adap_message_dialog_add_responses (dialog,
 *                                   "cancel",  _("_Cancel"),
 *                                   "discard", _("_Discard"),
 *                                   "save",    _("_Save"),
 *                                   NULL);
 * ```
 *
 * Since: 1.2
 */
void
adap_message_dialog_add_responses (AdapMessageDialog *self,
                                  const char       *first_id,
                                  ...)
{
  va_list args;
  const char *id, *label;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));

  if (!first_id)
    return;

  va_start (args, first_id);

  id = first_id;
  label = va_arg (args, const char *);

  while (id) {
    adap_message_dialog_add_response (self, id, label);

    id = va_arg (args, const char *);
    if (!id)
      break;

    label = va_arg (args, const char *);
  }

  va_end (args);
}

/**
 * adap_message_dialog_remove_response:
 * @self: a message dialog
 * @id: the response ID
 *
 * Removes a response from @self.
 *
 * Since: 1.5
 */
void
adap_message_dialog_remove_response (AdapMessageDialog *self,
                                    const char       *id)
{
  AdapMessageDialogPrivate *priv;
  ResponseInfo *info;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (id != NULL);

  priv = adap_message_dialog_get_instance_private (self);
  info = find_response (self, id);

  if (!info) {
    g_critical ("Trying to remove a response with id '%s' from an "
                "AdapMessageDialog, but such a response does not exist",
                id);
    return;
  }

  if (priv->default_response == info->id)
    gtk_window_set_default_widget (GTK_WINDOW (self), NULL);

  gtk_widget_unparent (info->button);

  if (info == priv->responses->data && priv->responses->next) {
    ResponseInfo *next_info = priv->responses->next->data;
    g_clear_pointer (&next_info->separator, gtk_widget_unparent);
  } else {
    g_clear_pointer (&info->separator, gtk_widget_unparent);
  }

  priv->responses = g_list_remove (priv->responses, info);
  g_hash_table_remove (priv->id_to_response, id);

  response_info_free (info);
}

/**
 * adap_message_dialog_get_response_label:
 * @self: a message dialog
 * @response: a response ID
 *
 * Gets the label of @response.
 *
 * See [method@MessageDialog.set_response_label].
 *
 * Returns: the label of @response
 *
 * Since: 1.2
 */
const char *
adap_message_dialog_get_response_label (AdapMessageDialog *self,
                                       const char       *response)
{
  ResponseInfo *info;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), NULL);
  g_return_val_if_fail (response != NULL, NULL);
  g_return_val_if_fail (adap_message_dialog_has_response (self, response), NULL);

  info = find_response (self, response);

  return info->label;
}

/**
 * adap_message_dialog_set_response_label:
 * @self: a message dialog
 * @response: a response ID
 * @label: the label of @response
 *
 * Sets the label of @response to @label.
 *
 * Labels are displayed on the dialog buttons. An embedded underline in @label
 * indicates a mnemonic.
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_response_label (AdapMessageDialog *self,
                                       const char       *response,
                                       const char       *label)
{
  ResponseInfo *info;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (response != NULL);
  g_return_if_fail (label != NULL);
  g_return_if_fail (adap_message_dialog_has_response (self, response));

  info = find_response (self, response);

  g_set_str (&info->label, label);

  gtk_button_set_label (GTK_BUTTON (info->button), label);
}

/**
 * adap_message_dialog_get_response_appearance:
 * @self: a message dialog
 * @response: a response ID
 *
 * Gets the appearance of @response.
 *
 * See [method@MessageDialog.set_response_appearance].
 *
 * Returns: the appearance of @response
 *
 * Since: 1.2
 */
AdapResponseAppearance
adap_message_dialog_get_response_appearance (AdapMessageDialog *self,
                                            const char       *response)
{
  ResponseInfo *info;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), FALSE);
  g_return_val_if_fail (response != NULL, FALSE);
  g_return_val_if_fail (adap_message_dialog_has_response (self, response), FALSE);

  info = find_response (self, response);

  return info->appearance;
}

/**
 * adap_message_dialog_set_response_appearance:
 * @self: a message dialog
 * @response: a response ID
 * @appearance: appearance for @response
 *
 * Sets the appearance for @response.
 *
 * <picture>
 *   <source srcset="message-dialog-appearance-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="message-dialog-appearance.png" alt="message-dialog-appearance">
 * </picture>
 *
 * Use `ADAP_RESPONSE_SUGGESTED` to mark important responses such as the
 * affirmative action, like the Save button in the example.
 *
 * Use `ADAP_RESPONSE_DESTRUCTIVE` to draw attention to the potentially damaging
 * consequences of using @response. This appearance acts as a warning to the
 * user. The Discard button in the example is using this appearance.
 *
 * The default appearance is `ADAP_RESPONSE_DEFAULT`.
 *
 * Negative responses like Cancel or Close should use the default appearance.
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_response_appearance (AdapMessageDialog      *self,
                                            const char            *response,
                                            AdapResponseAppearance  appearance)
{
  ResponseInfo *info;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (response != NULL);
  g_return_if_fail (appearance >= ADAP_RESPONSE_DEFAULT &&
                    appearance <= ADAP_RESPONSE_DESTRUCTIVE);
  g_return_if_fail (adap_message_dialog_has_response (self, response));

  info = find_response (self, response);

  if (appearance == info->appearance)
    return;

  info->appearance = appearance;

  if (info->appearance == ADAP_RESPONSE_SUGGESTED)
    gtk_widget_add_css_class (info->button, "suggested");
  else
    gtk_widget_remove_css_class (info->button, "suggested");

  if (info->appearance == ADAP_RESPONSE_DESTRUCTIVE)
    gtk_widget_add_css_class (info->button, "destructive");
  else
    gtk_widget_remove_css_class (info->button, "destructive");
}

/**
 * adap_message_dialog_get_response_enabled:
 * @self: a message dialog
 * @response: a response ID
 *
 * Gets whether @response is enabled.
 *
 * See [method@MessageDialog.set_response_enabled].
 *
 * Returns: whether @response is enabled
 *
 * Since: 1.2
 */
gboolean
adap_message_dialog_get_response_enabled (AdapMessageDialog *self,
                                         const char       *response)
{
  ResponseInfo *info;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), FALSE);
  g_return_val_if_fail (response != NULL, FALSE);
  g_return_val_if_fail (adap_message_dialog_has_response (self, response), FALSE);

  info = find_response (self, response);

  return info->enabled;
}

/**
 * adap_message_dialog_set_response_enabled:
 * @self: a message dialog
 * @response: a response ID
 * @enabled: whether to enable @response
 *
 * Sets whether @response is enabled.
 *
 * If @response is not enabled, the corresponding button will have
 * [property@Gtk.Widget:sensitive] set to `FALSE` and it can't be activated as
 * a default response.
 *
 * @response can still be used as [property@MessageDialog:close-response] while
 * it's not enabled.
 *
 * Responses are enabled by default.
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_response_enabled (AdapMessageDialog *self,
                                         const char       *response,
                                         gboolean          enabled)
{
  ResponseInfo *info;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (response != NULL);
  g_return_if_fail (adap_message_dialog_has_response (self, response));

  info = find_response (self, response);

  enabled = !!enabled;

  if (enabled == info->enabled)
    return;

  info->enabled = enabled;

  gtk_widget_set_sensitive (info->button, info->enabled);
}

/**
 * adap_message_dialog_get_default_response: (attributes org.gtk.Method.get_property=default-response)
 * @self: a message dialog
 *
 * Gets the ID of the default response of @self.
 *
 * Returns: (nullable): the default response ID
 *
 * Since: 1.2
 */
const char *
adap_message_dialog_get_default_response (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), NULL);

  priv = adap_message_dialog_get_instance_private (self);

  if (!priv->default_response)
    return NULL;

  return g_quark_to_string (priv->default_response);
}

/**
 * adap_message_dialog_set_default_response: (attributes org.gtk.Method.set_property=default-response)
 * @self: a message dialog
 * @response: (nullable): the default response ID
 *
 * Sets the ID of the default response of @self.
 *
 * If set, pressing <kbd>Enter</kbd> will activate the corresponding button.
 *
 * If set to `NULL` or to a non-existent response ID, pressing <kbd>Enter</kbd>
 * will do nothing.
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_default_response (AdapMessageDialog *self,
                                         const char       *response)
{
  AdapMessageDialogPrivate *priv;
  GQuark quark;
  ResponseInfo *info;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));

  priv = adap_message_dialog_get_instance_private (self);
  quark = g_quark_from_string (response);

  if (quark == priv->default_response)
    return;

  priv->default_response = quark;

  info = find_response (self, response);

  if (info)
    gtk_window_set_default_widget (GTK_WINDOW (self), info->button);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEFAULT_RESPONSE]);
}

/**
 * adap_message_dialog_get_close_response: (attributes org.gtk.Method.get_property=close-response)
 * @self: a message dialog
 *
 * Gets the ID of the close response of @self.
 *
 * Returns: the close response ID
 *
 * Since: 1.2
 */
const char *
adap_message_dialog_get_close_response (AdapMessageDialog *self)
{
  AdapMessageDialogPrivate *priv;

  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), NULL);

  priv = adap_message_dialog_get_instance_private (self);

  return g_quark_to_string (priv->close_response);
}

/**
 * adap_message_dialog_set_close_response: (attributes org.gtk.Method.set_property=close-response)
 * @self: a message dialog
 * @response: the close response ID
 *
 * Sets the ID of the close response of @self.
 *
 * It will be passed to [signal@MessageDialog::response] if the window is
 * closed by pressing <kbd>Escape</kbd> or with a system action.
 *
 * It doesn't have to correspond to any of the responses in the dialog.
 *
 * The default close response is `close`.
 *
 * Since: 1.2
 */
void
adap_message_dialog_set_close_response (AdapMessageDialog *self,
                                       const char       *response)
{
  AdapMessageDialogPrivate *priv;
  GQuark quark;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (response != NULL);

  priv = adap_message_dialog_get_instance_private (self);
  quark = g_quark_from_string (response);

  if (quark == priv->close_response)
    return;

  priv->close_response = quark;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CLOSE_RESPONSE]);
}

/**
 * adap_message_dialog_response: (attributes org.gtk.Method.signal=response)
 * @self: a message dialog
 * @response: response ID
 *
 * Emits the [signal@MessageDialog::response] signal with the given response ID.
 *
 * Used to indicate that the user has responded to the dialog in some way.
 *
 * Since: 1.2
 */
void
adap_message_dialog_response (AdapMessageDialog *self,
                             const char       *response)
{
  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));
  g_return_if_fail (response != NULL);

  g_signal_emit (self, signals[SIGNAL_RESPONSE],
                 g_quark_from_string (response), response);
}

/**
 * adap_message_dialog_has_response:
 * @self: a message dialog
 * @response: response ID
 *
 * Gets whether @self has a response with the ID @response.
 *
 * Returns: whether @self has a response with the ID @response.
 *
 * Since: 1.2
 */
gboolean
adap_message_dialog_has_response (AdapMessageDialog *self,
                                 const char       *response)
{
  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), FALSE);
  g_return_val_if_fail (response != NULL, FALSE);

  return find_response (self, response) != NULL;
}

static void choose_cancelled_cb (GCancellable *cancellable,
                                 GTask        *task);

static void
choose_response_cb (AdapMessageDialog *dialog,
                    const char       *response,
                    GTask            *task)
{
  GCancellable *cancellable = g_task_get_cancellable (task);

  if (cancellable)
    g_signal_handlers_disconnect_by_func (cancellable, choose_cancelled_cb, task);

  g_signal_handlers_disconnect_by_func (dialog, choose_response_cb, task);

  g_task_return_int (task, g_quark_from_string (response));

  g_object_unref (task);
}

static void
choose_cancelled_cb (GCancellable *cancellable,
                     GTask        *task)
{
  AdapMessageDialog *self = g_task_get_source_object (task);

  choose_response_cb (self, adap_message_dialog_get_close_response (self), task);
}

/**
 * adap_message_dialog_choose:
 * @self: a message dialog
 * @cancellable: (nullable): a `GCancellable` to cancel the operation
 * @callback: (scope async): a callback to call when the operation is complete
 * @user_data: data to pass to @callback
 *
 * This function shows @self to the user.
 *
 * The @callback will be called when the alert is dismissed. It should call
 * [method@MessageDialog.choose_finish] to obtain the result.
 *
 * Since: 1.3
 */
void
adap_message_dialog_choose (AdapMessageDialog    *self,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             user_data)
{
  GTask *task;

  g_return_if_fail (ADAP_IS_MESSAGE_DIALOG (self));

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, adap_message_dialog_choose);

  if (cancellable)
    g_signal_connect (cancellable, "cancelled", G_CALLBACK (choose_cancelled_cb), task);

  g_signal_connect (self, "response", G_CALLBACK (choose_response_cb), task);

  gtk_window_present (GTK_WINDOW (self));
}

/**
 * adap_message_dialog_choose_finish:
 * @self: a message dialog
 * @result: a `GAsyncResult`
 *
 * Finishes the [method@MessageDialog.choose] call and returns the response ID.
 *
 * Returns: the ID of the response that was selected, or
 *   [property@MessageDialog:close-response] if the call was cancelled.
 *
 * Since: 1.3
 */
const char *
adap_message_dialog_choose_finish (AdapMessageDialog *self,
                                  GAsyncResult     *result)
{
  GQuark id;
  g_return_val_if_fail (ADAP_IS_MESSAGE_DIALOG (self), NULL);
  g_return_val_if_fail (g_task_is_valid (result, self), NULL);
  g_return_val_if_fail (g_task_get_source_tag (G_TASK (result)) == adap_message_dialog_choose, NULL);

  id = g_task_propagate_int (G_TASK (result), NULL);

  return g_quark_to_string (id);
}
