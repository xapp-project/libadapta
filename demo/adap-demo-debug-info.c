#include "config.h"

#include "adw-demo-debug-info.h"

/* Copied and adapted from gtk/inspector/general.c */
static void
get_gtk_info (const char **backend,
              const char **renderer)
{
  GdkDisplay *display = gdk_display_get_default ();
  GdkSurface *surface;
  GskRenderer *gsk_renderer;

  if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkX11Display"))
    *backend = "X11";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkWaylandDisplay"))
    *backend = "Wayland";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkBroadwayDisplay"))
    *backend = "Broadway";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkWin32Display"))
    *backend = "Windows";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (display), "GdkMacosDisplay"))
    *backend = "macOS";
  else
    *backend = G_OBJECT_TYPE_NAME (display);

  surface = gdk_surface_new_toplevel (display);
  gsk_renderer = gsk_renderer_new_for_surface (surface);
  if (!g_strcmp0 (G_OBJECT_TYPE_NAME (gsk_renderer), "GskVulkanRenderer"))
    *renderer = "Vulkan";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (gsk_renderer), "GskNglRenderer"))
    *renderer = "NGL";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (gsk_renderer), "GskGLRenderer"))
    *renderer = "GL";
  else if (!g_strcmp0 (G_OBJECT_TYPE_NAME (gsk_renderer), "GskCairoRenderer"))
    *renderer = "Cairo";
  else
    *renderer = G_OBJECT_TYPE_NAME (gsk_renderer);

  gsk_renderer_unrealize (gsk_renderer);
  g_object_unref (gsk_renderer);
  gdk_surface_destroy (surface);
}

#ifndef G_OS_WIN32
static char *
get_flatpak_info (const char *group,
                  const char *key)
{
  GKeyFile *keyfile = g_key_file_new ();
  char *ret = NULL;

  if (g_key_file_load_from_file (keyfile, "/.flatpak-info", 0, NULL))
    ret = g_key_file_get_string (keyfile, group, key, NULL);

  g_key_file_unref (keyfile);

  return ret;
}
#endif

char *
adw_demo_generate_debug_info (void)
{
  GString *string = g_string_new (NULL);
#ifndef G_OS_WIN32
  gboolean flatpak = g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS);
#endif

  g_string_append_printf (string, "Libadwaita demo: %s (%s)\n", ADW_VERSION_S,
                                                                ADW_DEMO_VCS_TAG);
  g_string_append (string, "\n");

  g_string_append (string, "Compiled against:\n");
  g_string_append_printf (string, "- GLib: %d.%d.%d\n", GLIB_MAJOR_VERSION,
                                                        GLIB_MINOR_VERSION,
                                                        GLIB_MICRO_VERSION);
  g_string_append_printf (string, "- GTK: %d.%d.%d\n", GTK_MAJOR_VERSION,
                                                       GTK_MINOR_VERSION,
                                                       GTK_MICRO_VERSION);
  g_string_append (string, "\n");

  g_string_append (string, "Running against:\n");
  g_string_append_printf (string, "- GLib: %d.%d.%d\n", glib_major_version,
                                                        glib_minor_version,
                                                        glib_micro_version);
  g_string_append_printf (string, "- GTK: %d.%d.%d\n", gtk_get_major_version (),
                                                       gtk_get_minor_version (),
                                                       gtk_get_micro_version ());
  g_string_append (string, "\n");

  {
    char *os_name = g_get_os_info (G_OS_INFO_KEY_NAME);
    char *os_version = g_get_os_info (G_OS_INFO_KEY_VERSION);

    g_string_append (string, "System:\n");
    g_string_append_printf (string, "- Name: %s\n", os_name);
    g_string_append_printf (string, "- Version: %s\n", os_version);
    g_string_append (string, "\n");

    g_free (os_name);
    g_free (os_version);
  }

  {
    const char *backend, *renderer;

    get_gtk_info (&backend, &renderer);

    g_string_append (string, "GTK:\n");
    g_string_append_printf (string, "- GDK backend: %s\n", backend);
    g_string_append_printf (string, "- GSK renderer: %s\n", renderer);
    g_string_append (string, "\n");
  }

#ifndef G_OS_WIN32
  if (flatpak) {
    char *runtime = get_flatpak_info ("Application", "runtime");
    char *runtime_commit = get_flatpak_info ("Instance", "runtime-commit");
    char *arch = get_flatpak_info ("Instance", "arch");
    char *flatpak_version = get_flatpak_info ("Instance", "flatpak-version");
    char *devel = get_flatpak_info ("Instance", "devel");

    g_string_append (string, "Flatpak:\n");
    g_string_append_printf (string, "- Runtime: %s\n", runtime);
    g_string_append_printf (string, "- Runtime commit: %s\n", runtime_commit);
    g_string_append_printf (string, "- Arch: %s\n", arch);
    g_string_append_printf (string, "- Flatpak version: %s\n", flatpak_version);
    g_string_append_printf (string, "- Devel: %s\n", devel ? "yes" : "no");
    g_string_append (string, "\n");

    g_free (runtime);
    g_free (runtime_commit);
    g_free (arch);
    g_free (flatpak_version);
    g_free (devel);
  }
#endif

  {
    const char *desktop = g_getenv ("XDG_CURRENT_DESKTOP");
    const char *session_desktop = g_getenv ("XDG_SESSION_DESKTOP");
    const char *session_type = g_getenv ("XDG_SESSION_TYPE");
    const char *lang = g_getenv ("LANG");
    const char *builder = g_getenv ("INSIDE_GNOME_BUILDER");
    const char *gtk_debug = g_getenv ("GTK_DEBUG");
    const char *gtk_theme = g_getenv ("GTK_THEME");
    const char *adw_debug_color_scheme = g_getenv ("ADW_DEBUG_COLOR_SCHEME");
    const char *adw_debug_high_contrast = g_getenv ("ADW_DEBUG_HIGH_CONTRAST");
    const char *adw_disable_portal = g_getenv ("ADW_DISABLE_PORTAL");

    g_string_append (string, "Environment:\n");
    g_string_append_printf (string, "- Desktop: %s\n", desktop);
    g_string_append_printf (string, "- Session: %s (%s)\n", session_desktop,
                                                            session_type);
    g_string_append_printf (string, "- Language: %s\n", lang);
    g_string_append_printf (string, "- Running inside Builder: %s\n", builder ? "yes" : "no");

    if (gtk_debug)
      g_string_append_printf (string, "- GTK_DEBUG: %s\n", gtk_debug);
    if (gtk_theme)
      g_string_append_printf (string, "- GTK_THEME: %s\n", gtk_theme);
    if (adw_debug_color_scheme)
      g_string_append_printf (string, "- ADW_DEBUG_COLOR_SCHEME: %s\n", adw_debug_color_scheme);
    if (adw_debug_high_contrast)
      g_string_append_printf (string, "- ADW_DEBUG_HIGH_CONTRAST: %s\n", adw_debug_high_contrast);
    if (adw_disable_portal)
      g_string_append_printf (string, "- ADW_DISABLE_PORTAL: %s\n", adw_disable_portal);
  }

  return g_string_free_and_steal (string);
}
