#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_TAB_VIEW_DEMO_PAGE (adap_tab_view_demo_page_get_type())

G_DECLARE_FINAL_TYPE (AdapTabViewDemoPage, adap_tab_view_demo_page, ADAP, TAB_VIEW_DEMO_PAGE, AdapBin)

AdapTabViewDemoPage *adap_tab_view_demo_page_new           (const char         *title);
AdapTabViewDemoPage *adap_tab_view_demo_page_new_duplicate (AdapTabViewDemoPage *self);

void adap_tab_view_demo_page_refresh_icon    (AdapTabViewDemoPage *self);
void adap_tab_view_demo_page_set_enable_icon (AdapTabViewDemoPage *self,
                                             gboolean            enable_icon);

G_END_DECLS
