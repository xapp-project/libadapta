Title: Migrating to Adaptive Dialogs
Slug: migrating-to-adaptive-dialogs

# Migrating to Adaptive Dialogs

Libadapta 1.5 introduces [class@Dialog] and replacements for
[class@MessageDialog], [class@PreferencesWindow] and [class@AboutWindow] that
derive from it.

While the old widgets are not deprecated yet, they will be in the next release.

# Use `AdapWindow` or `AdapApplicationWindow` for the Parent Window

To use [class@Dialog], your parent window must be an instance of either
[class@Window] or [class@ApplicationWindow]. You should migrate to these widgets
if you're still using [class@Gtk.Window] or [class@Gtk.ApplicationWindow],
respectively.

Stop using window titlebar, instead, create an [class@ToolbarView] and add the
titlebar to it as a top bar. Set the window child as the toolbar view content.
Set the toolbar view as the [property@Window:content] property on the window.

If you have [class@Gtk.SearchBar], [class@Gtk.ActionBar] etc in a
[class@Gtk.Box], move them to the toolbar view as well as top/bottom bars
respectively. 

Example:

```xml
<object class="GtkWindow">
  <property name="titlebar">
    <object class="AdapHeaderBar"/>
  </property>
  <property name="child">
    <object class="GtkBox">
      <property name="orientation">vertical</property>
      <child>
        <!-- content -->
      </child>
      <child>
        <object class="GtkActionBar"/>
      </child>
    </object>
  </property>
</object>
```

becomes this:

```xml
<object class="AdapWindow">
  <property name="content">
    <object class="AdapToolbarView">
      <child type="top">
        <object class="AdapHeaderBar"/>
      </child>
      <property name="content">
        <!-- content -->
      </property>
      <child type="bottom">
        <object class="GtkActionBar"/>
      </child>
    </object>
  </property>
</object>
```

# Porting to `AdapDialog`

The API of [class@Dialog] is somewhat like a subset of the API of [class@Window].

`GtkWindow`                          | `AdapDialog`
-------------------------------------|---------------------------------
[property@Gtk.Window:child]          | [property@Dialog:child]
[property@Gtk.Window:title]          | [property@Dialog:title]
[property@Gtk.Window:default-width]  | [property@Dialog:content-width]
[property@Gtk.Window:default-height] | [property@Dialog:content-height]
[property@Gtk.Window:resizable]      | [property@Dialog:follows-content-size]
[property@Gtk.Window:focus-widget]   | [property@Dialog:focus-widget]
[property@Gtk.Window:default-widget] | [property@Dialog:default-widget]
[method@Gtk.Window.close]            | [method@Dialog.close]
[method@Gtk.Window.destroy]          | [method@Dialog.force_close]
[method@Gtk.Window.present]          | [method@Dialog.present]

`AdapWindow`                          | `AdapDialog`
-------------------------------------|---------------------------------
[property@Window:content]            | [property@Dialog:child]
[property@Window:current-breakpoint] | [property@Dialog:current-breakpoint]
[method@Window.add_breakpoint]       | [method@Dialog.add_breakpoint]

Since these dialogs are within the parent window, they are always destroyed with
parent, so there's no replacement for [property@Gtk.Window:destroy-with-parent].

[class@Gtk.Window] sizing can behave in two ways: when resizable, its size is
controlled with [property@Gtk.Window:default-width] and
[property@Gtk.Window:default-height]. If the content shrinks, the window keeps
the size it had before. If the content grows, the window grows with it. When not
resizable, it follows the content's size precisely.

While dialogs are not resizable, they can behave in both ways as well. By
default, their size is controlled with [property@Dialog:content-width] and
[property@Dialog:content-height], like for resizable windows. Set
[property@Dialog:follows-content-size] to `FALSE` to follow the content size
precisely, like for non-resizable windows.

Since the dialog size is constrained by the parent window, you may want to
adjust the content sizes. For example, if it had a small default height just so
it doesn't grow larger than its parent window, that can be increased or removed.

Dialogs are regular widgets, so they can be closed and presented again endlessly
as long as you hold a reference to them. If you were setting the
[property@Gtk.Window:hide-on-close] property to `TRUE`, do that instead.

Dialogs are always modal. The [property@Gtk.Window:transient-for] property is
replaced by a parameter in [method@Dialog.present].

::: tip
    The widget passed into `adap_dialog_present()` doesn't have to be a window.
    You can pass any widget within a window to get the same effect as passing
    the window itself. This is done both for convenience and to allow for more
    flexibility in future.

[signal@Gtk.Window::close-request] doesn't have a direct replacement. When
presented as a bottom sheet, dialogs can be swiped down, and because of that you
have to specify whether the dialog can be closed right now ahead of time.

- If you were using `::close-request` to completely prevent window closing, set
the [property@Dialog:can-close] property to `FALSE`.
- To have a close confirmation, use the [signal@Dialog::close-attempt] signal.
- [method@Dialog.force_close] closes the dialog even when `:can-close` is set to
  `FALSE`, and can be used to close the dialog after the confirmation.
- If you were using `::close-request` just to track when the window is closed,
  use the [signal@Dialog::closed] signal instead.

## Use `AdapHeaderBar` Instead of `GtkHeaderBar`

[class@HeaderBar] will use the dialog's title and adjust decoration layout to
always include a close button and nothing else. Since [class@Gtk.HeaderBar]
doesn't do this, the dialog will not behave correctly.

Replace your `GtkHeaderBar` with `AdapHeaderBar`, and
[property@Gtk.HeaderBar:show-title-buttons] with a combination of
[property@HeaderBar:show-start-title-buttons] and
[property@HeaderBar:show-end-title-buttons].

## Remove Close Shortcut

`AdapDialog` can be closed by pressing <kbd>Esc</kbd>. If you were handling that
manually, you can stop doing it.

# Port `AdapAboutWindow` to `AdapAboutDialog`

[class@AboutDialog] has identical API to [class@AboutWindow], so just replace
the function calls as appropriate - for example, [method@AboutWindow.add_link]
with [method@AboutDialog.add_link] and so on.

# Port `AdapPreferencesWindow` to `AdapPreferencesDialog`

[class@PreferencesDialog] has very similar API to [class@PreferencesWindow],
and only needs a few adjustments.

## Stop Using Deprecated API

The deprecated subpage API has been removed. Refer to the [breakpoints migration
guide](migrating-to-breakpoints.html#migrate-adappreferenceswindow-subpages) for
information on how to replace it.

## Adapt to Search Changes

`AdapPreferencesDialog` changes the default of the
[property@PreferencesWindow:search-enabled] property to `FALSE`.

If you want search, make sure to set it to `TRUE` manually, and if you were
disabling it, you can stop doing so.

Otherwise, replace the function calls as appropriate, for example
[method@PreferencesWindow.add_toast] with [method@PreferencesDialog.add_toast]
and so on.

# Port `AdapMessageDialog` to `AdapAlertDialog`

[class@AlertDialog] has mostly similar API to [class@MessageDialog].

## Adapt to Constructor Changes

[ctor@AlertDialog.new] doesn't accept a parent window anymore, so remove that
parameter. Instead, pass it as a parameter to [method@Dialog.present] or
[method@AlertDialog.choose].

## Adapt to `adap_alert_dialog_choose()` Changes

Just like [method@Dialog.present], [method@AlertDialog.choose] now takes the
parent widget as a parameter.

::: tip
    The widget passed into `adap_alert_dialog_choose()` doesn't have to be a
    window. You can pass any widget within a window to get the same effect as
    passing the window itself. This is done both for convenience and to allow
    for more flexibility in future.

## Stop Using `adap_message_dialog_response()`

[method@MessageDialog.response] has no replacement. Most applications shouldn't
be using it in the first place, but if you really do, emit the
[signal@AlertDialog::response] signal manually instead.

Otherwise, replace the function calls as appropriate, for example
[method@MessageDialog.add_response] with [method@AlertDialog.add_response]
and so on.

## Adapt to Scrolling

`AdapAlertDialog` can scroll its content. If you were setting a
[class@Gtk.ScrolledWindow] as [property@MessageDialog:extra-child], you may want
to remove the scrolled window and use its contents directly.
