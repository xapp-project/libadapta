#include "adap-demo-page-avatar.h"

#include <glib/gi18n.h>

struct _AdapDemoPageAvatar
{
  AdapBin parent_instance;

  AdapAvatar *avatar;
  AdapEntryRow *text;
  GtkLabel *file_chooser_label;
  GtkListBox *contacts;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageAvatar, adap_demo_page_avatar, ADAP_TYPE_BIN)

static char *
create_random_name (void)
{
  static const char *first_names[] = {
    "Adam",
    "Adrian",
    "Anna",
    "Charlotte",
    "Frédérique",
    "Ilaria",
    "Jakub",
    "Jennyfer",
    "Julia",
    "Justin",
    "Mario",
    "Miriam",
    "Mohamed",
    "Nourimane",
    "Owen",
    "Peter",
    "Petra",
    "Rachid",
    "Rebecca",
    "Sarah",
    "Thibault",
    "Wolfgang",
  };
  static const char *last_names[] = {
    "Bailey",
    "Berat",
    "Chen",
    "Farquharson",
    "Ferber",
    "Franco",
    "Galinier",
    "Han",
    "Lawrence",
    "Lepied",
    "Lopez",
    "Mariotti",
    "Rossi",
    "Urasawa",
    "Zwickelman",
  };

  return g_strdup_printf ("%s %s",
                          first_names[g_random_int_range (0, G_N_ELEMENTS (first_names))],
                          last_names[g_random_int_range (0, G_N_ELEMENTS (last_names))]);
}

static void
populate_contacts (AdapDemoPageAvatar *self)
{
  for (int i = 0; i < 30; i++) {
    char *name = create_random_name ();
    GtkWidget *contact = adap_action_row_new ();
    GtkWidget *avatar = adap_avatar_new (40, name, TRUE);

    gtk_widget_set_margin_top (avatar, 12);
    gtk_widget_set_margin_bottom (avatar, 12);

    adap_preferences_row_set_title (ADAP_PREFERENCES_ROW (contact), name);
    adap_action_row_add_prefix (ADAP_ACTION_ROW (contact), avatar);
    gtk_list_box_append (self->contacts, contact);

    g_free (name);
  }
}

static void
avatar_open_dialog_cb (GtkFileDialog     *dialog,
                       GAsyncResult      *result,
                       AdapDemoPageAvatar *self)
{
  GFile *file = gtk_file_dialog_open_finish (dialog, result, NULL);

  if (file) {
    GFileInfo *info;
    GdkTexture *texture;
    GError *error = NULL;

    info = g_file_query_info (file,
                              G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                              G_FILE_QUERY_INFO_NONE,
                              NULL,
                              NULL);

    if (info)
      gtk_label_set_label (self->file_chooser_label,
                           g_file_info_get_display_name (info));

    gtk_widget_action_set_enabled (GTK_WIDGET (self), "avatar.remove", TRUE);

    texture = gdk_texture_new_from_file (file, &error);
    if (error) {
      g_critical ("Failed to create texture from file: %s", error->message);
      g_clear_error (&error);
    }

    adap_avatar_set_custom_image (self->avatar, texture ? GDK_PAINTABLE (texture) : NULL);

    g_clear_object (&info);
    g_clear_object (&texture);
    g_object_unref (file);
  }
}

static void
avatar_open_cb (AdapDemoPageAvatar *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  GtkFileDialog *dialog = gtk_file_dialog_new ();

  gtk_file_dialog_set_title (dialog, _("Select an Avatar"));

  gtk_file_dialog_open (dialog,
                        GTK_WINDOW (root),
                        NULL,
                        (GAsyncReadyCallback) avatar_open_dialog_cb,
                        self);
}

static void
avatar_remove_cb (AdapDemoPageAvatar *self)
{
  gtk_label_set_label (self->file_chooser_label, _("(None)"));
  gtk_widget_action_set_enabled (GTK_WIDGET (self), "avatar.remove", FALSE);
  adap_avatar_set_custom_image (self->avatar, NULL);
}

static void
avatar_save_dialog_cb (GtkFileDialog     *dialog,
                       GAsyncResult      *result,
                       AdapDemoPageAvatar *self)
{
  GFile *file = gtk_file_dialog_save_finish (dialog, result, NULL);

  if (file) {
    GdkTexture *texture =
      adap_avatar_draw_to_texture (self->avatar,
                                  gtk_widget_get_scale_factor (GTK_WIDGET (self)));

    gdk_texture_save_to_png (texture, g_file_peek_path (file));

    g_object_unref (texture);
    g_object_unref (file);
  }
}

static void
avatar_save_cb (AdapDemoPageAvatar *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  GtkFileDialog *dialog = gtk_file_dialog_new ();

  gtk_file_dialog_set_title (dialog, _("Save Avatar"));

  gtk_file_dialog_save (dialog,
                        GTK_WINDOW (root),
                        NULL,
                        (GAsyncReadyCallback) avatar_save_dialog_cb,
                        self);
}

static void
adap_demo_page_avatar_class_init (AdapDemoPageAvatarClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/avatar/adap-demo-page-avatar.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAvatar, avatar);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAvatar, text);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAvatar, file_chooser_label);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAvatar, contacts);

  gtk_widget_class_install_action (widget_class, "avatar.open", NULL, (GtkWidgetActionActivateFunc) avatar_open_cb);
  gtk_widget_class_install_action (widget_class, "avatar.remove", NULL, (GtkWidgetActionActivateFunc) avatar_remove_cb);
  gtk_widget_class_install_action (widget_class, "avatar.save", NULL, (GtkWidgetActionActivateFunc) avatar_save_cb);
}

static void
adap_demo_page_avatar_init (AdapDemoPageAvatar *self)
{
  char *name;

  gtk_widget_init_template (GTK_WIDGET (self));

  name = create_random_name ();
  gtk_editable_set_text (GTK_EDITABLE (self->text), name);

  populate_contacts (self);
  avatar_remove_cb (self);

  g_free (name);
}
