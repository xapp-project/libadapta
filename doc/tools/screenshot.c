#include <adapta.h>

#define RESOURCE_PATH "/org/gnome/Adapta/Screenshot/"

static GMainLoop *loop;
static char *option_image = NULL;
static gboolean option_list = FALSE;

static GOptionEntry entries[] =
{
  { "image", 'i', 0, G_OPTION_ARG_STRING, &option_image, "Generate only one image", "NAME" },
  { "list",  'l', 0, G_OPTION_ARG_NONE,   &option_list,  "List images",             NULL   },
  { NULL }
};

static void
pixbuf_texture_unref_cb (guchar   *pixels,
                         gpointer  bytes)
{
  g_bytes_unref (bytes);
}

static GdkPixbuf *
create_pixbuf_from_texture (GdkTexture *texture)
{
  GdkTextureDownloader *downloader;
  GBytes *bytes;
  gsize stride;

  downloader = gdk_texture_downloader_new (texture);
  gdk_texture_downloader_set_format (downloader, GDK_MEMORY_R8G8B8A8);
  bytes = gdk_texture_downloader_download_bytes (downloader, &stride);

  gdk_texture_downloader_free (downloader);

  return gdk_pixbuf_new_from_data (g_bytes_get_data (bytes, NULL),
                                   GDK_COLORSPACE_RGB,
                                   TRUE,
                                   8,
                                   gdk_texture_get_width (texture),
                                   gdk_texture_get_height (texture),
                                   stride,
                                   pixbuf_texture_unref_cb,
                                   bytes);
}

typedef struct {
  GtkWidget *window;
  GtkWidget *widget;
  GtkWidget *hover_widget;
  GtkWidget *hscroll_widget;
  GtkWidget *vscroll_widget;
  GtkWidget *nav_view_child_widget;
  GdkPaintable *paintable;
  char *name;
  GtkCssProvider *provider;
  GtkCssProvider *provider_dark;
} ScreenshotData;

static void
screenshot_data_free (ScreenshotData *data)
{
  g_object_unref (data->paintable);
  gtk_window_destroy (GTK_WINDOW (gtk_widget_get_root (data->widget)));
  g_object_unref (data->provider);
  g_clear_object (&data->provider_dark);
  g_free (data->name);
  g_free (data);
}

static GdkPixbuf *
crop_alpha (GdkPixbuf *pixbuf)
{
  const guint8 *pixels = gdk_pixbuf_read_pixels (pixbuf);
  int row, col, width, height, stride, top, bottom, left, right;

  if (!gdk_pixbuf_get_has_alpha (pixbuf))
    return g_object_ref (pixbuf);

  if (gdk_pixbuf_get_n_channels (pixbuf) != 4)
    return g_object_ref (pixbuf);

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  stride = gdk_pixbuf_get_rowstride (pixbuf);

  left = top = 0;
  right = width;
  bottom = height;

  /* Left */
  for (col = 0; col < width; col++) {
    gboolean empty = TRUE;

    for (row = 0; row < height; row++) {
      if (pixels[row * stride + col * 4 + 3] != 0) {
        empty = FALSE;
        break;
      }
    }

    if (!empty) {
      left = col;
      break;
    }
  }

  /* Right */
  for (col = width - 1; col > left; col--) {
    gboolean empty = TRUE;

    for (row = 0; row < height; row++) {
      if (pixels[row * stride + col * 4 + 3] != 0) {
        empty = FALSE;
        break;
      }
    }

    if (!empty) {
      right = col + 1;
      break;
    }
  }

  /* Top */
  for (row = 0; row < height; row++) {
    gboolean empty = TRUE;

    for (col = 0; col < width; col++) {
      if (pixels[row * stride + col * 4 + 3] != 0) {
        empty = FALSE;
        break;
      }
    }

    if (!empty) {
      top = row;
      break;
    }
  }

  /* Bottom */
  for (row = height - 1; row > top; row--) {
    gboolean empty = TRUE;

    for (col = 0; col < width; col++) {
      if (pixels[row * stride + col * 4 + 3] != 0) {
        empty = FALSE;
        break;
      }
    }

    if (!empty) {
      bottom = row + 1;
      break;
    }
  }

  return gdk_pixbuf_new_subpixbuf (pixbuf, left, top, right - left, bottom - top);
}

static void
draw_paintable_cb (ScreenshotData *data)
{
  GtkSnapshot *snapshot;
  GskRenderer *renderer;
  GdkTexture *texture;
  GskRenderNode *node;
  int x, y, width, height;
  graphene_rect_t bounds;

  g_assert (gtk_widget_compute_bounds (data->widget, data->window, &bounds));

  if (GTK_IS_NATIVE (data->widget)) {
    GdkSurface *surface;
    double transform_x, transform_y;

    surface = gtk_native_get_surface (GTK_NATIVE (data->widget));
    gtk_native_get_surface_transform (GTK_NATIVE (data->widget),
                                      &transform_x, &transform_y);
    x = floor (transform_x);
    y = floor (transform_y);
    width = gdk_surface_get_width (surface);
    height = gdk_surface_get_height (surface);
  } else {
    x = gtk_widget_get_margin_start (data->widget);
    y = gtk_widget_get_margin_top (data->widget);
    width = bounds.size.width + x + gtk_widget_get_margin_end (data->widget);
    height = bounds.size.height + y + gtk_widget_get_margin_bottom (data->widget);
  }

  snapshot = gtk_snapshot_new ();

  gtk_snapshot_translate (snapshot, &GRAPHENE_POINT_INIT (x, y));
  gdk_paintable_snapshot (data->paintable, snapshot, bounds.size.width, bounds.size.height);
  node = gtk_snapshot_free_to_node (snapshot);

  if (gsk_render_node_get_node_type (node) == GSK_CLIP_NODE &&
      (x > 0 || y > 0 || bounds.size.width < width || bounds.size.height < height)) {
    GskRenderNode *original_node = g_steal_pointer (&node);

    node = gsk_render_node_ref (gsk_clip_node_get_child (original_node));

    gsk_render_node_unref (original_node);
  }

  renderer = gtk_native_get_renderer (gtk_widget_get_native (data->widget));

  texture = gsk_renderer_render_texture (renderer, node,
                                         &GRAPHENE_RECT_INIT (0, 0, width, height));

  if (GTK_IS_NATIVE (data->widget)) {
    GdkPixbuf *pixbuf = create_pixbuf_from_texture (texture);
    GdkPixbuf *cropped_pixbuf = crop_alpha (pixbuf);

    gdk_pixbuf_save (cropped_pixbuf, data->name, "png", NULL, NULL);

    g_object_unref (cropped_pixbuf);
    g_object_unref (pixbuf);
  } else {
    gdk_texture_save_to_png (texture, data->name);
  }

  screenshot_data_free (data);

  g_main_loop_quit (loop);

  g_object_unref (texture);
  gsk_render_node_unref (node);
}

static void
draw_paintable (ScreenshotData *data)
{
  g_signal_handlers_disconnect_by_func (data->paintable,
                                        G_CALLBACK (draw_paintable),
                                        data);

  /* Handle the case where something immediately invalidates allocation. */
  g_timeout_add_once (50, (GSourceOnceFunc) draw_paintable_cb, data);
}

static GtkCssProvider *
load_css (const char *name)
{
  GtkCssProvider *provider = gtk_css_provider_new ();
  char *path = g_strdup_printf (RESOURCE_PATH "%s.css", name);

  gtk_css_provider_load_from_resource (provider, path);

  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_free (path);
  return provider;
}

static void
take_screenshot_cb (ScreenshotData *data)
{
  if (GTK_IS_POPOVER (data->widget))
    gtk_popover_popup (GTK_POPOVER (data->widget));

  if (data->hover_widget) {
    GtkStateFlags flags = gtk_widget_get_state_flags (data->hover_widget);

    gtk_widget_set_state_flags (data->hover_widget,
                                flags | GTK_STATE_FLAG_PRELIGHT, FALSE);
  }

  if (data->hscroll_widget) {
    GtkAdjustment *adj;

    g_assert (GTK_IS_SCROLLED_WINDOW (data->hscroll_widget));

    adj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (data->hscroll_widget));

    gtk_adjustment_set_value (adj,
                              (gtk_adjustment_get_lower (adj) +
                               gtk_adjustment_get_upper (adj) -
                               gtk_adjustment_get_page_size (adj)) / 2);
  }

  if (data->vscroll_widget) {
    GtkAdjustment *adj;

    g_assert (GTK_IS_SCROLLED_WINDOW (data->vscroll_widget));

    adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (data->vscroll_widget));

    gtk_adjustment_set_value (adj,
                              (gtk_adjustment_get_lower (adj) +
                               gtk_adjustment_get_upper (adj) -
                               gtk_adjustment_get_page_size (adj)) / 2);
  }

  if (data->nav_view_child_widget) {
    AdapNavigationPage *visible_page;
    GtkWidget *view;

    g_assert (ADAP_IS_NAVIGATION_PAGE (data->nav_view_child_widget));

    view = gtk_widget_get_parent (data->nav_view_child_widget);

    g_assert (ADAP_IS_NAVIGATION_VIEW (view));

    visible_page = adap_navigation_view_get_visible_page (ADAP_NAVIGATION_VIEW (view));

    adap_navigation_view_replace (ADAP_NAVIGATION_VIEW (view),
                                 (AdapNavigationPage *[2]) {
                                   visible_page,
                                   ADAP_NAVIGATION_PAGE (data->nav_view_child_widget)
                                 }, 2);
  }

  g_signal_connect_swapped (data->paintable, "invalidate-contents",
                            G_CALLBACK (draw_paintable), data);

  gtk_widget_queue_draw (data->widget);
}

static void
take_screenshot (const char *name,
                 gboolean    dark,
                 GFile      *input_dir,
                 GFile      *output_dir)
{
  char *input_name;
  GFile *input_file;
  char *input_path;
  char *output_name;
  GtkBuilder *builder;
  GFile *output_file;
  ScreenshotData *data;
  GObject *widget;
  GObject *hover_widget;
  GObject *hscroll_widget;
  GObject *vscroll_widget;
  GObject *nav_view_child_widget;
  GtkWidget *window;
  gboolean wait = FALSE;

  input_name = g_strconcat (name, ".ui", NULL);
  input_file = g_file_get_child (input_dir, input_name);
  input_path = g_file_get_path (input_file);

  if (dark)
    output_name = g_strdup_printf ("%s-dark.png", name);
  else
    output_name = g_strdup_printf ("%s.png", name);

  output_file = g_file_get_child (output_dir, output_name);

  loop = g_main_loop_new (NULL, FALSE);

  if (dark)
    adap_style_manager_set_color_scheme (adap_style_manager_get_default (),
                                        ADAP_COLOR_SCHEME_FORCE_DARK);
  else
    adap_style_manager_set_color_scheme (adap_style_manager_get_default (),
                                        ADAP_COLOR_SCHEME_FORCE_LIGHT);

  builder = gtk_builder_new_from_file (input_path);
  widget = gtk_builder_get_object (builder, "widget");
  hover_widget = gtk_builder_get_object (builder, "hover");
  hscroll_widget = gtk_builder_get_object (builder, "hscroll");
  vscroll_widget = gtk_builder_get_object (builder, "vscroll");
  nav_view_child_widget = gtk_builder_get_object (builder, "nav-page");

  g_assert (GTK_IS_WIDGET (widget));

  data = g_new0 (ScreenshotData, 1);

  if (GTK_IS_WINDOW (widget)) {
    window = GTK_WIDGET (widget);
  } else if (GTK_IS_POPOVER (widget)) {
    GtkWidget *button;

    gtk_popover_set_autohide (GTK_POPOVER (widget), FALSE);

    button = gtk_menu_button_new ();
    gtk_menu_button_set_popover (GTK_MENU_BUTTON (button), GTK_WIDGET (widget));

    window = gtk_window_new ();
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_child (GTK_WINDOW (window), button);

    wait = TRUE;
  } else if (ADAP_IS_DIALOG (widget)) {
    window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
    adap_dialog_present (ADAP_DIALOG (widget), window);
    widget = G_OBJECT (window);
  } else if (gtk_widget_get_root (GTK_WIDGET (widget))) {
    window = GTK_WIDGET (gtk_widget_get_root (GTK_WIDGET (widget)));
  } else {
    window = gtk_window_new ();
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (widget));
  }

  data->widget = GTK_WIDGET (widget);
  data->window = window;
  data->paintable = gtk_widget_paintable_new (data->widget);
  data->name = g_file_get_path (output_file);
  data->provider = load_css ("style");

  if (dark)
    data->provider_dark = load_css ("style-dark");

  if (hover_widget)
    data->hover_widget = GTK_WIDGET (hover_widget);

  if (hscroll_widget)
    data->hscroll_widget = GTK_WIDGET (hscroll_widget);

  if (vscroll_widget)
    data->vscroll_widget = GTK_WIDGET (vscroll_widget);

  if (nav_view_child_widget)
    data->nav_view_child_widget = GTK_WIDGET (nav_view_child_widget);

  if (wait)
    g_timeout_add_once (1000, (GSourceOnceFunc) take_screenshot_cb, data);

  gtk_window_present (GTK_WINDOW (window));

  if (!wait) {
    if (hscroll_widget || vscroll_widget)
      g_idle_add_once ((GSourceOnceFunc) take_screenshot_cb, data);
    else
      take_screenshot_cb (data);
  }

  g_main_loop_run (loop);
  g_main_loop_unref (loop);

  g_object_unref (builder);
  g_object_unref (output_file);
  g_free (output_name);
  g_free (input_path);
  g_object_unref (input_file);
  g_free (input_name);
}

static inline char *
get_shortname (const char *basename)
{
  const char *first_period = strstr (basename, ".");

  if (first_period)
    return g_strndup (basename, first_period - basename);

  return g_strdup (basename);
}

static void
init_libadapta (void)
{
  adap_init ();

  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_for_display (gdk_display_get_default ()),
                                    RESOURCE_PATH "icons/");

  g_object_set (gtk_settings_get_default (),
                "gtk-enable-animations", FALSE,
                "gtk-font-name", "Cantarell 11",
                "gtk-icon-theme-name", "Adapta",
                "gtk-decoration-layout", ":close",
                "gtk-hint-font-metrics", TRUE,
                NULL);
}

static GList *
list_images (GFile *input_dir)
{
  GFileEnumerator *enumerator;
  GError *error = NULL;
  GList *children = NULL;
  GFileInfo *info;

  enumerator =
    g_file_enumerate_children (input_dir,
                               G_FILE_ATTRIBUTE_STANDARD_NAME,
                               G_FILE_QUERY_INFO_NONE,
                               NULL,
                               &error);
  if (error) {
    g_critical ("Couldn't enumerate images: %s", error->message);

    g_clear_error (&error);

    return NULL;
  }

  while ((info = g_file_enumerator_next_file (enumerator, NULL, &error))) {
    const char *name = NULL;
    char *shortname;

    if (error) {
      g_critical ("Couldn't enumerate image: %s", error->message);

      g_clear_error (&error);

      continue;
    }

    name = g_file_info_get_name (info);

    if (!g_str_has_suffix (name, ".ui"))
      continue;

    shortname = get_shortname (name);

    children = g_list_prepend (children, shortname);
  }

  g_object_unref (enumerator);

  return g_list_sort (children, (GCompareFunc) g_ascii_strcasecmp);
}

static void
process_image (const char *name,
               GFile      *input_dir,
               GFile      *output_dir)
{
  g_print ("Processing %s\n", name);

  take_screenshot (name, FALSE, input_dir, output_dir);
  take_screenshot (name, TRUE,  input_dir, output_dir);
}

static void
run_screenshot (GFile *input_dir,
                GFile *output_dir)
{
  GList *children = NULL;
  GList *l;

  if (option_image) {
    char *input_name = g_strconcat (option_image, ".ui", NULL);
    GFile *input_file = g_file_get_child (input_dir, input_name);

    if (g_file_query_exists (input_file, NULL))
      process_image (option_image, input_dir, output_dir);
    else
      g_printerr ("No such image: %s\n", option_image);

    g_object_unref (input_file);
    g_free (input_name);

    return;
  }

  children = list_images (input_dir);

  for (l = children; l; l = l->next) {
    char *shortname = l->data;

    process_image (shortname, input_dir, output_dir);
	g_free (shortname);
  }

  g_list_free (children);
}

static void
run_list_images (GFile *input_dir)
{
  GList *children = list_images (input_dir);
  GList *l;

  for (l = children; l; l = l->next) {
    char *shortname = l->data;

    g_print ("%s\n", shortname);

    g_free (shortname);
  }

  g_list_free (children);
}

int
main (int    argc,
      char **argv)
{
  GOptionContext *context = g_option_context_new ("INPUT_DIR OUTPUT_DIR");
  GFile *input_dir;
  GFile *output_dir = NULL;
  GError *error = NULL;
  gboolean result;

  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, NULL)) {
    g_printerr ("%s\n", g_option_context_get_help (context, FALSE, NULL));

    return 1;
  }

  if (option_list) {
    if (argc < 2 || !argv[1]) {
      g_printerr ("Input directory must be set to list images\n");

      return 1;
    }

    input_dir = g_file_new_for_path (argv[1]);

    result = g_file_query_exists (input_dir, NULL);

    if (result)
      run_list_images (input_dir);
    else
      g_critical ("Input directory does not exist");

    g_object_unref (input_dir);

    return (!result);
  }

  if (argc < 3 || !argv[1] || !argv[2]) {
    g_printerr ("%s\n", g_option_context_get_help (context, FALSE, NULL));

    return 1;
  }

  input_dir = g_file_new_for_path (argv[1]);

  result = g_file_query_exists (input_dir, NULL);

  if (result)
    output_dir = g_file_new_for_path (argv[2]);
  else
    g_critical ("Input directory does not exist");

  if (result && !g_file_query_exists (output_dir, NULL))
    g_file_make_directory_with_parents (output_dir, NULL, &error);

  if (G_UNLIKELY (error != NULL)) {
    g_critical ("Failed to create output directory: %s", error->message);

    g_clear_error (&error);
    result = FALSE;
  }

  if (result) {
    init_libadapta ();
    run_screenshot (input_dir, output_dir);
  }

  if (output_dir != NULL)
    g_object_unref (output_dir);

  g_object_unref (input_dir);

  return (!result);
}
