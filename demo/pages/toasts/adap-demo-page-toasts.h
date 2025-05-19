#pragma once

#include <adapta.h>

G_BEGIN_DECLS

#define ADAP_TYPE_DEMO_PAGE_TOASTS (adap_demo_page_toasts_get_type())

G_DECLARE_FINAL_TYPE (AdapDemoPageToasts, adap_demo_page_toasts, ADAP, DEMO_PAGE_TOASTS, AdapBin)

void adap_demo_page_toasts_undo (AdapDemoPageToasts *self);

G_END_DECLS
