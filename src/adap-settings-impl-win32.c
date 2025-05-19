/*
 * Copyright (C) 2023 Jason Francis
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#define _WIN32_WINNT 0x0602
#define INITGUID

#include "adw-settings-impl-private.h"

#include <gtk/gtk.h>
#include <gdk/win32/gdkwin32.h>

#ifdef HAS_WINRT
#define COBJMACROS
#include <inspectable.h>
#include <roapi.h>
#include <winstring.h>
#include <Windows.UI.ViewManagement.h>
#include <Windows.Foundation.h>
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x031A
#endif

struct _AdwSettingsImplWin32
{
  AdwSettingsImpl parent_instance;

#ifdef HAS_WINRT
  gboolean initialized;

  HMODULE combase;
  HRESULT (WINAPI *RoInitialize_func)(RO_INIT_TYPE initType);
  HRESULT (WINAPI *RoActivateInstance_func)(HSTRING        activatableClassId,
                                            IInspectable **instance);
  HRESULT (WINAPI *WindowsCreateStringReference_func)(PCWSTR          sourceString,
                                                      UINT32          length,
                                                      HSTRING_HEADER *hstringHeader,
                                                      HSTRING        *string);

  IInspectable *ii_ui;
  __x_ABI_CWindows_CUI_CViewManagement_CIUISettings3 *ui;

  EventRegistrationToken color_changed_token;
#endif

  gboolean added_filter;
};

G_DEFINE_FINAL_TYPE (AdwSettingsImplWin32, adw_settings_impl_win32, ADW_TYPE_SETTINGS_IMPL)

/* Dark mode is only supported if WinRT is available on Windows 10 or above. */
#ifdef HAS_WINRT

DEFINE_GUID (IID_IUISettings3, 0x03021be4, 0x5254, 0x4781, 0x81, 0x94, 0x51, 0x68, 0xf7, 0xd0, 0x6d, 0x7b);
DEFINE_GUID (IID_UISettingsEventHandler, 0x2dbdba9d, 0x20da, 0x519d, 0x90, 0x78, 0x09, 0xf8, 0x35, 0xbc, 0x5b, 0xc7);

static inline IInspectable *
com_activate (AdwSettingsImplWin32 *self,
              const WCHAR          *name)
{
  HRESULT res;
  HSTRING_HEADER header;
  HSTRING str;
  IInspectable *iface;

  if (!self->initialized)
    return NULL;

  res = (self->WindowsCreateStringReference_func) (name, wcslen (name), &header, &str);
  if (FAILED (res))
    return NULL;

  res = (self->RoActivateInstance_func) (str, &iface);
  if (FAILED (res))
    return NULL;

  return iface;
}

/* COM Object for ColorValuesChanged closure */
typedef struct
{
  gpointer lpVtbl;
  int ref_count;
  GWeakRef settings;
} TypedEventHandler;

static inline TypedEventHandler *
TypedEventHandler_New (gpointer              vtbl,
                       AdwSettingsImplWin32 *settings)
{
  TypedEventHandler *handler = g_new0 (TypedEventHandler, 1);

  handler->lpVtbl = vtbl;
  handler->ref_count = 1;
  g_weak_ref_init (&handler->settings, settings);

  return handler;
}

static ULONG STDMETHODCALLTYPE
TypedEventHandler_AddRef (TypedEventHandler *self)
{
  return g_atomic_int_add (&self->ref_count, 1) + 1;
}

static ULONG STDMETHODCALLTYPE
TypedEventHandler_Release (TypedEventHandler *self)
{
  int rc;

  rc = g_atomic_int_add (&self->ref_count, -1) - 1;
  if (rc < 0)
    g_error ("Event handler over-released");
  if (rc == 0) {
    g_weak_ref_clear (&self->settings);
    g_free (self);
  }
  return rc;
}

static HRESULT STDMETHODCALLTYPE
UISettingsEvent_QueryInterface (__FITypedEventHandler_2_Windows__CUI__CViewManagement__CUISettings_IInspectable *self,
                                REFIID                                                                           iid,
                                gpointer                                                                        *iface)
{
   if (IsEqualIID (iid, &IID_UISettingsEventHandler) ||
       IsEqualIID (iid, &IID_IUnknown) ||
       IsEqualIID (iid, &IID_IAgileObject)) {
    *iface = &self->lpVtbl;
    self->lpVtbl->AddRef (self);
    return S_OK;
  }

  *iface = NULL;
  return E_NOINTERFACE;
}

/* Set dark mode if the foreground color is brighter than a threshold.
 * Algorithm is suggested by IsColorLight() in this example:
 * https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes
 */
static inline AdwSystemColorScheme
scheme_for_fg_color (DWORD c)
{
  if (5 * GetGValue (c) + 2 * GetRValue (c) + GetBValue (c) > 8 * 128)
    return ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;
  else
    return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

static inline HRESULT
color_values_changed (AdwSettingsImplWin32 *self)
{
  struct __x_ABI_CWindows_CUI_CColor color;
  HRESULT res;

  if (!self->ui)
    return S_FALSE;

  res = self->ui->lpVtbl->GetColorValue (self->ui, UIColorType_Foreground, &color);
  if (FAILED (res))
    return res;

  adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                      scheme_for_fg_color (RGB (color.R, color.G, color.B)));

  return S_OK;
}

static void
color_values_changed_idle (gpointer settings)
{
  color_values_changed (settings);
  g_object_unref (settings);
}

static HRESULT STDMETHODCALLTYPE
ColorValuesChanged_Invoke (__FITypedEventHandler_2_Windows__CUI__CViewManagement__CUISettings_IInspectable *self,
                           __x_ABI_CWindows_CUI_CViewManagement_CIUISettings                               *sender,
                           IInspectable                                                                    *args)
{
  TypedEventHandler *handler = (gpointer) self;
  AdwSettingsImplWin32 *settings;

  settings = g_weak_ref_get (&handler->settings);
  if (settings != NULL) {
    /* Event handler is invoked from another thread */
    g_idle_add_once (color_values_changed_idle, settings);
  }
  return S_OK;
}

struct __FITypedEventHandler_2_Windows__CUI__CViewManagement__CUISettings_IInspectableVtbl ColorValuesChangedVtbl = {
  .QueryInterface = UISettingsEvent_QueryInterface,
  .AddRef = (gpointer) TypedEventHandler_AddRef,
  .Release = (gpointer) TypedEventHandler_Release,
  .Invoke = ColorValuesChanged_Invoke,
};

static HRESULT
init_winrt_ui_settings (AdwSettingsImplWin32 *self)
{
  HRESULT res;
  TypedEventHandler *handler;

  self->ii_ui = com_activate (self, RuntimeClass_Windows_UI_ViewManagement_UISettings);
  if (!self->ii_ui)
    return E_FAIL;

  res = IInspectable_QueryInterface (self->ii_ui,
                                     &IID_IUISettings3,
                                     (gpointer*) &self->ui);
  if (FAILED (res))
    return E_FAIL;

  handler = TypedEventHandler_New (&ColorValuesChangedVtbl, self);
  res = self->ui->lpVtbl->add_ColorValuesChanged (self->ui,
                                                  (gpointer) handler,
                                                  &self->color_changed_token);
  TypedEventHandler_Release (handler);

  if (FAILED (res))
    return E_FAIL;

  res = color_values_changed (self);
  if (FAILED (res))
    return E_FAIL;

  return S_OK;
}

static void
init_winrt_module (AdwSettingsImplWin32 *self)
{
  HRESULT res;

  self->RoInitialize_func = (gpointer) GetProcAddress (self->combase, "RoInitialize");
  if (self->RoInitialize_func == NULL)
    return;

  self->RoActivateInstance_func = (gpointer) GetProcAddress (self->combase, "RoActivateInstance");
  if (self->RoActivateInstance_func == NULL)
    return;

  self->WindowsCreateStringReference_func = (gpointer) GetProcAddress (self->combase, "WindowsCreateStringReference");
  if (self->WindowsCreateStringReference_func == NULL)
    return;

  res = (self->RoInitialize_func) (RO_INIT_MULTITHREADED);
  if (res != RPC_E_CHANGED_MODE && FAILED (res))
    return;

  self->initialized = TRUE;
}

static void
cleanup_winrt_settings (AdwSettingsImplWin32 *self)
{
  if (self->ui) {
    if (self->color_changed_token.value)
      self->ui->lpVtbl->remove_ColorValuesChanged (self->ui, self->color_changed_token);

    self->ui->lpVtbl->Release (self->ui);
    self->ui = NULL;
  }

  if (self->ii_ui) {
    IInspectable_Release (self->ii_ui);
    self->ii_ui = NULL;
  }

  if (self->combase) {
    FreeLibrary (self->combase);
    self->combase = NULL;
  }
}

static HRESULT
init_winrt_settings (AdwSettingsImplWin32 *self)
{
  HRESULT res;

  self->combase = LoadLibraryW (L"combase.dll");
  if (self->combase == NULL)
    return E_FAIL;

  init_winrt_module (self);
  res = init_winrt_ui_settings (self);

  if (FAILED (res))
    cleanup_winrt_settings (self);

  return res;
}

#endif

/* High contrast is supported on all Windows versions. */
static void
system_colors_changed (AdwSettingsImplWin32 *self)
{
  HIGHCONTRASTA hc;

  hc.cbSize = sizeof hc;
  hc.dwFlags = 0;
  hc.lpszDefaultScheme = NULL;

  if (SystemParametersInfoA (SPI_GETHIGHCONTRAST, sizeof hc, &hc, 0)) {
    gboolean high_contrast = (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;

    adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self), high_contrast);
  }

#ifdef HAS_WINRT
  color_values_changed (self);
#endif
}

static GdkWin32MessageFilterReturn
system_colors_filter (GdkWin32Display *display,
                      MSG             *message,
                      int             *return_value,
                      gpointer         data)
{
  AdwSettingsImplWin32 *self = data;

  if (message->message == WM_SYSCOLORCHANGE ||
      message->message == WM_THEMECHANGED)
    system_colors_changed (self);

  return GDK_WIN32_MESSAGE_FILTER_CONTINUE;
}

static void
adw_settings_impl_win32_dispose (GObject *object)
{
  AdwSettingsImplWin32 *self = ADW_SETTINGS_IMPL_WIN32 (object);

#ifdef HAS_WINRT
  cleanup_winrt_settings (self);
#endif

  if (self->added_filter) {
    GdkDisplay *display = gdk_display_get_default ();

    gdk_win32_display_remove_filter (GDK_WIN32_DISPLAY (display),
                                     system_colors_filter,
                                     self);

    self->added_filter = FALSE;
  }

  G_OBJECT_CLASS (adw_settings_impl_win32_parent_class)->dispose (object);
}

static void
adw_settings_impl_win32_class_init (AdwSettingsImplWin32Class *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_settings_impl_win32_dispose;
}

static void
adw_settings_impl_win32_init (AdwSettingsImplWin32 *self)
{
}

AdwSettingsImpl *
adw_settings_impl_win32_new (gboolean enable_color_scheme,
                             gboolean enable_high_contrast)
{
  AdwSettingsImplWin32 *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_WIN32, NULL);
  GdkDisplay *display = gdk_display_get_default ();
  gboolean found_color_scheme = FALSE;

  if (!GDK_IS_WIN32_DISPLAY (display))
    return ADW_SETTINGS_IMPL (self);

  if (enable_high_contrast) {
    gdk_win32_display_add_filter (GDK_WIN32_DISPLAY (display),
                                  system_colors_filter,
                                  self);
    self->added_filter = TRUE;
  }

#ifdef HAS_WINRT
  if (enable_color_scheme && SUCCEEDED (init_winrt_settings (self)))
    found_color_scheme = TRUE;
#endif

  if (enable_high_contrast)
    system_colors_changed (self);

  adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                  found_color_scheme,
                                  enable_high_contrast);

  return ADW_SETTINGS_IMPL (self);
}
