#include "adap-demo-page-about.h"

#include <glib/gi18n.h>

struct _AdapDemoPageAbout
{
  AdapBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdapDemoPageAbout, adap_demo_page_about, ADAP_TYPE_BIN)

static void
demo_run_cb (AdapDemoPageAbout *self)
{
  AdapDialog *about;

  const char *developers[] = {
    "Angela Avery <angela@example.org>",
    NULL
  };

  const char *artists[] = {
    "GNOME Design Team",
    NULL
 };

  const char *special_thanks[] = {
    "My cat",
    NULL
 };

  const char *release_notes = "\
<p>\
  This release adds the following features:\
</p>\
<ul>\
  <li>Added a way to export fonts.</li>\
  <li>Better support for <code>monospace</code> fonts.</li>\
  <li>Added a way to preview <em>italic</em> text.</li>\
  <li>Bug fixes and performance improvements.</li>\
  <li>Translation updates.</li>\
</ul>\
  ";

  about =
    g_object_new (ADAP_TYPE_ABOUT_DIALOG,
                  "application-icon", "org.example.Typeset",
                  "application-name", _("Typeset"),
                  "developer-name", _("Angela Avery"),
                  "version", "1.2.3",
                  "release-notes-version", "1.2.0",
                  "release-notes", release_notes,
                  "comments", _("Typeset is an app that doesn’t exist and is used as an example content for this about window."),
                  "website", "https://example.org",
                  "issue-url", "https://example.org",
                  "support-url", "https://example.org",
                  "copyright", "© 2022 Angela Avery",
                  "license-type", GTK_LICENSE_LGPL_2_1,
                  "developers", developers,
                  "artists", artists,
                  "translator-credits", _("translator-credits"),
                  NULL);

  adap_about_dialog_add_link (ADAP_ABOUT_DIALOG (about),
                             _("_Documentation"),
                             "https://gnome.pages.gitlab.gnome.org/libadapta/doc/main/class.AboutDialog.html");

  adap_about_dialog_add_legal_section (ADAP_ABOUT_DIALOG (about),
                                      _("Fonts"),
                                      NULL,
                                      GTK_LICENSE_CUSTOM,
                                      "This application uses font data from <a href='https://example.org'>somewhere</a>.");

  adap_about_dialog_add_acknowledgement_section (ADAP_ABOUT_DIALOG (about),
                                                _("Special thanks to"),
                                                special_thanks);

  adap_dialog_present (about, GTK_WIDGET (self));
}

static void
adap_demo_page_about_class_init (AdapDemoPageAboutClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/about/adap-demo-page-about.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adap_demo_page_about_init (AdapDemoPageAbout *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
