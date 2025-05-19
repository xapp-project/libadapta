#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_TAB_VIEW_DEMO_PAGE (adw_tab_view_demo_page_get_type())

G_DECLARE_FINAL_TYPE (AdwTabViewDemoPage, adw_tab_view_demo_page, ADW, TAB_VIEW_DEMO_PAGE, AdwBin)

AdwTabViewDemoPage *adw_tab_view_demo_page_new           (const char         *title);
AdwTabViewDemoPage *adw_tab_view_demo_page_new_duplicate (AdwTabViewDemoPage *self);

void adw_tab_view_demo_page_refresh_icon    (AdwTabViewDemoPage *self);
void adw_tab_view_demo_page_set_enable_icon (AdwTabViewDemoPage *self,
                                             gboolean            enable_icon);

G_END_DECLS
