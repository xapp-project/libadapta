/*
 * Copyright (C) 2021-2022 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <appstream.h>

#include "adw-about-dialog.h"

#include "adw-action-row.h"
#include "adw-header-bar.h"
#include "adw-marshalers.h"
#include "adw-alert-dialog.h"
#include "adw-navigation-view.h"
#include "adw-preferences-group.h"
#include "adw-toast-overlay.h"

/**
 * AdwAboutDialog:
 *
 * A dialog showing information about the application.
 *
 * <picture>
 *   <source srcset="about-dialog-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="about-dialog.png" alt="about-dialog">
 * </picture>
 *
 * an about dialog is typically opened when the user activates the `About …`
 * item in the application's primary menu. All parts of the dialog are optional.
 *
 * ## Main page
 *
 * `AdwAboutDialog` prominently displays the application's icon, name, developer
 * name and version. They can be set with the [property@AboutDialog:application-icon],
 * [property@AboutDialog:application-name],
 * [property@AboutDialog:developer-name] and [property@AboutDialog:version]
 * respectively.
 *
 * ## What's New
 *
 * `AdwAboutDialog` provides a way for applications to display their release
 * notes, set with the [property@AboutDialog:release-notes] property.
 *
 * Release notes are formatted the same way as
 * [AppStream descriptions](https://freedesktop.org/software/appstream/docs/chap-Metadata.html#tag-description).
 *
 * The supported formatting options are:
 *
 * * Paragraph (`<p>`)
 * * Ordered list (`<ol>`), with list items (`<li>`)
 * * Unordered list (`<ul>`), with list items (`<li>`)
 *
 * Within paragraphs and list items, emphasis (`<em>`) and inline code
 * (`<code>`) text styles are supported. The emphasis is rendered in italic,
 * while inline code is shown in a monospaced font.
 *
 * Any text outside paragraphs or list items is ignored.
 *
 * Nested lists are not supported.
 *
 * Only one version can be shown at a time. By default, the displayed version
 * number matches [property@AboutDialog:version]. Use
 * [property@AboutDialog:release-notes-version] to override it.
 *
 * ## Details
 *
 * The Details page displays the application comments and links.
 *
 * The comments can be set with the [property@AboutDialog:comments] property.
 * Unlike [property@Gtk.AboutDialog:comments], this string can be long and
 * detailed. It can also contain links and Pango markup.
 *
 * To set the application website, use [property@AboutDialog:website].
 * To add extra links below the website, use [method@AboutDialog.add_link].
 *
 * If the Details page doesn't have any other content besides website, the
 * website will be displayed on the main page instead.
 *
 * ## Troubleshooting
 *
 * `AdwAboutDialog` displays the following two links on the main page:
 *
 * * Support Questions, set with the [property@AboutDialog:support-url] property,
 * * Report an Issue, set with the [property@AboutDialog:issue-url] property.
 *
 * Additionally, applications can provide debugging information. It will be
 * shown separately on the Troubleshooting page. Use the
 * [property@AboutDialog:debug-info] property to specify it.
 *
 * It's intended to be attached to issue reports when reporting issues against
 * the application. As such, it cannot contain markup or links.
 *
 * `AdwAboutDialog` provides a quick way to save debug information to a file.
 * When saving, [property@AboutDialog:debug-info-filename] would be used as
 * the suggested filename.
 *
 * ## Credits and Acknowledgements
 *
 * The Credits page has the following default sections:
 *
 * * Developers, set with the [property@AboutDialog:developers] property,
 * * Designers, set with the [property@AboutDialog:designers] property,
 * * Artists, set with the [property@AboutDialog:artists] property,
 * * Documenters, set with the [property@AboutDialog:documenters] property,
 * * Translators, set with the [property@AboutDialog:translator-credits] property.
 *
 * When setting translator credits, use the strings `"translator-credits"` or
 * `"translator_credits"` and mark them as translatable.
 *
 * The default sections that don't contain any names won't be displayed.
 *
 * The Credits page can also contain an arbitrary number of extra sections below
 * the default ones. Use [method@AboutDialog.add_credit_section] to add them.
 *
 * The Acknowledgements page can be used to acknowledge additional people and
 * organizations for their non-development contributions. Use
 * [method@AboutDialog.add_acknowledgement_section] to add sections to it. For
 * example, it can be used to list backers in a crowdfunded project or to give
 * special thanks.
 *
 * Each of the people or organizations can have an email address or a website
 * specified. To add a email address, use a string like
 * `Edgar Allan Poe <edgar@poe.com>`. To specify a website with a title, use a
 * string like `The GNOME Project https://www.gnome.org`:
 *
 * <picture>
 *   <source srcset="about-dialog-credits-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="about-dialog-credits.png" alt="about-dialog-credits">
 * </picture>
 *
 * ## Legal
 *
 * The Legal page displays the copyright and licensing information for the
 * application and other modules.
 *
 * The copyright string is set with the [property@AboutDialog:copyright]
 * property and should be a short string of one or two lines, for example:
 * `© 2022 Example`.
 *
 * Licensing information can be quickly set from a list of known licenses with
 * the [property@AboutDialog:license-type] property. If the application's
 * license is not in the list, [property@AboutDialog:license] can be used
 * instead.
 *
 * To add information about other modules, such as application dependencies or
 * data, use [method@AboutDialog.add_legal_section].
 *
 * ## Constructing
 *
 * To make constructing an `AdwAboutDialog` as convenient as possible, you can
 * use the function [func@show_about_dialog] which constructs and shows a
 * dialog.
 *
 * ```c
 * static void
 * show_about (GtkApplication *app)
 * {
 *   const char *developers[] = {
 *     "Angela Avery",
 *     NULL
 *   };
 *
 *   const char *designers[] = {
 *     "GNOME Design Team",
 *     NULL
 *   };
 *
 *   adw_show_about_dialog (GTK_WIDGET (gtk_application_get_active_window (app)),
 *                          "application-name", _("Example"),
 *                          "application-icon", "org.example.App",
 *                          "version", "1.2.3",
 *                          "copyright", "© 2022 Angela Avery",
 *                          "issue-url", "https://gitlab.gnome.org/example/example/-/issues/new",
 *                          "license-type", GTK_LICENSE_GPL_3_0,
 *                          "developers", developers,
 *                          "designers", designers,
 *                          "translator-credits", _("translator-credits"),
 *                          NULL);
 * }
 * ```
 *
 * ## CSS nodes
 *
 * `AdwAboutDialog` has a main CSS node with the name `dialog` and the
 * style class `.about`.
 *
 * Since: 1.5
 */

/* Copied from GTK 4 for consistency with GtkAboutDialog. */
typedef struct {
  const char *name;
  const char *url;
  const char *spdx_id;
} LicenseInfo;

/* Copied from GTK 4 for consistency with GtkAboutDialog. */
/* LicenseInfo for each GtkLicense type; keep in the same order as the
 * enumeration. */
static const LicenseInfo gtk_license_info [] = {
  { NULL, NULL, NULL },
  { NULL, NULL, NULL },
  { N_("GNU General Public License, version 2 or later"), "https://www.gnu.org/licenses/old-licenses/gpl-2.0.html", "GPL-2.0-or-later" },
  { N_("GNU General Public License, version 3 or later"), "https://www.gnu.org/licenses/gpl-3.0.html", "GPL-3.0-or-later" },
  { N_("GNU Lesser General Public License, version 2.1 or later"), "https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html", "LGPL-2.1-or-later" },
  { N_("GNU Lesser General Public License, version 3 or later"), "https://www.gnu.org/licenses/lgpl-3.0.html", "LGPL-3.0-or-later" },
  { N_("BSD 2-Clause License"), "https://opensource.org/licenses/bsd-license.php", "BSD-2-Clause" },
  { N_("The MIT License (MIT)"), "https://opensource.org/licenses/mit-license.php", "MIT" },
  { N_("Artistic License 2.0"), "https://opensource.org/licenses/artistic-license-2.0.php", "Artistic-2.0" },
  { N_("GNU General Public License, version 2 only"), "https://www.gnu.org/licenses/old-licenses/gpl-2.0.html", "GPL-2.0-only" },
  { N_("GNU General Public License, version 3 only"), "https://www.gnu.org/licenses/gpl-3.0.html", "GPL-3.0-only" },
  { N_("GNU Lesser General Public License, version 2.1 only"), "https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html", "LGPL-2.1-only" },
  { N_("GNU Lesser General Public License, version 3 only"), "https://www.gnu.org/licenses/lgpl-3.0.html", "LGPL-3.0-only" },
  { N_("GNU Affero General Public License, version 3 or later"), "https://www.gnu.org/licenses/agpl-3.0.html", "AGPL-3.0-or-later" },
  { N_("GNU Affero General Public License, version 3 only"), "https://www.gnu.org/licenses/agpl-3.0.html", "AGPL-3.0-only" },
  { N_("BSD 3-Clause License"), "https://opensource.org/licenses/BSD-3-Clause", "BSD-3-Clause" },
  { N_("Apache License, Version 2.0"), "https://opensource.org/licenses/Apache-2.0", "Apache-2.0" },
  { N_("Mozilla Public License 2.0"), "https://opensource.org/licenses/MPL-2.0", "MPL-2.0" },
  { N_("BSD Zero-Clause License"), "https://opensource.org/license/0bsd", "0BSD" }
};
/* Copied from GTK 4 for consistency with GtkAboutDialog. */
/* Keep this static assertion updated with the last element of the enumeration,
 * and make sure it matches the last element of the array. */
G_STATIC_ASSERT (G_N_ELEMENTS (gtk_license_info) - 1 == GTK_LICENSE_0BSD);

typedef struct {
  const char *spdx_id;
  GtkLicense license;
} LicenseAlias;

/* Deprecated SPDX IDs */
static const LicenseAlias license_aliases [] = {
  { "GPL-2.0", GTK_LICENSE_GPL_2_0_ONLY },
  { "GPL-3.0", GTK_LICENSE_GPL_3_0_ONLY },
};

typedef struct {
  char *name;
  char **people;
} CreditsSection;

typedef struct {
  char *title;
  char *copyright;
  char *license;
  GtkLicense license_type;
} LegalSection;

struct _AdwAboutDialog {
  AdwDialog parent_instance;

  GtkWidget *navigation_view;
  GtkWidget *toast_overlay;
  GtkWidget *main_scrolled_window;
  GtkWidget *main_headerbar;

  GtkWidget *app_icon_image;
  GtkWidget *app_name_label;
  GtkWidget *developer_name_label;
  GtkWidget *version_button;

  GtkWidget *details_group;
  GtkWidget *whats_new_row;
  GtkWidget *comments_label;
  GtkWidget *website_row;
  GtkWidget *links_group;
  GtkWidget *details_website_row;
  GtkWidget *details_row;
  GtkTextBuffer *release_notes_buffer;

  GtkWidget *support_group;
  GtkWidget *support_row;
  GtkWidget *issue_row;
  GtkWidget *troubleshooting_row;
  AdwNavigationPage *debug_info_page;

  GtkWidget *credits_legal_group;
  GtkWidget *credits_box;
  GtkWidget *legal_box;
  GtkWidget *acknowledgements_box;

  char *application_icon;
  char *application_name;
  char *developer_name;
  char *version;
  char *release_notes_version;
  char *release_notes;
  char *comments;
  char *website;
  char *support_url;
  char *issue_url;
  char *debug_info;
  char *debug_info_filename;
  char **developers;
  char **designers;
  char **artists;
  char **documenters;
  char *translator_credits;
  GSList *credit_sections;
  char *copyright;
  char *license;
  GtkLicense license_type;
  GSList *legal_sections;
  gboolean has_custom_links;

  guint legal_showing_idle_id;
};

G_DEFINE_FINAL_TYPE (AdwAboutDialog, adw_about_dialog, ADW_TYPE_DIALOG)

enum {
  PROP_0,
  PROP_APPLICATION_ICON,
  PROP_APPLICATION_NAME,
  PROP_DEVELOPER_NAME,
  PROP_VERSION,
  PROP_RELEASE_NOTES_VERSION,
  PROP_RELEASE_NOTES,
  PROP_COMMENTS,
  PROP_WEBSITE,
  PROP_SUPPORT_URL,
  PROP_ISSUE_URL,
  PROP_DEBUG_INFO,
  PROP_DEBUG_INFO_FILENAME,
  PROP_DEVELOPERS,
  PROP_DESIGNERS,
  PROP_ARTISTS,
  PROP_DOCUMENTERS,
  PROP_TRANSLATOR_CREDITS,
  PROP_COPYRIGHT,
  PROP_LICENSE_TYPE,
  PROP_LICENSE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_ACTIVATE_LINK,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
free_credit_section (CreditsSection *section)
{
  g_free (section->name);
  g_strfreev (section->people);
  g_free (section);
}

static void
free_legal_section (LegalSection *section)
{
  g_free (section->title);
  g_free (section->license);
  g_free (section);
}

static void
update_headerbar_cb (AdwAboutDialog *self)
{
  GtkAdjustment *adj;

  adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->main_scrolled_window));

  adw_header_bar_set_show_title (ADW_HEADER_BAR (self->main_headerbar),
                                 gtk_adjustment_get_value (adj) > 0);
}

static inline void
activate_link (AdwAboutDialog *self,
               const char     *uri)
{
  gboolean ret = FALSE;
  g_signal_emit (self, signals[SIGNAL_ACTIVATE_LINK], 0, uri, &ret);
}

static gboolean
activate_link_cb (AdwAboutDialog *self,
                  const char     *uri)
{
  activate_link (self, uri);

  return GDK_EVENT_STOP;
}

static gboolean
activate_link_default_cb (AdwAboutDialog *self,
                          const char     *uri)
{
  GtkUriLauncher *launcher = gtk_uri_launcher_new (uri);
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  gtk_uri_launcher_launch (launcher, GTK_WINDOW (root), NULL, NULL, NULL);

  g_object_unref (launcher);

  return GDK_EVENT_STOP;
}

static void
legal_showing_idle_cb (AdwAboutDialog *self)
{
  GtkWidget *focus = adw_dialog_get_focus (ADW_DIALOG (self));

  if (GTK_IS_LABEL (focus) && !gtk_label_get_current_uri (GTK_LABEL (focus)))
    gtk_label_select_region (GTK_LABEL (focus), 0, 0);
}

static void
legal_showing_cb (AdwAboutDialog *self)
{
  self->legal_showing_idle_id =
    g_idle_add_once ((GSourceOnceFunc) legal_showing_idle_cb, self);

  self->legal_showing_idle_id = 0;
}

static gboolean
get_release_for_version (AsRelease  *rel,
                         const char *version)
{
  return !g_strcmp0 (as_release_get_version (rel), version);
}

static void
update_credits_legal_group (AdwAboutDialog *self)
{
  gtk_widget_set_visible (self->credits_legal_group,
                          gtk_widget_get_visible (self->credits_box) ||
                          gtk_widget_get_visible (self->legal_box) ||
                          gtk_widget_get_visible (self->acknowledgements_box));
}

/* Adapted from text_buffer_new() in gtkaboutdialog.c */
static void
parse_person (char      *person,
              char     **name,
              char     **link,
              gboolean  *is_email)
{

  const char *q1, *q2, *r1, *r2;

  q1 = strchr (person, '<');
  q2 = q1 ? strchr (q1, '>') : NULL;
  r1 = strstr (person, "http://");
  r2 = strstr (person, "https://");

  if (!r1 || (r1 && r2 && r2 < r1))
    r1 = r2;
  if (r1) {
    r2 = strpbrk (r1, " \n\t>");
    if (!r2)
      r2 = strchr (r1, '\0');
  } else {
    r2 = NULL;
  }

  if (r1 && r2 && (!q1 || !q2 || (r1 <= q1 + 1))) {
    q1 = r1;
    q2 = r2;
  }

  if (q1 && q2) {
    *name = g_strndup (person, q1 - person);
    *is_email = (*q1 == '<');

    if (*is_email)
      *link = g_strndup (q1 + 1, q2 - q1 - 1);
    else
      *link = g_strndup (q1, q2 - q1);
  } else {
    *name = g_strdup (person);
    *link = NULL;
    *is_email = FALSE;
  }

  g_strstrip (*name);
}

static void
add_credits_section (GtkWidget   *box,
                     const char  *title,
                     char       **people)
{
  GtkWidget *group;
  char **person;

  if (!people || !*people)
    return;

  group = adw_preferences_group_new ();
  adw_preferences_group_set_title (ADW_PREFERENCES_GROUP (group), title);

  for (person = people; *person; person++) {
    GtkWidget *row;
    char *name = NULL;
    char *link = NULL;
    gboolean is_email = FALSE;

    if (!*person)
      continue;

    parse_person (*person, &name, &link, &is_email);

    row = adw_action_row_new ();
    adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (row), FALSE);
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (row), name);
    adw_preferences_group_add (ADW_PREFERENCES_GROUP (group), row);

    if (link) {
      GtkWidget *image = g_object_new (GTK_TYPE_IMAGE,
                                       "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                       NULL);

      if (is_email)
        gtk_image_set_from_icon_name (GTK_IMAGE (image), "adw-mail-send-symbolic");
      else
        gtk_image_set_from_icon_name (GTK_IMAGE (image), "adw-external-link-symbolic");

      adw_action_row_add_suffix (ADW_ACTION_ROW (row), image);
      gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), TRUE);
      gtk_actionable_set_action_name (GTK_ACTIONABLE (row), "about.show-url");

      if (is_email) {
        char *escaped = g_uri_escape_string (link, NULL, FALSE);
        char *mailto = g_strconcat ("mailto:", escaped, NULL);

        gtk_actionable_set_action_target (GTK_ACTIONABLE (row), "s", mailto);

        g_free (mailto);
        g_free (escaped);
      } else {
        gtk_actionable_set_action_target (GTK_ACTIONABLE (row), "s", link);
      }

      gtk_widget_set_tooltip_text (row, link);
    }

    g_free (name);
    g_free (link);
  }

  gtk_box_append (GTK_BOX (box), group);
}

static void
update_credits (AdwAboutDialog *self)
{
  GtkWidget *widget;
  char **translators;
  GSList *l;

  while ((widget = gtk_widget_get_first_child (self->credits_box)))
    gtk_box_remove (GTK_BOX (self->credits_box), widget);

  if (self->translator_credits &&
      g_strcmp0 (self->translator_credits, "translator_credits") &&
      g_strcmp0 (self->translator_credits, "translator-credits"))
    translators = g_strsplit (self->translator_credits, "\n", 0);
  else
    translators = NULL;

  add_credits_section (self->credits_box, _("Code by"),          self->developers);
  add_credits_section (self->credits_box, _("Design by"),        self->designers);
  add_credits_section (self->credits_box, _("Artwork by"),       self->artists);
  add_credits_section (self->credits_box, _("Documentation by"), self->documenters);
  add_credits_section (self->credits_box, _("Translated by"),    translators);

  for (l = self->credit_sections; l; l = l->next) {
    CreditsSection *section = l->data;

    add_credits_section (self->credits_box, section->name, section->people);
  }

  g_strfreev (translators);

  gtk_widget_set_visible (self->credits_box,
                          !!gtk_widget_get_first_child (self->credits_box));

  update_credits_legal_group (self);
}

static char *
get_license_text (GtkLicense  license_type,
                  const char *license)
{
  if (license_type == GTK_LICENSE_UNKNOWN)
    return NULL;

  if (license_type == GTK_LICENSE_CUSTOM)
    return g_strdup (license);

  /* Translators: this is the license preamble; the string at the end
   * contains the name of the license as link text.
   */
  return g_strdup_printf (_("This application comes with absolutely no warranty. See the <a href=\"%s\">%s</a> for details."),
                          gtk_license_info[license_type].url,
                          _(gtk_license_info[license_type].name));
}

static void
append_legal_section (AdwAboutDialog *self,
                      LegalSection   *section,
                      gboolean        force_title)
{
  GtkWidget *label;
  char *license;

  if (force_title)
    g_assert (section->title);

  license = get_license_text (section->license_type, section->license);

  if ((!section->copyright || !*section->copyright) &&
      (!license || !*license) && !force_title) {
    g_free (license);

    return;
  }

  if (section->title && *section->title) {
    label = gtk_label_new (section->title);
    gtk_label_set_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_xalign (GTK_LABEL (label), 0);
    gtk_widget_add_css_class (label, "heading");
    gtk_box_append (GTK_BOX (self->legal_box), label);
  }

  if ((!section->copyright || !*section->copyright) &&
      (!license || !*license)) {
    g_free (license);

    return;
  }

  label = gtk_label_new (NULL);
  gtk_label_set_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD_CHAR);
  gtk_label_set_xalign (GTK_LABEL (label), 0);
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_widget_add_css_class (label, "body");
  g_signal_connect_swapped (label, "activate-link", G_CALLBACK (activate_link_cb), self);

  if (section->copyright && *section->copyright && license && *license) {
    char *text = g_strconcat (section->copyright, "\n\n", license, NULL);

    gtk_label_set_markup (GTK_LABEL (label), text);

    g_free (text);
  } else if (section->copyright && *section->copyright) {
    gtk_label_set_markup (GTK_LABEL (label), section->copyright);
  } else {
    gtk_label_set_markup (GTK_LABEL (label), license);
  }

  gtk_box_append (GTK_BOX (self->legal_box), label);

  g_free (license);
}

static void
update_legal (AdwAboutDialog *self)
{
  LegalSection default_section;
  GtkWidget *widget;
  GSList *l;

  while ((widget = gtk_widget_get_first_child (self->legal_box)))
    gtk_box_remove (GTK_BOX (self->legal_box), widget);

  /* We only want to show the default title if there's more than one section */
  if (self->legal_sections)
    default_section.title = _("This Application");
  else
    default_section.title = NULL;

  default_section.copyright = self->copyright;
  default_section.license_type = self->license_type;
  default_section.license = self->license;
  append_legal_section (self, &default_section, FALSE);

  for (l = self->legal_sections; l; l = l->next)
    append_legal_section (self, l->data, TRUE);

  gtk_widget_set_visible (self->legal_box,
                          !!gtk_widget_get_first_child (self->legal_box));

  update_credits_legal_group (self);
}

typedef enum {
  STATE_NONE,
  STATE_PARAGRAPH,
  STATE_UNORDERED_LIST,
  STATE_UNORDERED_ITEM,
  STATE_ORDERED_LIST,
  STATE_ORDERED_ITEM,
} ReleaseNotesState;

typedef struct {
  GtkTextBuffer *buffer;
  GtkTextIter iter;

  ReleaseNotesState state;
  int n_item;
  int section_start;
  int paragraph_start;
  gboolean last_trailing_space;
} ReleaseNotesParserData;

static void
start_element_handler (GMarkupParseContext  *context,
                       const char           *element_name,
                       const char          **attribute_names,
                       const char          **attribute_values,
                       gpointer              user_data,
                       GError              **error)
{
  ReleaseNotesParserData *pdata = user_data;

  switch (pdata->state) {
  case STATE_NONE:
    if (!g_strcmp0 (element_name, "p")) {
      pdata->state = STATE_PARAGRAPH;
      pdata->paragraph_start = gtk_text_iter_get_offset (&pdata->iter);
    }

    if (!g_strcmp0 (element_name, "ul"))
      pdata->state = STATE_UNORDERED_LIST;

    if (!g_strcmp0 (element_name, "ol"))
      pdata->state = STATE_ORDERED_LIST;

    if (pdata->state != STATE_NONE) {
      pdata->section_start = gtk_text_iter_get_offset (&pdata->iter);
      break;
    }

    g_set_error (error,
                 G_MARKUP_ERROR,
                 G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                 "Unexpected element '%s'",
                 element_name);
    break;

  case STATE_PARAGRAPH:
  case STATE_UNORDERED_ITEM:
  case STATE_ORDERED_ITEM:
    if (!g_strcmp0 (element_name, "em") ||
        !g_strcmp0 (element_name, "code")) {
      break;
    }

    g_set_error (error,
                 G_MARKUP_ERROR,
                 G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                 "Unexpected element '%s'",
                 element_name);
    break;

  case STATE_UNORDERED_LIST:
  case STATE_ORDERED_LIST:
    if (!g_strcmp0 (element_name, "li")) {
      char *bullet;

      if (pdata->n_item > 0)
        gtk_text_buffer_insert (pdata->buffer, &pdata->iter, "\n", -1);

      if (pdata->state == STATE_ORDERED_LIST) {
        pdata->state = STATE_ORDERED_ITEM;
        bullet = g_strdup_printf ("%d. ", pdata->n_item + 1);
      } else {
        pdata->state = STATE_UNORDERED_ITEM;
        bullet = g_strdup ("• ");
      }

      gtk_text_buffer_insert_with_tags_by_name (pdata->buffer, &pdata->iter, bullet, -1, "bullet", NULL);
      pdata->paragraph_start = gtk_text_iter_get_offset (&pdata->iter);

      g_free (bullet);

      break;
    }

    g_set_error (error,
                 G_MARKUP_ERROR,
                 G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                 "Unexpected element '%s'",
                 element_name);
    break;

  default:
    g_assert_not_reached ();
  }

  if (!g_strcmp0 (element_name, "em") ||
      !g_strcmp0 (element_name, "code") ||
      !g_strcmp0 (element_name, "ul") ||
      !g_strcmp0 (element_name, "ol") ||
      !g_strcmp0 (element_name, "li")) {
    g_markup_collect_attributes (element_name,
                                 attribute_names,
                                 attribute_values,
                                 error,
                                 G_MARKUP_COLLECT_INVALID,
                                 NULL);
    return;
  }

  /* We don't support attributes anywhere, so error out if we encounter any. */
  g_markup_collect_attributes (element_name,
                               attribute_names,
                               attribute_values,
                               error,
                               G_MARKUP_COLLECT_INVALID,
                               NULL);
}

static void
end_element_handler (GMarkupParseContext  *context,
                     const char           *element_name,
                     gpointer              user_data,
                     GError              **error)
{
  ReleaseNotesParserData *pdata = user_data;

  if (!g_strcmp0 (element_name, "p") ||
      !g_strcmp0 (element_name, "ul") ||
      !g_strcmp0 (element_name, "ol")) {

   if (pdata->section_start != gtk_text_iter_get_offset (&pdata->iter)) {
     gtk_text_buffer_insert (pdata->buffer, &pdata->iter, "\n", -1);

     if (pdata->section_start > 0 && !g_strcmp0 (element_name, "p")) {
       GtkTextIter start_iter;
       gtk_text_buffer_get_iter_at_offset (pdata->buffer, &start_iter, pdata->section_start);
       gtk_text_buffer_apply_tag_by_name (pdata->buffer, "section", &start_iter, &pdata->iter);
     }
   }

    pdata->state = STATE_NONE;
    pdata->section_start = -1;
    pdata->paragraph_start = -1;
    pdata->n_item = 0;
    return;
  }

  if (!g_strcmp0 (element_name, "li")) {
    if (pdata->state == STATE_UNORDERED_ITEM)
      pdata->state = STATE_UNORDERED_LIST;
    else if (pdata->state == STATE_ORDERED_ITEM)
      pdata->state = STATE_ORDERED_LIST;
    else
      g_assert_not_reached ();

    if (pdata->section_start > 0 && pdata->n_item == 0) {
       GtkTextIter start_iter;
       gtk_text_buffer_get_iter_at_offset (pdata->buffer, &start_iter,
                                           pdata->section_start);
       gtk_text_buffer_apply_tag_by_name (pdata->buffer, "section", &start_iter, &pdata->iter);
     }

    pdata->n_item++;
    pdata->paragraph_start = -1;
    return;
  }
}

static void
text_handler (GMarkupParseContext  *context,
              const char           *text,
              gsize                 text_len,
              gpointer              user_data,
              GError              **error)
{
  ReleaseNotesParserData *pdata = user_data;
  const char *element;
  char *escaped;
  static GRegex *regex = NULL;
  gboolean leading_space;
  gboolean trailing_space;

  if (pdata->state != STATE_PARAGRAPH &&
      pdata->state != STATE_UNORDERED_ITEM &&
      pdata->state != STATE_ORDERED_ITEM)
    return;

  /* Replace random amounts of spaces and newlines with single spaces */
  if (!regex) {
    GError *regex_error = NULL;

    regex = g_regex_new ("\\s+", 0, 0, &regex_error);

    if (regex_error) {
      g_error ("Couldn't compile regex: %s", regex_error->message);
      g_error_free (regex_error);
    }
  }

  element = g_markup_parse_context_get_element (context);

  escaped = g_regex_replace_literal (regex, text, text_len, 0, " ", 0, error);

  if (*error)
    return;

  if (!*escaped) {
    g_free (escaped);
    return;
  }

  leading_space = *escaped == ' ';
  trailing_space = escaped[strlen (escaped) - 1] == ' ';

  g_strstrip (escaped);

  /* This might have made the string empty, skip it in that case */
  if (!*escaped) {
    g_free (escaped);
    pdata->last_trailing_space = trailing_space;
    return;
  }

  /* We've got rid of inner spaces before <em> and <code>. Bring them back */
  if ((leading_space || pdata->last_trailing_space) &&
      pdata->paragraph_start != gtk_text_iter_get_offset (&pdata->iter))
    gtk_text_buffer_insert (pdata->buffer, &pdata->iter, " ", -1);

  if (!g_strcmp0 (element, "em") || !g_strcmp0 (element, "code"))
    gtk_text_buffer_insert_with_tags_by_name (pdata->buffer, &pdata->iter,
                                              escaped, -1, element, NULL);
  else
    gtk_text_buffer_insert (pdata->buffer, &pdata->iter, escaped, -1);

  pdata->last_trailing_space = trailing_space;

  g_free (escaped);
}

static const GMarkupParser markup_parser =
{
  start_element_handler,
  end_element_handler,
  text_handler,
  NULL,
  NULL
};

static void
update_release_notes (AdwAboutDialog *self)
{
  GMarkupParseContext *context;
  ReleaseNotesParserData pdata;
  GError *error = NULL;
  GtkTextIter end_iter;
  const char *version = NULL;

  gtk_text_buffer_set_text (self->release_notes_buffer, "", -1);

  if (!self->release_notes || !*self->release_notes) {
    gtk_widget_set_visible (self->whats_new_row, FALSE);

    return;
  }

  pdata.buffer = self->release_notes_buffer;
  gtk_text_buffer_get_start_iter (self->release_notes_buffer, &pdata.iter);

  if (self->release_notes_version && *self->release_notes_version)
    version = self->release_notes_version;
  else if (self->version && *self->version)
    version = self->version;

  if (version) {
    char *heading = g_strdup_printf (_("Version %s"), version);

    gtk_text_buffer_insert_with_tags_by_name (self->release_notes_buffer,
                                              &pdata.iter, heading, -1,
                                              "heading", NULL);
    gtk_text_buffer_insert (self->release_notes_buffer, &pdata.iter, "\n", -1);

    g_free (heading);
  }

  pdata.state = STATE_NONE;
  pdata.n_item = 0;
  pdata.last_trailing_space = FALSE;
  context = g_markup_parse_context_new (&markup_parser, 0, &pdata, NULL);

  if (!g_markup_parse_context_parse (context, self->release_notes, -1, &error) ||
      !g_markup_parse_context_end_parse (context, &error)) {
    int line_number, char_number;
    char *position;
    g_markup_parse_context_get_position (context, &line_number, &char_number);

    g_critical ("Unable to parse release notes: %s at line %d, char %d", error->message, line_number, char_number);

    gtk_text_buffer_set_text (self->release_notes_buffer, "", -1);
    gtk_text_buffer_get_start_iter (self->release_notes_buffer, &pdata.iter);

    gtk_text_buffer_insert (self->release_notes_buffer, &pdata.iter, _("Unable to parse release notes:"), -1);
    gtk_text_buffer_insert (self->release_notes_buffer, &pdata.iter, "\n", -1);

    gtk_text_buffer_insert (self->release_notes_buffer, &pdata.iter, error->message, -1);
    gtk_text_buffer_insert (self->release_notes_buffer, &pdata.iter, "\n", -1);

    position = g_strdup_printf (_("Line: %d, character: %d"), line_number, char_number);
    gtk_text_buffer_insert (self->release_notes_buffer, &pdata.iter, position, -1);

    g_markup_parse_context_free (context);
    g_error_free (error);
    g_free (position);

    gtk_widget_set_visible (self->whats_new_row, TRUE);

    return;
  }

  /* Remove the trailing newline */
  gtk_text_iter_backward_chars (&pdata.iter, 1);
  gtk_text_buffer_get_end_iter (self->release_notes_buffer, &end_iter);
  gtk_text_buffer_delete (self->release_notes_buffer, &pdata.iter, &end_iter);

  g_markup_parse_context_free (context);

  gtk_widget_set_visible (self->whats_new_row, TRUE);
}

static void
update_details (AdwAboutDialog *self)
{
  gboolean has_website = self->website && *self->website;
  gboolean has_comments = self->comments && *self->comments;
  gboolean has_release_notes = gtk_widget_get_visible (self->whats_new_row);
  gboolean show_details = has_comments || self->has_custom_links;
  gboolean show_links = (has_website && has_comments) || self->has_custom_links;

  gtk_widget_set_visible (self->comments_label, has_comments);
  gtk_widget_set_visible (self->website_row, has_website && !show_details);
  gtk_widget_set_visible (self->details_website_row, has_website && show_details);
  gtk_widget_set_visible (self->links_group, show_links);
  gtk_widget_set_visible (self->details_row, has_comments || show_links);
  gtk_widget_set_visible (self->details_group,
                          has_website || has_comments ||
                          show_links || has_release_notes);
}

static void
update_support (AdwAboutDialog *self)
{
  gboolean has_support_url = self->support_url && *self->support_url;
  gboolean has_issue_url = self->issue_url && *self->issue_url;
  gboolean has_debug_info = self->debug_info && *self->debug_info;

  gtk_widget_set_visible (self->support_row, has_support_url);
  gtk_widget_set_visible (self->issue_row, has_issue_url);
  gtk_widget_set_visible (self->troubleshooting_row, has_debug_info);
  gtk_widget_set_visible (self->support_group,
                          has_support_url || has_issue_url || has_debug_info);
}

static void
adw_about_dialog_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwAboutDialog *self = ADW_ABOUT_DIALOG (object);

  switch (prop_id) {
  case PROP_APPLICATION_ICON:
    g_value_set_string (value, adw_about_dialog_get_application_icon (self));
    break;
  case PROP_APPLICATION_NAME:
    g_value_set_string (value, adw_about_dialog_get_application_name (self));
    break;
  case PROP_DEVELOPER_NAME:
    g_value_set_string (value, adw_about_dialog_get_developer_name (self));
    break;
  case PROP_VERSION:
    g_value_set_string (value, adw_about_dialog_get_version (self));
    break;
  case PROP_RELEASE_NOTES_VERSION:
    g_value_set_string (value, adw_about_dialog_get_release_notes_version (self));
    break;
  case PROP_RELEASE_NOTES:
    g_value_set_string (value, adw_about_dialog_get_release_notes (self));
    break;
  case PROP_COMMENTS:
    g_value_set_string (value, adw_about_dialog_get_comments (self));
    break;
  case PROP_WEBSITE:
    g_value_set_string (value, adw_about_dialog_get_website (self));
    break;
  case PROP_SUPPORT_URL:
    g_value_set_string (value, adw_about_dialog_get_support_url (self));
    break;
  case PROP_ISSUE_URL:
    g_value_set_string (value, adw_about_dialog_get_issue_url (self));
    break;
  case PROP_DEBUG_INFO:
    g_value_set_string (value, adw_about_dialog_get_debug_info (self));
    break;
  case PROP_DEBUG_INFO_FILENAME:
    g_value_set_string (value, adw_about_dialog_get_debug_info_filename (self));
    break;
  case PROP_DEVELOPERS:
    g_value_set_boxed (value, adw_about_dialog_get_developers (self));
    break;
  case PROP_DESIGNERS:
    g_value_set_boxed (value, adw_about_dialog_get_designers (self));
    break;
  case PROP_ARTISTS:
    g_value_set_boxed (value, adw_about_dialog_get_artists (self));
    break;
  case PROP_DOCUMENTERS:
    g_value_set_boxed (value, adw_about_dialog_get_documenters (self));
    break;
  case PROP_TRANSLATOR_CREDITS:
    g_value_set_string (value, adw_about_dialog_get_translator_credits (self));
    break;
  case PROP_COPYRIGHT:
    g_value_set_string (value, adw_about_dialog_get_copyright (self));
    break;
  case PROP_LICENSE_TYPE:
    g_value_set_enum (value, adw_about_dialog_get_license_type (self));
    break;
  case PROP_LICENSE:
    g_value_set_string (value, adw_about_dialog_get_license (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_about_dialog_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwAboutDialog *self = ADW_ABOUT_DIALOG (object);

  switch (prop_id) {
  case PROP_APPLICATION_ICON:
    adw_about_dialog_set_application_icon (self, g_value_get_string (value));
    break;
  case PROP_APPLICATION_NAME:
    adw_about_dialog_set_application_name (self, g_value_get_string (value));
    break;
  case PROP_DEVELOPER_NAME:
    adw_about_dialog_set_developer_name (self, g_value_get_string (value));
    break;
  case PROP_VERSION:
    adw_about_dialog_set_version (self, g_value_get_string (value));
    break;
  case PROP_RELEASE_NOTES_VERSION:
    adw_about_dialog_set_release_notes_version (self, g_value_get_string (value));
    break;
  case PROP_RELEASE_NOTES:
    adw_about_dialog_set_release_notes (self, g_value_get_string (value));
    break;
  case PROP_COMMENTS:
    adw_about_dialog_set_comments (self, g_value_get_string (value));
    break;
  case PROP_WEBSITE:
    adw_about_dialog_set_website (self, g_value_get_string (value));
    break;
  case PROP_SUPPORT_URL:
    adw_about_dialog_set_support_url (self, g_value_get_string (value));
    break;
  case PROP_ISSUE_URL:
    adw_about_dialog_set_issue_url (self, g_value_get_string (value));
    break;
  case PROP_DEBUG_INFO:
    adw_about_dialog_set_debug_info (self, g_value_get_string (value));
    break;
  case PROP_DEBUG_INFO_FILENAME:
    adw_about_dialog_set_debug_info_filename (self, g_value_get_string (value));
    break;
  case PROP_DEVELOPERS:
    adw_about_dialog_set_developers (self, g_value_get_boxed (value));
    break;
  case PROP_DESIGNERS:
    adw_about_dialog_set_designers (self, g_value_get_boxed (value));
    break;
  case PROP_DOCUMENTERS:
    adw_about_dialog_set_documenters (self, g_value_get_boxed (value));
    break;
  case PROP_ARTISTS:
    adw_about_dialog_set_artists (self, g_value_get_boxed (value));
    break;
  case PROP_TRANSLATOR_CREDITS:
    adw_about_dialog_set_translator_credits (self, g_value_get_string (value));
    break;
  case PROP_COPYRIGHT:
    adw_about_dialog_set_copyright (self, g_value_get_string (value));
    break;
  case PROP_LICENSE_TYPE:
    adw_about_dialog_set_license_type (self, g_value_get_enum (value));
    break;
  case PROP_LICENSE:
    adw_about_dialog_set_license (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_about_dialog_dispose (GObject *object)
{
  AdwAboutDialog *self = ADW_ABOUT_DIALOG (object);

  g_clear_handle_id (&self->legal_showing_idle_id, g_source_remove);

  G_OBJECT_CLASS (adw_about_dialog_parent_class)->dispose (object);
}

static void
adw_about_dialog_finalize (GObject *object)
{
  AdwAboutDialog *self = ADW_ABOUT_DIALOG (object);

  g_free (self->application_icon);
  g_free (self->application_name);
  g_free (self->developer_name);
  g_free (self->version);
  g_free (self->release_notes_version);
  g_free (self->release_notes);
  g_free (self->comments);
  g_free (self->website);
  g_free (self->support_url);
  g_free (self->issue_url);
  g_free (self->debug_info);
  g_free (self->debug_info_filename);

  g_strfreev (self->developers);
  g_strfreev (self->designers);
  g_strfreev (self->artists);
  g_strfreev (self->documenters);
  g_free (self->translator_credits);
  g_slist_free_full (self->credit_sections, (GDestroyNotify) free_credit_section);

  g_free (self->copyright);
  g_free (self->license);
  g_slist_free_full (self->legal_sections, (GDestroyNotify) free_legal_section);

  G_OBJECT_CLASS (adw_about_dialog_parent_class)->finalize (object);
}

static void
show_url_cb (AdwAboutDialog *self,
             const char     *action_name,
             GVariant       *params)
{
  const char *url = g_variant_get_string (params, NULL);

  activate_link (self, url);
}

static void
show_url_property_cb (AdwAboutDialog *self,
                      const char     *action_name,
                      GVariant       *params)
{
  const char *property = g_variant_get_string (params, NULL);
  char *url;

  g_object_get (self, property, &url, NULL);

  activate_link (self, url);

  g_free (url);
}

static void
copy_property_cb (AdwAboutDialog *self,
                  const char     *action_name,
                  GVariant       *params)
{
  const char *property = g_variant_get_string (params, NULL);
  char *value;

  g_object_get (self, property, &value, NULL);

  if (value && *value) {
    GdkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (self));

    gdk_clipboard_set_text (clipboard, value);

    adw_toast_overlay_add_toast (ADW_TOAST_OVERLAY (self->toast_overlay),
                                 adw_toast_new (_("Copied to clipboard")));
  }

  g_free (value);
}

static void
save_debug_info_dialog_cb (GtkFileDialog  *dialog,
                           GAsyncResult   *result,
                           AdwAboutDialog *self)
{
  GFile *file = gtk_file_dialog_save_finish (dialog, result, NULL);

  if (file) {
    GError *error = NULL;

    g_file_replace_contents (file,
                             self->debug_info,
                             strlen (self->debug_info),
                             NULL,
                             FALSE,
                             G_FILE_CREATE_NONE,
                             NULL,
                             NULL,
                             &error);

    if (error) {
      AdwDialog *alert = adw_alert_dialog_new (_("Unable to save debugging information"),
                                               NULL);
      adw_alert_dialog_format_body (ADW_ALERT_DIALOG (alert),
                                    "%s", error->message);
      adw_alert_dialog_add_response (ADW_ALERT_DIALOG (alert),
                                     "close", _("Close"));

      adw_dialog_present (alert, GTK_WIDGET (self));

      g_error_free (error);
    }

    g_object_unref (file);
  }
}

static void
save_debug_info_cb (AdwAboutDialog *self)
{
  GtkFileDialog *dialog = gtk_file_dialog_new ();
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  gtk_file_dialog_set_title (dialog, _("Save debugging information"));
  gtk_file_dialog_set_initial_name (dialog, self->debug_info_filename);

  gtk_file_dialog_save (dialog,
                        GTK_WINDOW (root),
                        NULL,
                        (GAsyncReadyCallback) save_debug_info_dialog_cb,
                        self);
}

static gboolean
save_debug_info_shortcut_cb (GtkWidget *widget,
                             GVariant  *args,
                             gpointer   user_data)
{
  AdwAboutDialog *self = ADW_ABOUT_DIALOG (widget);

  if (adw_navigation_view_get_visible_page (ADW_NAVIGATION_VIEW (self->navigation_view)) != self->debug_info_page)
    return GDK_EVENT_PROPAGATE;

  save_debug_info_cb (self);

  return GDK_EVENT_STOP;
}

static void
adw_about_dialog_class_init (AdwAboutDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_about_dialog_dispose;
  object_class->finalize = adw_about_dialog_finalize;
  object_class->get_property = adw_about_dialog_get_property;
  object_class->set_property = adw_about_dialog_set_property;

  /**
   * AdwAboutDialog:application-icon: (attributes org.gtk.Property.get=adw_about_dialog_get_application_icon org.gtk.Property.set=adw_about_dialog_set_application_icon)
   *
   * The name of the application icon.
   *
   * The icon is displayed at the top of the main page.
   *
   * Since: 1.5
   */
  props[PROP_APPLICATION_ICON] =
    g_param_spec_string ("application-icon", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:application-name: (attributes org.gtk.Property.get=adw_about_dialog_get_application_name org.gtk.Property.set=adw_about_dialog_set_application_name)
   *
   * The name of the application.
   *
   * The name is displayed at the top of the main page.
   *
   * Since: 1.5
   */
  props[PROP_APPLICATION_NAME] =
    g_param_spec_string ("application-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:developer-name: (attributes org.gtk.Property.get=adw_about_dialog_get_developer_name org.gtk.Property.set=adw_about_dialog_set_developer_name)
   *
   * The developer name.
   *
   * The developer name is displayed on the main page, under the application
   * name.
   *
   * If the application is developed by multiple people, the developer name can
   * be set to values like "AppName team", "AppName developers" or
   * "The AppName project", and the individual contributors can be listed on the
   * Credits page, with [property@AboutDialog:developers] and related
   * properties.
   *
   * Since: 1.5
   */
  props[PROP_DEVELOPER_NAME] =
    g_param_spec_string ("developer-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:version: (attributes org.gtk.Property.get=adw_about_dialog_get_version org.gtk.Property.set=adw_about_dialog_set_version)
   *
   * The version of the application.
   *
   * The version is displayed on the main page.
   *
   * If [property@AboutDialog:release-notes-version] is not set, the version
   * will also be displayed above the release notes on the What's New page.
   *
   * Since: 1.5
   */
  props[PROP_VERSION] =
    g_param_spec_string ("version", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:release-notes-version: (attributes org.gtk.Property.get=adw_about_dialog_get_release_notes_version org.gtk.Property.set=adw_about_dialog_set_release_notes_version)
   *
   * The version described by the application's release notes.
   *
   * The release notes version is displayed on the What's New page, above the
   * release notes.
   *
   * If not set, [property@AboutDialog:version] will be used instead.
   *
   * For example, an application with the current version 2.0.2 might want to
   * keep the release notes from 2.0.0, and set the release notes version
   * accordingly.
   *
   * See [property@AboutDialog:release-notes].
   *
   * Since: 1.5
   */
  props[PROP_RELEASE_NOTES_VERSION] =
    g_param_spec_string ("release-notes-version", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:release-notes: (attributes org.gtk.Property.get=adw_about_dialog_get_release_notes org.gtk.Property.set=adw_about_dialog_set_release_notes)
   *
   * The release notes of the application.
   *
   * Release notes are displayed on the the What's New page.
   *
   * Release notes are formatted the same way as
   * [AppStream descriptions](https://freedesktop.org/software/appstream/docs/chap-Metadata.html#tag-description).
   *
   * The supported formatting options are:
   *
   * * Paragraph (`<p>`)
   * * Ordered list (`<ol>`), with list items (`<li>`)
   * * Unordered list (`<ul>`), with list items (`<li>`)
   *
   * Within paragraphs and list items, emphasis (`<em>`) and inline code
   * (`<code>`) text styles are supported. The emphasis is rendered in italic,
   * while inline code is shown in a monospaced font.
   *
   * Any text outside paragraphs or list items is ignored.
   *
   * Nested lists are not supported.
   *
   * `AdwAboutDialog` displays the version above the release notes. If set, the
   * [property@AboutDialog:release-notes-version] of the property will be used
   * as the version; otherwise, [property@AboutDialog:version] is used.
   *
   * Since: 1.5
   */
  props[PROP_RELEASE_NOTES] =
    g_param_spec_string ("release-notes", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:comments: (attributes org.gtk.Property.get=adw_about_dialog_get_comments org.gtk.Property.set=adw_about_dialog_set_comments)
   *
   * The comments about the application.
   *
   * Comments will be shown on the Details page, above links.
   *
   * Unlike [property@Gtk.AboutDialog:comments], this string can be long and
   * detailed. It can also contain links and Pango markup.
   *
   * Since: 1.5
   */
  props[PROP_COMMENTS] =
    g_param_spec_string ("comments", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:website: (attributes org.gtk.Property.get=adw_about_dialog_get_website org.gtk.Property.set=adw_about_dialog_set_website)
   *
   * The URL of the application's website.
   *
   * Website is displayed on the Details page, below comments, or on the main
   * page if the Details page doesn't have any other content.
   *
   * Applications can add other links below, see [method@AboutDialog.add_link].
   *
   * Since: 1.5
   */
  props[PROP_WEBSITE] =
    g_param_spec_string ("website", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:support-url: (attributes org.gtk.Property.get=adw_about_dialog_get_support_url org.gtk.Property.set=adw_about_dialog_set_support_url)
   *
   * The URL of the application's support page.
   *
   * The support page link is displayed on the main page.
   *
   * Since: 1.5
   */
  props[PROP_SUPPORT_URL] =
    g_param_spec_string ("support-url", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:issue-url: (attributes org.gtk.Property.get=adw_about_dialog_get_issue_url org.gtk.Property.set=adw_about_dialog_set_issue_url)
   *
   * The URL for the application's issue tracker.
   *
   * The issue tracker link is displayed on the main page.
   *
   * Since: 1.5
   */
  props[PROP_ISSUE_URL] =
    g_param_spec_string ("issue-url", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:debug-info: (attributes org.gtk.Property.get=adw_about_dialog_get_debug_info org.gtk.Property.set=adw_about_dialog_set_debug_info)
   *
   * The debug information.
   *
   * Debug information will be shown on the Troubleshooting page. It's intended
   * to be attached to issue reports when reporting issues against the
   * application.
   *
   * `AdwAboutDialog` provides a quick way to save debug information to a file.
   * When saving, [property@AboutDialog:debug-info-filename] would be used as
   * the suggested filename.
   *
   * Debug information cannot contain markup or links.
   *
   * Since: 1.5
   */
  props[PROP_DEBUG_INFO] =
    g_param_spec_string ("debug-info", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:debug-info-filename: (attributes org.gtk.Property.get=adw_about_dialog_get_debug_info_filename org.gtk.Property.set=adw_about_dialog_set_debug_info_filename)
   *
   * The debug information filename.
   *
   * It will be used as the suggested filename when saving debug information to
   * a file.
   *
   * See [property@AboutDialog:debug-info].
   *
   * Since: 1.5
   */
  props[PROP_DEBUG_INFO_FILENAME] =
    g_param_spec_string ("debug-info-filename", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:developers: (attributes org.gtk.Property.get=adw_about_dialog_get_developers org.gtk.Property.set=adw_about_dialog_set_developers)
   *
   * The list of developers of the application.
   *
   * It will be displayed on the Credits page.
   *
   * Each name may contain email addresses and URLs, see the introduction for
   * more details.
   *
   * See also:
   *
   * * [property@AboutDialog:designers]
   * * [property@AboutDialog:artists]
   * * [property@AboutDialog:documenters]
   * * [property@AboutDialog:translator-credits]
   * * [method@AboutDialog.add_credit_section]
   * * [method@AboutDialog.add_acknowledgement_section]
   *
   * Since: 1.5
   */
  props[PROP_DEVELOPERS] =
      g_param_spec_boxed ("developers", NULL, NULL,
                          G_TYPE_STRV,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:designers: (attributes org.gtk.Property.get=adw_about_dialog_get_designers org.gtk.Property.set=adw_about_dialog_set_designers)
   *
   * The list of designers of the application.
   *
   * It will be displayed on the Credits page.
   *
   * Each name may contain email addresses and URLs, see the introduction for
   * more details.
   *
   * See also:
   *
   * * [property@AboutDialog:developers]
   * * [property@AboutDialog:artists]
   * * [property@AboutDialog:documenters]
   * * [property@AboutDialog:translator-credits]
   * * [method@AboutDialog.add_credit_section]
   * * [method@AboutDialog.add_acknowledgement_section]
   *
   * Since: 1.5
   */
  props[PROP_DESIGNERS] =
      g_param_spec_boxed ("designers", NULL, NULL,
                          G_TYPE_STRV,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:artists: (attributes org.gtk.Property.get=adw_about_dialog_get_artists org.gtk.Property.set=adw_about_dialog_set_artists)
   *
   * The list of artists of the application.
   *
   * It will be displayed on the Credits page.
   *
   * Each name may contain email addresses and URLs, see the introduction for
   * more details.
   *
   * See also:
   *
   * * [property@AboutDialog:developers]
   * * [property@AboutDialog:designers]
   * * [property@AboutDialog:documenters]
   * * [property@AboutDialog:translator-credits]
   * * [method@AboutDialog.add_credit_section]
   * * [method@AboutDialog.add_acknowledgement_section]
   *
   * Since: 1.5
   */
  props[PROP_ARTISTS] =
      g_param_spec_boxed ("artists", NULL, NULL,
                          G_TYPE_STRV,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:documenters: (attributes org.gtk.Property.get=adw_about_dialog_get_documenters org.gtk.Property.set=adw_about_dialog_set_documenters)
   *
   * The list of documenters of the application.
   *
   * It will be displayed on the Credits page.
   *
   * Each name may contain email addresses and URLs, see the introduction for
   * more details.
   *
   * See also:
   *
   * * [property@AboutDialog:developers]
   * * [property@AboutDialog:designers]
   * * [property@AboutDialog:artists]
   * * [property@AboutDialog:translator-credits]
   * * [method@AboutDialog.add_credit_section]
   * * [method@AboutDialog.add_acknowledgement_section]
   *
   * Since: 1.5
   */
  props[PROP_DOCUMENTERS] =
      g_param_spec_boxed ("documenters", NULL, NULL,
                          G_TYPE_STRV,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:translator-credits: (attributes org.gtk.Property.get=adw_about_dialog_get_translator_credits org.gtk.Property.set=adw_about_dialog_set_translator_credits)
   *
   * The translator credits string.
   *
   * It will be displayed on the Credits page.
   *
   * This string should be `"translator-credits"` or `"translator_credits"` and
   * should be marked as translatable.
   *
   * The string may contain email addresses and URLs, see the introduction for
   * more details.
   *
   * See also:
   *
   * * [property@AboutDialog:developers]
   * * [property@AboutDialog:designers]
   * * [property@AboutDialog:artists]
   * * [property@AboutDialog:documenters]
   * * [method@AboutDialog.add_credit_section]
   * * [method@AboutDialog.add_acknowledgement_section]
   *
   * Since: 1.5
   */
  props[PROP_TRANSLATOR_CREDITS] =
      g_param_spec_string ("translator-credits", NULL, NULL,
                           "",
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:copyright: (attributes org.gtk.Property.get=adw_about_dialog_get_copyright org.gtk.Property.set=adw_about_dialog_set_copyright)
   *
   * The copyright information.
   *
   * This should be a short string of one or two lines, for example:
   * `© 2022 Example`.
   *
   * The copyright information will be displayed on the Legal page, above the
   * application license.
   *
   * [method@AboutDialog.add_legal_section] can be used to add copyright
   * information for the application dependencies or other components.
   *
   * Since: 1.5
   */
  props[PROP_COPYRIGHT] =
    g_param_spec_string ("copyright", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:license-type: (attributes org.gtk.Property.get=adw_about_dialog_get_license_type org.gtk.Property.set=adw_about_dialog_set_license_type)
   *
   * The license type.
   *
   * Allows to set the application's license froma list of known licenses.
   *
   * If the application's license is not in the list,
   * [property@AboutDialog:license] can be used instead. The license type will
   * be automatically set to `GTK_LICENSE_CUSTOM` in that case.
   *
   * If set to `GTK_LICENSE_UNKNOWN`, no information will be displayed.
   *
   * If the license type is different from `GTK_LICENSE_CUSTOM`.
   * [property@AboutDialog:license] will be cleared out.
   *
   * The license description will be displayed on the Legal page, below the
   * copyright information.
   *
   * [method@AboutDialog.add_legal_section] can be used to add license
   * information for the application dependencies or other components.
   *
   * Since: 1.5
   */
  props[PROP_LICENSE_TYPE] =
    g_param_spec_enum ("license-type", NULL, NULL,
                       GTK_TYPE_LICENSE,
                       GTK_LICENSE_UNKNOWN,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAboutDialog:license: (attributes org.gtk.Property.get=adw_about_dialog_get_license org.gtk.Property.set=adw_about_dialog_set_license)
   *
   * The license text.
   *
   * This can be used to set a custom text for the license if it can't be set
   * via [property@AboutDialog:license-type].
   *
   * When set, [property@AboutDialog:license-type] will be set to
   * `GTK_LICENSE_CUSTOM`.
   *
   * The license text will be displayed on the Legal page, below the copyright
   * information.
   *
   * License text can contain Pango markup and links.
   *
   * [method@AboutDialog.add_legal_section] can be used to add license
   * information for the application dependencies or other components.
   *
   * Since: 1.5
   */
  props[PROP_LICENSE] =
    g_param_spec_string ("license", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwAboutDialog::activate-link:
   * @self: an about dialog
   * @uri: the URI to activate
   *
   * Emitted when a URL is activated.
   *
   * Applications may connect to it to override the default behavior, which is
   * to call [func@Gtk.show_uri].
   *
   * Returns: `TRUE` if the link has been activated
   *
   * Since: 1.5
   */
  signals[SIGNAL_ACTIVATE_LINK] =
    g_signal_new ("activate-link",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_true_handled,
                  NULL,
                  adw_marshal_BOOLEAN__STRING,
                  G_TYPE_BOOLEAN,
                  1,
                  G_TYPE_STRING);
  g_signal_set_va_marshaller (signals[SIGNAL_ACTIVATE_LINK],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_BOOLEAN__STRINGv);

  g_signal_override_class_handler ("activate-link",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (activate_link_default_cb));

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-about-dialog.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, navigation_view);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, toast_overlay);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, main_scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, main_headerbar);

  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, app_icon_image);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, app_name_label);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, developer_name_label);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, version_button);

  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, details_group);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, whats_new_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, comments_label);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, website_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, links_group);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, details_website_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, details_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, release_notes_buffer);

  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, support_group);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, support_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, issue_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, troubleshooting_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, debug_info_page);

  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, credits_legal_group);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, credits_box);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, legal_box);
  gtk_widget_class_bind_template_child (widget_class, AdwAboutDialog, acknowledgements_box);

  gtk_widget_class_bind_template_callback (widget_class, activate_link_cb);
  gtk_widget_class_bind_template_callback (widget_class, legal_showing_cb);

  gtk_widget_class_install_action (widget_class, "about.show-url", "s",
                                   (GtkWidgetActionActivateFunc) show_url_cb);
  gtk_widget_class_install_action (widget_class, "about.show-url-property", "s",
                                   (GtkWidgetActionActivateFunc) show_url_property_cb);
  gtk_widget_class_install_action (widget_class, "about.copy-property", "s",
                                   (GtkWidgetActionActivateFunc) copy_property_cb);
  gtk_widget_class_install_action (widget_class, "about.save-debug-info", NULL,
                                   (GtkWidgetActionActivateFunc) save_debug_info_cb);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_S, GDK_CONTROL_MASK,
                                save_debug_info_shortcut_cb, NULL);
}

static void
adw_about_dialog_init (AdwAboutDialog *self)
{
  GtkAdjustment *adj;
  self->application_icon = g_strdup ("");
  self->application_name = g_strdup ("");
  self->developer_name = g_strdup ("");
  self->version = g_strdup ("");
  self->release_notes_version = g_strdup ("");
  self->release_notes = g_strdup ("");
  self->comments = g_strdup ("");
  self->website = g_strdup ("");
  self->support_url = g_strdup ("");
  self->issue_url = g_strdup ("");
  self->debug_info = g_strdup ("");
  self->debug_info_filename = g_strdup ("");
  self->copyright = g_strdup ("");
  self->license = g_strdup ("");
  self->translator_credits = g_strdup ("");

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_text_buffer_create_tag (self->release_notes_buffer, "em",
                              "style", PANGO_STYLE_ITALIC,
                              NULL);
  gtk_text_buffer_create_tag (self->release_notes_buffer, "code",
                              "family", "monospace",
                              NULL);
  gtk_text_buffer_create_tag (self->release_notes_buffer, "bullet",
                              "font-features", "tnum=1",
                              "left-margin", 24,
                              "pixels-above-lines", 6,
                              NULL);
  gtk_text_buffer_create_tag (self->release_notes_buffer, "section",
                              "pixels-above-lines", 12,
                              NULL);
  gtk_text_buffer_create_tag (self->release_notes_buffer, "heading",
                              "weight", PANGO_WEIGHT_BOLD,
                              NULL);

  adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (self->main_scrolled_window));

  g_signal_connect_swapped (adj, "value-changed", G_CALLBACK (update_headerbar_cb), self);

  update_headerbar_cb (self);
}

/**
 * adw_about_dialog_new:
 *
 * Creates a new `AdwAboutDialog`.
 *
 * Returns: the newly created `AdwAboutDialog`
 *
 * Since: 1.5
 */
AdwDialog *
adw_about_dialog_new (void)
{
  return g_object_new (ADW_TYPE_ABOUT_DIALOG, NULL);
}

/**
 * adw_about_dialog_new_from_appdata:
 * @resource_path: The resource to use
 * @release_notes_version: (nullable): The version to retrieve release notes for
 *
 * Creates a new `AdwAboutDialog` using AppStream metadata.
 *
 * This automatically sets the following properties with the following AppStream
 * values:
 *
 * * [property@AboutDialog:application-icon] is set from the `<id>`
 * * [property@AboutDialog:application-name] is set from the `<name>`
 * * [property@AboutDialog:developer-name] is set from the `<name>` within
 *      `<developer>`
 * * [property@AboutDialog:version] is set from the version of the latest release
 * * [property@AboutDialog:website] is set from the `<url type="homepage">`
 * * [property@AboutDialog:support-url] is set from the `<url type="help">`
 * * [property@AboutDialog:issue-url] is set from the `<url type="bugtracker">`
 * * [property@AboutDialog:license-type] is set from the `<project_license>`.
 *     If the license type retrieved from AppStream is not listed in
 *     [enum@Gtk.License], it will be set to `GTK_LICENCE_CUSTOM`.
 *
 * If @release_notes_version is not `NULL`,
 * [property@AboutDialog:release-notes-version] is set to match it, while
 * [property@AboutDialog:release-notes] is set from the AppStream release
 * description for that version.
 *
 * Returns: the newly created `AdwAboutDialog`
 *
 * Since: 1.5
 */
AdwDialog *
adw_about_dialog_new_from_appdata (const char *resource_path,
                                   const char *release_notes_version)
{
  AdwAboutDialog *self;
  GFile *appdata_file;
  char *appdata_uri;
  AsMetadata *metadata;
  GPtrArray *releases;
  AsComponent *component;
  char *application_id;
  const char *name, *developer_name, *project_license;
  const char *issue_url, *support_url, *website_url;
  GError *error = NULL;

  g_return_val_if_fail (resource_path, NULL);

  appdata_uri = g_strconcat ("resource://", resource_path, NULL);
  appdata_file = g_file_new_for_uri (appdata_uri);

  self = ADW_ABOUT_DIALOG (adw_about_dialog_new ());
  metadata = as_metadata_new ();

  if (!as_metadata_parse_file (metadata, appdata_file, AS_FORMAT_KIND_UNKNOWN, &error)) {
    g_error ("Could not parse metadata file: %s", error->message);
    g_clear_error (&error);
  }

  component = as_metadata_get_component (metadata);

  if (component == NULL)
    g_error ("Could not find valid AppStream metadata");

  application_id = g_strdup (as_component_get_id (component));

  if (g_str_has_suffix (application_id, ".desktop")) {
    AsLaunchable *launchable;
    char *appid_desktop;
    GPtrArray *entries = NULL;

    launchable = as_component_get_launchable (component,
                                              AS_LAUNCHABLE_KIND_DESKTOP_ID);

    if (launchable)
      entries = as_launchable_get_entries (launchable);

    appid_desktop = g_strconcat (application_id, ".desktop", NULL);

    if (!entries || !g_ptr_array_find_with_equal_func (entries, appid_desktop,
                                                       g_str_equal, NULL))
      application_id[strlen(application_id) - 8] = '\0';

    g_free (appid_desktop);
  }

#if AS_CHECK_VERSION (1, 0, 0)
  releases = as_release_list_get_entries (as_component_get_releases_plain (component));
#else
  releases = as_component_get_releases (component);
#endif

  if (release_notes_version) {
    guint release_index = 0;

    if (g_ptr_array_find_with_equal_func (releases, release_notes_version,
                                         (GEqualFunc) get_release_for_version,
                                         &release_index)) {
      AsRelease *notes_release;
      const char *release_notes, *version;

      notes_release = g_ptr_array_index (releases, release_index);

      release_notes = as_release_get_description (notes_release);
      version = as_release_get_version (notes_release);

      if (release_notes && version) {
        adw_about_dialog_set_release_notes (self, release_notes);
        adw_about_dialog_set_release_notes_version (self, version);
      }
    } else {
      g_critical ("No valid release found for version %s", release_notes_version);
    }
  }

  if (releases->len > 0) {
    AsRelease *latest_release = g_ptr_array_index (releases, 0);
    const char *version = as_release_get_version (latest_release);

    if (version)
      adw_about_dialog_set_version (self, version);
  }

  name = as_component_get_name (component);
  project_license = as_component_get_project_license (component);
  issue_url = as_component_get_url (component, AS_URL_KIND_BUGTRACKER);
  support_url = as_component_get_url (component, AS_URL_KIND_HELP);
  website_url = as_component_get_url (component, AS_URL_KIND_HOMEPAGE);

#if AS_CHECK_VERSION (0, 16, 4)
  developer_name = as_developer_get_name (as_component_get_developer (component));
#else
  developer_name = as_component_get_developer_name (component);
#endif

  adw_about_dialog_set_application_icon (self, application_id);

  if (name)
    adw_about_dialog_set_application_name (self, name);

  if (developer_name)
    adw_about_dialog_set_developer_name (self, developer_name);

  if (project_license) {
    int i;

    for (i = 0; i < G_N_ELEMENTS (gtk_license_info); i++) {
      if (g_strcmp0 (gtk_license_info[i].spdx_id, project_license) == 0) {
        adw_about_dialog_set_license_type (self, (GtkLicense) i);
        break;
      }
    }

    /* Handle deprecated SPDX IDs */
    for (i = 0; i < G_N_ELEMENTS (license_aliases); i++) {
      if (g_strcmp0 (license_aliases[i].spdx_id, project_license) == 0) {
        adw_about_dialog_set_license_type (self, license_aliases[i].license);
        break;
      }
    }

    if (adw_about_dialog_get_license_type (self) == GTK_LICENSE_UNKNOWN)
      adw_about_dialog_set_license_type (self, GTK_LICENSE_CUSTOM);
  }

  if (issue_url)
    adw_about_dialog_set_issue_url (self, issue_url);

  if (support_url)
    adw_about_dialog_set_support_url (self, support_url);

  if (website_url)
    adw_about_dialog_set_website (self, website_url);

  g_object_unref (appdata_file);
  g_object_unref (metadata);
  g_free (application_id);
  g_free (appdata_uri);

  return ADW_DIALOG (self);
}

/**
 * adw_about_dialog_get_application_icon: (attributes org.gtk.Method.get_property=application-icon)
 * @self: an about dialog
 *
 * Gets the name of the application icon for @self.
 *
 * Returns: the application icon name
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_application_icon (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->application_icon;
}

/**
 * adw_about_dialog_set_application_icon: (attributes org.gtk.Method.set_property=application-icon)
 * @self: an about dialog
 * @application_icon: the application icon name
 *
 * Sets the name of the application icon for @self.
 *
 * The icon is displayed at the top of the main page.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_application_icon (AdwAboutDialog *self,
                                       const char     *application_icon)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (application_icon != NULL);

  if (!g_set_str (&self->application_icon, application_icon))
    return;

  gtk_widget_set_visible (self->app_icon_image,
                          application_icon && *application_icon);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_APPLICATION_ICON]);
}

/**
 * adw_about_dialog_get_application_name: (attributes org.gtk.Method.get_property=application-name)
 * @self: an about dialog
 *
 * Gets the application name for @self.
 *
 * Returns: the application name
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_application_name (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->application_name;
}

/**
 * adw_about_dialog_set_application_name: (attributes org.gtk.Method.set_property=application-name)
 * @self: an about dialog
 * @application_name: the application name
 *
 * Sets the application name for @self.
 *
 * The name is displayed at the top of the main page.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_application_name (AdwAboutDialog *self,
                                       const char     *application_name)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (application_name != NULL);

  if (!g_set_str (&self->application_name, application_name))
    return;

  gtk_widget_set_visible (self->app_name_label,
                          application_name && *application_name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_APPLICATION_NAME]);
}

/**
 * adw_about_dialog_get_developer_name: (attributes org.gtk.Method.get_property=developer-name)
 * @self: an about dialog
 *
 * Gets the developer name for @self.
 *
 * Returns: the developer_name
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_developer_name (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->developer_name;
}

/**
 * adw_about_dialog_set_developer_name: (attributes org.gtk.Method.set_property=developer-name)
 * @self: an about dialog
 * @developer_name: the developer name
 *
 * Sets the developer name for @self.
 *
 * The developer name is displayed on the main page, under the application name.
 *
 * If the application is developed by multiple people, the developer name can be
 * set to values like "AppName team", "AppName developers" or
 * "The AppName project", and the individual contributors can be listed on the
 * Credits page, with [property@AboutDialog:developers] and related properties.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_developer_name (AdwAboutDialog *self,
                                     const char     *developer_name)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (developer_name != NULL);

  if (!g_set_str (&self->developer_name, developer_name))
    return;

  gtk_widget_set_visible (self->developer_name_label,
                          developer_name && *developer_name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEVELOPER_NAME]);
}

/**
 * adw_about_dialog_get_version: (attributes org.gtk.Method.get_property=version)
 * @self: an about dialog
 *
 * Gets the version for @self.
 *
 * Returns: the version
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_version (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->version;
}

/**
 * adw_about_dialog_set_version: (attributes org.gtk.Method.set_property=version)
 * @self: an about dialog
 * @version: the version
 *
 * Sets the version for @self.
 *
 * The version is displayed on the main page.
 *
 * If [property@AboutDialog:release-notes-version] is not set, the version will
 * also be displayed above the release notes on the What's New page.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_version (AdwAboutDialog *self,
                              const char     *version)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (version != NULL);

  if (!g_set_str (&self->version, version))
    return;

  gtk_widget_set_visible (self->version_button, version && *version);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VERSION]);
}

/**
 * adw_about_dialog_get_release_notes_version: (attributes org.gtk.Method.get_property=release-notes-version)
 * @self: an about dialog
 *
 * Gets the version described by the application's release notes.
 *
 * Returns: the release notes version
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_release_notes_version (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->release_notes_version;
}

/**
 * adw_about_dialog_set_release_notes_version: (attributes org.gtk.Method.set_property=release-notes-version)
 * @self: an about dialog
 * @version: the release notes version
 *
 * Sets the version described by the application's release notes.
 *
 * The release notes version is displayed on the What's New page, above the
 * release notes.
 *
 * If not set, [property@AboutDialog:version] will be used instead.
 *
 * For example, an application with the current version 2.0.2 might want to
 * keep the release notes from 2.0.0, and set the release notes version
 * accordingly.
 *
 * See [property@AboutDialog:release-notes].
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_release_notes_version (AdwAboutDialog *self,
                                            const char     *version)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (version != NULL);

  if (!g_set_str (&self->release_notes_version, version))
    return;

  update_release_notes (self);
  update_details (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_RELEASE_NOTES_VERSION]);
}

/**
 * adw_about_dialog_get_release_notes: (attributes org.gtk.Method.get_property=release-notes)
 * @self: an about dialog
 *
 * Gets the release notes for @self.
 *
 * Returns: the release notes
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_release_notes (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->release_notes;
}

/**
 * adw_about_dialog_set_release_notes: (attributes org.gtk.Method.set_property=release-notes)
 * @self: an about dialog
 * @release_notes: the release notes
 *
 * Sets the release notes for @self.
 *
 * Release notes are displayed on the the What's New page.
 *
 * Release notes are formatted the same way as
 * [AppStream descriptions](https://freedesktop.org/software/appstream/docs/chap-Metadata.html#tag-description).
 *
 * The supported formatting options are:
 *
 * * Paragraph (`<p>`)
 * * Ordered list (`<ol>`), with list items (`<li>`)
 * * Unordered list (`<ul>`), with list items (`<li>`)
 *
 * Within paragraphs and list items, emphasis (`<em>`) and inline code
 * (`<code>`) text styles are supported. The emphasis is rendered in italic,
 * while inline code is shown in a monospaced font.
 *
 * Any text outside paragraphs or list items is ignored.
 *
 * Nested lists are not supported.
 *
 * `AdwAboutDialog` displays the version above the release notes. If set, the
 * [property@AboutDialog:release-notes-version] of the property will be used
 * as the version; otherwise, [property@AboutDialog:version] is used.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_release_notes (AdwAboutDialog *self,
                                    const char     *release_notes)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (release_notes != NULL);

  if (!g_set_str (&self->release_notes, release_notes))
    return;

  update_release_notes (self);
  update_details (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_RELEASE_NOTES]);
}

/**
 * adw_about_dialog_get_comments: (attributes org.gtk.Method.get_property=comments)
 * @self: an about dialog
 *
 * Gets the comments about the application.
 *
 * Returns: the comments
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_comments (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->comments;
}

/**
 * adw_about_dialog_set_comments: (attributes org.gtk.Method.set_property=comments)
 * @self: an about dialog
 * @comments: the comments
 *
 * Sets the comments about the application.
 *
 * Comments will be shown on the Details page, above links.
 *
 * Unlike [property@Gtk.AboutDialog:comments], this string can be long and
 * detailed. It can also contain links and Pango markup.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_comments (AdwAboutDialog *self,
                               const char     *comments)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (comments != NULL);

  if (!g_set_str (&self->comments, comments))
    return;

  update_details (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COMMENTS]);
}

/**
 * adw_about_dialog_get_website: (attributes org.gtk.Method.get_property=website)
 * @self: an about dialog
 *
 * Gets the application website URL for @self.
 *
 * Returns: the website URL
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_website (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->website;
}

/**
 * adw_about_dialog_set_website: (attributes org.gtk.Method.set_property=website)
 * @self: an about dialog
 * @website: the website URL
 *
 * Sets the application website URL for @self.
 *
 * Website is displayed on the Details page, below comments, or on the main page
 * if the Details page doesn't have any other content.
 *
 * Applications can add other links below, see [method@AboutDialog.add_link].
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_website (AdwAboutDialog *self,
                              const char     *website)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (website != NULL);

  if (!g_set_str (&self->website, website))
    return;

  update_details (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WEBSITE]);
}

/**
 * adw_about_dialog_get_support_url: (attributes org.gtk.Method.get_property=support-url)
 * @self: an about dialog
 *
 * Gets the URL of the support page for @self.
 *
 * Returns: the support page URL
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_support_url (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->support_url;
}

/**
 * adw_about_dialog_set_support_url: (attributes org.gtk.Method.set_property=support-url)
 * @self: an about dialog
 * @support_url: the support page URL
 *
 * Sets the URL of the support page for @self.
 *
 * The support page link is displayed on the main page.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_support_url (AdwAboutDialog *self,
                                  const char     *support_url)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (support_url != NULL);

  if (!g_set_str (&self->support_url, support_url))
    return;

  update_support (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUPPORT_URL]);
}

/**
 * adw_about_dialog_get_issue_url: (attributes org.gtk.Method.get_property=issue-url)
 * @self: an about dialog
 *
 * Gets the issue tracker URL for @self.
 *
 * Returns: the issue tracker URL
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_issue_url (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->issue_url;
}

/**
 * adw_about_dialog_set_issue_url: (attributes org.gtk.Method.set_property=issue-url)
 * @self: an about dialog
 * @issue_url: the issue tracker URL
 *
 * Sets the issue tracker URL for @self.
 *
 * The issue tracker link is displayed on the main page.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_issue_url (AdwAboutDialog *self,
                                const char     *issue_url)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (issue_url != NULL);

  if (!g_set_str (&self->issue_url, issue_url))
    return;

  update_support (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ISSUE_URL]);
}

/**
 * adw_about_dialog_add_link:
 * @self: an about dialog
 * @title: the link title
 * @url: the link URL
 *
 * Adds an extra link to the Details page.
 *
 * Extra links are displayed under the comment and website.
 *
 * Underlines in @title will be interpreted as indicating a mnemonic.
 *
 * See [property@AboutDialog:website].
 *
 * Since: 1.5
 */
void
adw_about_dialog_add_link (AdwAboutDialog *self,
                           const char     *title,
                           const char     *url)
{
  GtkWidget *row;
  GtkWidget *image;

  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (title != NULL);
  g_return_if_fail (url != NULL);

  row = adw_action_row_new ();
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (row), title);
  adw_preferences_row_set_use_underline (ADW_PREFERENCES_ROW (row), TRUE);

  image = g_object_new (GTK_TYPE_IMAGE,
                        "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                        "icon-name", "adw-external-link-symbolic",
                        NULL);
  adw_action_row_add_suffix (ADW_ACTION_ROW (row), image);

  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), TRUE);
  gtk_actionable_set_action_name (GTK_ACTIONABLE (row), "about.show-url");
  gtk_actionable_set_action_target (GTK_ACTIONABLE (row), "s", url);

  gtk_widget_set_tooltip_text (row, url);

  adw_preferences_group_add (ADW_PREFERENCES_GROUP (self->links_group), row);

  self->has_custom_links = TRUE;

  update_details (self);
}

/**
 * adw_about_dialog_get_debug_info: (attributes org.gtk.Method.get_property=debug-info)
 * @self: an about dialog
 *
 * Gets the debug information for @self.
 *
 * Returns: the debug information
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_debug_info (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->debug_info;
}

/**
 * adw_about_dialog_set_debug_info: (attributes org.gtk.Method.set_property=debug-info)
 * @self: an about dialog
 * @debug_info: the debug information
 *
 * Sets the debug information for @self.
 *
 * Debug information will be shown on the Troubleshooting page. It's intended
 * to be attached to issue reports when reporting issues against the
 * application.
 *
 * `AdwAboutDialog` provides a quick way to save debug information to a file.
 * When saving, [property@AboutDialog:debug-info-filename] would be used as
 * the suggested filename.
 *
 * Debug information cannot contain markup or links.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_debug_info (AdwAboutDialog *self,
                                 const char     *debug_info)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (debug_info != NULL);

  if (!g_set_str (&self->debug_info, debug_info))
    return;

  update_support (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEBUG_INFO]);
}

/**
 * adw_about_dialog_get_debug_info_filename: (attributes org.gtk.Method.get_property=debug-info-filename)
 * @self: an about dialog
 *
 * Gets the debug information filename for @self.
 *
 * Returns: the debug information filename
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_debug_info_filename (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->debug_info_filename;
}

/**
 * adw_about_dialog_set_debug_info_filename: (attributes org.gtk.Method.set_property=debug-info)
 * @self: an about dialog
 * @filename: the debug info filename
 *
 * Sets the debug information filename for @self.
 *
 * It will be used as the suggested filename when saving debug information to a
 * file.
 *
 * See [property@AboutDialog:debug-info].
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_debug_info_filename (AdwAboutDialog *self,
                                          const char     *filename)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (filename != NULL);

  if (!g_set_str (&self->debug_info_filename, filename))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEBUG_INFO_FILENAME]);
}

/**
 * adw_about_dialog_get_developers: (attributes org.gtk.Method.get_property=developers)
 * @self: an about dialog
 *
 * Gets the list of developers of the application.
 *
 * Returns: (nullable) (transfer none) (array zero-terminated=1): The list of developers
 *
 * Since: 1.5
 */
const char * const *
adw_about_dialog_get_developers (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return (const char * const *) self->developers;
}

/**
 * adw_about_dialog_set_developers: (attributes org.gtk.Method.set_property=developers)
 * @self: an about dialog
 * @developers: (nullable) (transfer none) (array zero-terminated=1): the list of developers
 *
 * Sets the list of developers of the application.
 *
 * It will be displayed on the Credits page.
 *
 * Each name may contain email addresses and URLs, see the introduction for more
 * details.
 *
 * See also:
 *
 * * [property@AboutDialog:designers]
 * * [property@AboutDialog:artists]
 * * [property@AboutDialog:documenters]
 * * [property@AboutDialog:translator-credits]
 * * [method@AboutDialog.add_credit_section]
 * * [method@AboutDialog.add_acknowledgement_section]
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_developers (AdwAboutDialog  *self,
                                 const char     **developers)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));

  if ((const char **) self->developers == developers)
    return;

  g_strfreev (self->developers);
  self->developers = g_strdupv ((char **) developers);

  update_credits (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEVELOPERS]);
}

/**
 * adw_about_dialog_get_designers: (attributes org.gtk.Method.get_property=designers)
 * @self: an about dialog
 *
 * Gets the list of designers of the application.
 *
 * Returns: (nullable) (transfer none) (array zero-terminated=1): The list of designers
 *
 * Since: 1.5
 */
const char * const *
adw_about_dialog_get_designers (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return (const char * const *) self->designers;
}

/**
 * adw_about_dialog_set_designers: (attributes org.gtk.Method.set_property=designers)
 * @self: an about dialog
 * @designers: (nullable) (transfer none) (array zero-terminated=1): the list of designers
 *
 * Sets the list of designers of the application.
 *
 * It will be displayed on the Credits page.
 *
 * Each name may contain email addresses and URLs, see the introduction for more
 * details.
 *
 * See also:
 *
 * * [property@AboutDialog:developers]
 * * [property@AboutDialog:artists]
 * * [property@AboutDialog:documenters]
 * * [property@AboutDialog:translator-credits]
 * * [method@AboutDialog.add_credit_section]
 * * [method@AboutDialog.add_acknowledgement_section]
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_designers (AdwAboutDialog  *self,
                                const char     **designers)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));

  if ((const char **) self->designers == designers)
    return;

  g_strfreev (self->designers);
  self->designers = g_strdupv ((char **) designers);

  update_credits (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESIGNERS]);
}

/**
 * adw_about_dialog_get_artists: (attributes org.gtk.Method.get_property=artists)
 * @self: an about dialog
 *
 * Gets the list of artists of the application.
 *
 * Returns: (nullable) (transfer none) (array zero-terminated=1): The list of artists
 *
 * Since: 1.5
 */
const char * const *
adw_about_dialog_get_artists (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return (const char * const *) self->artists;
}

/**
 * adw_about_dialog_set_artists: (attributes org.gtk.Method.set_property=artists)
 * @self: an about dialog
 * @artists: (nullable) (transfer none) (array zero-terminated=1): the list of artists
 *
 * Sets the list of artists of the application.
 *
 * It will be displayed on the Credits page.
 *
 * Each name may contain email addresses and URLs, see the introduction for more
 * details.
 *
 * See also:
 *
 * * [property@AboutDialog:developers]
 * * [property@AboutDialog:designers]
 * * [property@AboutDialog:documenters]
 * * [property@AboutDialog:translator-credits]
 * * [method@AboutDialog.add_credit_section]
 * * [method@AboutDialog.add_acknowledgement_section]
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_artists (AdwAboutDialog  *self,
                              const char     **artists)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));

  if ((const char **) self->artists == artists)
    return;

  g_strfreev (self->artists);
  self->artists = g_strdupv ((char **) artists);

  update_credits (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ARTISTS]);
}

/**
 * adw_about_dialog_get_documenters: (attributes org.gtk.Method.get_property=documenters)
 * @self: an about dialog
 *
 * Gets the list of documenters of the application.
 *
 * Returns: (nullable) (transfer none) (array zero-terminated=1): The list of documenters
 *
 * Since: 1.5
 */
const char * const *
adw_about_dialog_get_documenters (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return (const char * const *) self->documenters;
}

/**
 * adw_about_dialog_set_documenters: (attributes org.gtk.Method.set_property=documenters)
 * @self: an about dialog
 * @documenters: (nullable) (transfer none) (array zero-terminated=1): the list of documenters
 *
 * Sets the list of documenters of the application.
 *
 * It will be displayed on the Credits page.
 *
 * Each name may contain email addresses and URLs, see the introduction for more
 * details.
 *
 * See also:
 *
 * * [property@AboutDialog:developers]
 * * [property@AboutDialog:designers]
 * * [property@AboutDialog:artists]
 * * [property@AboutDialog:translator-credits]
 * * [method@AboutDialog.add_credit_section]
 * * [method@AboutDialog.add_acknowledgement_section]
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_documenters (AdwAboutDialog  *self,
                                  const char     **documenters)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));

  if ((const char **) self->documenters == documenters)
    return;

  g_strfreev (self->documenters);
  self->documenters = g_strdupv ((char **) documenters);

  update_credits (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DOCUMENTERS]);
}

/**
 * adw_about_dialog_get_translator_credits: (attributes org.gtk.Method.get_property=translator-credits)
 * @self: an about dialog
 *
 * Gets the translator credits string.
 *
 * Returns: The translator credits string
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_translator_credits (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->translator_credits;
}

/**
 * adw_about_dialog_set_translator_credits: (attributes org.gtk.Method.set_property=translator-credits)
 * @self: an about dialog
 * @translator_credits: the translator credits
 *
 * Sets the translator credits string.
 *
 * It will be displayed on the Credits page.
 *
 * This string should be `"translator-credits"` or `"translator_credits"` and
 * should be marked as translatable.
 *
 * The string may contain email addresses and URLs, see the introduction for
 * more details.
 *
 * See also:
 *
 * * [property@AboutDialog:developers]
 * * [property@AboutDialog:designers]
 * * [property@AboutDialog:artists]
 * * [property@AboutDialog:documenters]
 * * [method@AboutDialog.add_credit_section]
 * * [method@AboutDialog.add_acknowledgement_section]
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_translator_credits (AdwAboutDialog *self,
                                         const char     *translator_credits)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (translator_credits != NULL);

  if (!g_set_str (&self->translator_credits, translator_credits))
    return;

  update_credits (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSLATOR_CREDITS]);
}

/**
 * adw_about_dialog_add_credit_section:
 * @self: an about dialog
 * @name: (nullable): the section name
 * @people: (array zero-terminated=1): the list of names
 *
 * Adds an extra section to the Credits page.
 *
 * Extra sections are displayed below the standard categories.
 *
 * Each name may contain email addresses and URLs, see the introduction for more
 * details.
 *
 * See also:
 *
 * * [property@AboutDialog:developers]
 * * [property@AboutDialog:designers]
 * * [property@AboutDialog:artists]
 * * [property@AboutDialog:documenters]
 * * [property@AboutDialog:translator-credits]
 * * [method@AboutDialog.add_acknowledgement_section]
 *
 * Since: 1.5
 */
void
adw_about_dialog_add_credit_section (AdwAboutDialog  *self,
                                     const char      *name,
                                     const char     **people)
{
  CreditsSection *section;

  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (people != NULL);

  section = g_new0 (CreditsSection, 1);
  section->name = g_strdup (name);
  section->people = g_strdupv ((char **) people);

  self->credit_sections = g_slist_append (self->credit_sections, section);

  update_credits (self);
}

/**
 * adw_about_dialog_add_acknowledgement_section:
 * @self: an about dialog
 * @name: (nullable): the section name
 * @people: (array zero-terminated=1): the list of names
 *
 * Adds a section to the Acknowledgements page.
 *
 * This can be used to acknowledge additional people and organizations for their
 * non-development contributions - for example, backers in a crowdfunded
 * project.
 *
 * Each name may contain email addresses and URLs, see the introduction for more
 * details.
 *
 * See also:
 *
 * * [property@AboutDialog:developers]
 * * [property@AboutDialog:designers]
 * * [property@AboutDialog:artists]
 * * [property@AboutDialog:documenters]
 * * [property@AboutDialog:translator-credits]
 * * [method@AboutDialog.add_credit_section]
 *
 * Since: 1.5
 */
void
adw_about_dialog_add_acknowledgement_section (AdwAboutDialog  *self,
                                              const char      *name,
                                              const char     **people)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (people != NULL);

  add_credits_section (self->acknowledgements_box, name, (char **) people);

  gtk_widget_set_visible (self->acknowledgements_box, TRUE);

  update_credits_legal_group (self);
}

/**
 * adw_about_dialog_get_copyright: (attributes org.gtk.Method.get_property=copyright)
 * @self: an about dialog
 *
 * Gets the copyright information for @self.
 *
 * Returns: the copyright information
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_copyright (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->copyright;
}

/**
 * adw_about_dialog_set_copyright: (attributes org.gtk.Method.set_property=copyright)
 * @self: an about dialog
 * @copyright: the copyright information
 *
 * Sets the copyright information for @self.
 *
 * This should be a short string of one or two lines, for example:
 * `© 2022 Example`.
 *
 * The copyright information will be displayed on the Legal page, before the
 * application license.
 *
 * [method@AboutDialog.add_legal_section] can be used to add copyright
 * information for the application dependencies or other components.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_copyright (AdwAboutDialog *self,
                                const char     *copyright)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (copyright != NULL);

  if (!g_set_str (&self->copyright, copyright))
    return;

  update_legal (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COPYRIGHT]);
}

/**
 * adw_about_dialog_get_license_type: (attributes org.gtk.Method.get_property=license-type)
 * @self: an about dialog
 *
 * Gets the license type for @self.
 *
 * Returns: the license type
 *
 * Since: 1.5
 */
GtkLicense
adw_about_dialog_get_license_type (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), GTK_LICENSE_UNKNOWN);

  return self->license_type;
}

/**
 * adw_about_dialog_set_license_type: (attributes org.gtk.Method.set_property=license-type)
 * @self: an about dialog
 * @license_type: the license type
 *
 * Sets the license for @self from a list of known licenses.
 *
 * If the application's license is not in the list,
 * [property@AboutDialog:license] can be used instead. The license type will be
 * automatically set to `GTK_LICENSE_CUSTOM` in that case.
 *
 * If @license_type is `GTK_LICENSE_UNKNOWN`, no information will be displayed.
 *
 * If @license_type is different from `GTK_LICENSE_CUSTOM`.
 * [property@AboutDialog:license] will be cleared out.
 *
 * The license description will be displayed on the Legal page, below the
 * copyright information.
 *
 * [method@AboutDialog.add_legal_section] can be used to add license information
 * for the application dependencies or other components.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_license_type (AdwAboutDialog *self,
                                   GtkLicense      license_type)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (license_type >= GTK_LICENSE_UNKNOWN &&
                    license_type < G_N_ELEMENTS (gtk_license_info));

  if (self->license_type == license_type)
    return;

  if (license_type != GTK_LICENSE_CUSTOM)
    g_set_str (&self->license, "");

  self->license_type = license_type;

  update_legal (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LICENSE]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LICENSE_TYPE]);
}

/**
 * adw_about_dialog_get_license: (attributes org.gtk.Method.get_property=license)
 * @self: an about dialog
 *
 * Gets the license for @self.
 *
 * Returns: the license
 *
 * Since: 1.5
 */
const char *
adw_about_dialog_get_license (AdwAboutDialog *self)
{
  g_return_val_if_fail (ADW_IS_ABOUT_DIALOG (self), NULL);

  return self->license;
}

/**
 * adw_about_dialog_set_license: (attributes org.gtk.Method.set_property=license)
 * @self: an about dialog
 * @license: the license
 *
 * Sets the license for @self.
 *
 * This can be used to set a custom text for the license if it can't be set via
 * [property@AboutDialog:license-type].
 *
 * When set, [property@AboutDialog:license-type] will be set to
 * `GTK_LICENSE_CUSTOM`.
 *
 * The license text will be displayed on the Legal page, below the copyright
 * information.
 *
 * License text can contain Pango markup and links.
 *
 * [method@AboutDialog.add_legal_section] can be used to add license information
 * for the application dependencies or other components.
 *
 * Since: 1.5
 */
void
adw_about_dialog_set_license (AdwAboutDialog *self,
                              const char     *license)
{
  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (license != NULL);

  if (g_strcmp0 (self->license, license) == 0)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  g_set_str (&self->license, license);
  self->license_type = GTK_LICENSE_CUSTOM;

  update_legal (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LICENSE]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LICENSE_TYPE]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_about_dialog_add_legal_section:
 * @self: an about dialog
 * @title: the name of the section
 * @copyright: (nullable): a copyright string
 * @license_type: the type of license
 * @license: (nullable): custom license information
 *
 * Adds an extra section to the Legal page.
 *
 * Extra sections will be displayed below the application's own information.
 *
 * The parameters @copyright, @license_type and @license will be used to present
 * the it the same way as [property@AboutDialog:copyright],
 * [property@AboutDialog:license-type] and [property@AboutDialog:license] are
 * for the application's own information.
 *
 * See those properties for more details.
 *
 * This can be useful to attribute the application dependencies or data.
 *
 * Examples:
 *
 * ```c
 * adw_about_dialog_add_legal_section (ADW_ABOUT_DIALOG (about),
 *                                     _("Copyright and a known license"),
 *                                     "© 2022 Example",
 *                                     GTK_LICENSE_LGPL_2_1,
 *                                     NULL);
 *
 * adw_about_dialog_add_legal_section (ADW_ABOUT_DIALOG (about),
 *                                     _("Copyright and custom license"),
 *                                     "© 2022 Example",
 *                                     GTK_LICENSE_CUSTOM,
 *                                     "Custom license text");
 *
 * adw_about_dialog_add_legal_section (ADW_ABOUT_DIALOG (about),
 *                                     _("Copyright only"),
 *                                     "© 2022 Example",
 *                                     GTK_LICENSE_UNKNOWN,
 *                                     NULL);
 *
 * adw_about_dialog_add_legal_section (ADW_ABOUT_DIALOG (about),
 *                                     _("Custom license only"),
 *                                     NULL,
 *                                     GTK_LICENSE_CUSTOM,
 *                                     "Something completely custom here.");
 * ```
 *
 * Since: 1.5
 */
void
adw_about_dialog_add_legal_section (AdwAboutDialog *self,
                                    const char     *title,
                                    const char     *copyright,
                                    GtkLicense      license_type,
                                    const char     *license)
{
  LegalSection *section;

  g_return_if_fail (ADW_IS_ABOUT_DIALOG (self));
  g_return_if_fail (title != NULL);
  g_return_if_fail (license_type >= GTK_LICENSE_UNKNOWN &&
                    license_type < G_N_ELEMENTS (gtk_license_info));

  section = g_new0 (LegalSection, 1);
  section->title = g_strdup (title);
  section->copyright = g_strdup (copyright);
  section->license_type = license_type;
  section->license = g_strdup (license);

  self->legal_sections = g_slist_append (self->legal_sections, section);

  update_legal (self);
}

/**
 * adw_show_about_dialog: (skip)
 * @parent: the parent widget
 * @first_property_name: the name of the first property
 * @...: value of first property, followed by more pairs of property name and
 *   value, `NULL`-terminated
 *
 * A convenience function for showing an application’s about dialog.
 *
 * Since: 1.5
 */
void
adw_show_about_dialog (GtkWidget  *parent,
                       const char *first_property_name,
                       ...)
{
  AdwDialog *dialog;
  va_list var_args;

  g_return_if_fail (GTK_IS_WIDGET (parent));

  dialog = adw_about_dialog_new ();

  va_start (var_args, first_property_name);
  g_object_set_valist (G_OBJECT (dialog), first_property_name, var_args);
  va_end (var_args);

  adw_dialog_present (dialog, parent);
}

/**
 * adw_show_about_dialog_from_appdata: (skip)
 * @parent: the parent widget
 * @resource_path: The resource to use
 * @release_notes_version: (nullable): The version to retrieve release notes for
 * @first_property_name: the name of the first property
 * @...: value of first property, followed by more pairs of property name and
 *   value, `NULL`-terminated
 *
 * A convenience function for showing an application’s about dialog from
 * AppStream metadata.
 *
 * See [ctor@AboutDialog.new_from_appdata] for details.
 *
 * Since: 1.5
 */
void
adw_show_about_dialog_from_appdata (GtkWidget  *parent,
                                    const char *resource_path,
                                    const char *release_notes_version,
                                    const char *first_property_name,
                                    ...)
{
  AdwDialog *dialog;
  va_list var_args;

  g_return_if_fail (GTK_IS_WIDGET (parent));

  dialog = adw_about_dialog_new_from_appdata (resource_path,
                                              release_notes_version);

  va_start (var_args, first_property_name);
  g_object_set_valist (G_OBJECT (dialog), first_property_name, var_args);
  va_end (var_args);

  adw_dialog_present (dialog, parent);
}
