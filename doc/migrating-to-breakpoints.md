Title: Migrating to Breakpoints
Slug: migrating-to-breakpoints

# Migrating to Breakpoints

Libadapta 1.4 introduces [class@Breakpoint] and new adaptive widgets that can
integrate with it, while deprecating the old widgets.

# Start Using `AdapToolbarView`

[class@ToolbarView] is a widget that can be used instead of [class@Gtk.Box] for
widgets like [class@HeaderBar]. It's required to have the correct header bar
style, especially in sidebars.

Replace your `GtkBox` with `AdapToolbarView` and add your header bar and other
toolbars (e.g. [class@Gtk.SearchBar] or [class@TabBar]) as top bars using
[method@ToolbarView.add_top_bar] or `<child type="top">` in a UI file.

If you have any toolbars at the bottom of the window, e.g. [class@Gtk.ActionBar]
or [class@Adap.ViewSwitcherBar], add them using
[method@ToolbarView.add_bottom_bar] or `<child type="bottom">`.

Finally, use [property@ToolbarView:content] to set the content widget. Unlike in
`GtkBox`, there's no need to set [property@Gtk.Widget:vexpand] to `TRUE`, so
that can be removed.

Example:

```xml
<object class="GtkBox">
  <property name="orientation">vertical</property>
  <child>
    <object class="AdapHeaderBar">
      <!-- ... -->
    </object>
  </child>
  <child>
    <object class="...">
      <property name="vexpand">True</property>
      <!-- ... -->
    </object>
  </child>
</object>
```

becomes this:

```xml
<object class="AdapToolbarView">
  <child type="top">
    <object class="AdapHeaderBar">
      <!-- ... -->
    </object>
  </child>
  <property name="content">
    <object class="...">
      <!-- ... -->
    </object>
  </property>
</object>
```

`AdapToolbarView` defaults to flat header bars, replacing the `.flat` style
class. To use a raised style, set the [property@ToolbarView:top-bar-style]
and/or [property@ToolbarView:bottom-bar-style] properties to
`ADAP_TOOLBAR_RAISED`.

Subsequent sections will assume that you're using `AdapToolbarView`.

# Start Using `AdapWindow` or `AdapApplicationWindow`

All of the new adaptive widgets require using breakpoints, which can only be
added into [class@Window] or [class@ApplicationWindow]. Alternatively, they can
be added into [class@BreakpointBin] when only a specific part of the window
needs to be adaptive.

To migrate from `GtkWindow` to `AdapWindow`, or from `GtkApplicationWindow` to
`AdapApplicationWindow`, put your header bar and window content into an
`AdapToolbarView` and set that as the window's content.

Example:

```xml
<object class="GtkWindow">
  <property name="titlebar">
    <!-- titlebar -->
  </property>
  <property name="child">
    <!-- content -->
  </property>
</object>
```

becomes this:

```xml
<object class="AdapWindow">
  <property name="content">
    <object class="AdapToolbarView">
      <child type="top">
        <!-- titlebar -->
      </child>
      <property name="content">
        <!-- content -->
    </object>
  </property>
</object>
```

Using breakpoints also requires the window to have a minimum size. It can be set
using [method@Gtk.Widget.set_size_request], or the
[property@Gtk.Widget:width-request] and [property@Gtk.Widget:height-request]
properties.

The target window size that would work on phones both in portrait and landscape
orientations is 360x294 pixels:

```xml
<object class="AdapWindow">
  <property name="width-request">360</property>
  <property name="height-request">294</property>
  <property name="content">
    <object class="AdapToolbarView">
      <child type="top">
        <!-- titlebar -->
      </child>
      <property name="content">
        <!-- content -->
    </object>
  </property>
</object>
```

The child widget must completely fit into your minimum size. If it doesn't, then
as soon as you add a breakpoint and resize the window to that size, the contents
will overflow and a warning message will be printed.

When checking if the contents fit, consider translations and text scale factor
changes. Make sure to leave enough space for text labels, and enable ellipsizing
or wrapping if they might not fit.

For [class@Gtk.Label] this can be done via [property@Gtk.Label:ellipsize], or
via [property@Gtk.Label:wrap] together with [property@Gtk.Label:wrap-mode].

For buttons, use [property@Gtk.Button:can-shrink],
[property@Gtk.MenuButton:can-shrink], [property@Adap.SplitButton:can-shrink], or
[property@Adap.ButtonContent:can-shrink].

Subsequent sections will assume that you're using `AdapWindow` or
`AdapApplicationWindow`.

# Start using `AdapHeaderBar`

[class@HeaderBar] provides additional integration with some of the widgets
listed below, compared to [class@Gtk.HeaderBar], and can make them noticeably
easier to use.

```xml
<object class="GtkHeaderBar">
  <property name="show-title-buttons">False</property>
</object>
```

Replace your header bar with `AdapHeaderBar`, and
[property@Gtk.HeaderBar:show-title-buttons] with a combination of
[property@HeaderBar:show-start-title-buttons] and
[property@HeaderBar:show-end-title-buttons].

```xml
<object class="AdapHeaderBar">
  <property name="show-start-title-buttons">False</property>
  <property name="show-end-title-buttons">False</property>
</object>
```

# Replace `AdapLeaflet`

`AdapLeaflet` can be replaced by either [class@NavigationView] or
[class@NavigationSplitView], depending on how it's used.

## Navigation

Leaflets that have their [property@Leaflet:can-unfold] property set to `FALSE`
can be replaced using `AdapNavigationView`.

A typical use case with two pages looks as follows:

```xml
<object class="AdapLeaflet">
  <property name="can-unfold">False</property>
  <property name="can-navigate-back">True</property>
  <child>
    <object class="AdapLeafletPage">
      <property name="name">page-1</property>
      <property name="child">
        <object class="AdapToolbarView">
          <child type="top">
            <object class="GtkHeaderBar">
              <property name="title-widget">
                <object class="AdapWindowTitle">
                  <property name="title" translatable="yes">Page 1</property>
                </object>
              </property>
            </object>
          </child>
          <property name="content">
            <!-- ... -->
          </property>
        </object>
      </property>
    </object>
  </child>
  <child>
    <object class="AdapLeafletPage">
      <property name="name">page-2</property>
      <property name="child">
        <object class="AdapToolbarView">
          <child type="top">
            <object class="GtkHeaderBar">
              <child type="start">
                <object class="GtkButton">
                  <property name="icon-name">go-previous-symbolic</property>
                  <property name="tooltip-text" translatable="yes">Back</property>
                  <signal name="clicked" handler="back_clicked_cb" swapped="yes"/>
                </object>
              </child>
              <property name="title-widget">
                <object class="AdapWindowTitle">
                  <property name="title" translatable="yes">Page 2</property>
                </object>
              </property>
            </object>
          </child>
          <property name="content">
            <!-- ... -->
          </property>
        </object>
      </property>
    </object>
  </child>
</object>
```

Replace the leaflet with `AdapNavigationView` and `AdapLeafletPage` with
[class@NavigationPage]. `AdapNavigationView` requires all children to be
`AdapNavigationPage`, so if you didn't explicitly specify `AdapLeafletPage`,
wrap your child widgets into `AdapNavigationPage` instead.

Use [property@NavigationPage:title] instead of manually adding titles to your
header bars. Even if you need a subtitle, [property@NavigationPage:title] should
always be set to something meaningful, as it will be used as a tooltip for the
back button and label in its context menu, as well as read out by the screen
reader.

Replace [property@LeafletPage:name] with [property@NavigationPage:tag] and
remove the back button as [class@HeaderBar] already provides one. If back
buttons are unwanted, set [property@HeaderBar:show-back-button] on your header
bar to `FALSE`.

`AdapNavigationView` always has back gestures and shortcuts enabled. To disable
them (as well as the back button) on a specific page, set its
[property@NavigationPage:can-pop] property to `FALSE`.

If you're using [class@Gtk.HeaderBar], replace it with `AdapHeaderBar` as well:

```xml
<object class="AdapNavigationView">
  <child>
    <object class="AdapNavigationPage">
      <property name="title" translatable="yes">Page 1</property>
      <property name="tag">page-1</property>
      <property name="child">
        <object class="AdapToolbarView">
          <child type="top">
            <object class="AdapHeaderBar"/>
          </child>
          <property name="content">
            <!-- ... -->
          </property>
        </object>
      </property>
    </object>
  </child>
  <child>
    <object class="AdapNavigationPage">
      <property name="title" translatable="yes">Page 2</property>
      <property name="tag">page-2</property>
      <property name="child">
        <object class="AdapToolbarView">
          <child type="top">
            <object class="AdapHeaderBar"/>
          </child>
          <property name="content">
            <!-- ... -->
          </property>
        </object>
      </property>
    </object>
  </child>
</object>
```

Replace [method@Leaflet.navigate] calls with
[method@NavigationView.push] for `ADAP_NAVIGATION_DIRECTION_FORWARD` and
[method@NavigationView.pop] for `ADAP_NAVIGATION_DIRECTION_BACK`. It's also
possible to push a page using the `navigation.push` action and the page's tag as
parameter, or pop the visible page using the `navigation.pop` action.

If you were using nested `AdapLeaflet`s or [class@Gtk.Stack] with pages inside it
to arrange a non-linear navigation structure (for example, page 1 that can lead
to page 2 and page 3, both of which lead back to page 1), `AdapNavigationView`
can support that directly.

To replace [property@Leaflet:can-navigate-forward], connect to the
[signal@NavigationView::get-next-page] signal.

## Sidebar

Leaflets that implement adaptive sidebar layouts can be replaced with
[class@NavigationSplitView].

A typical use case looks as follows:

```xml
<object class="AdapWindow">
  <property name="content">
    <object class="AdapLeaflet" id="leaflet">
      <property name="can-navigate-back">True</property>
      <child>
        <object class="AdapToolbarView">
          <child type="top">
            <object class="AdapHeaderBar">
              <property name="show-end-title-buttons"
                              bind-object="leaflet"
                              bind-property="folded"
                              bind-flags="sync-create"/>
              <property name="title-widget">
                <object class="AdapWindowTitle">
                  <property name="title" translatable="yes">Sidebar</property>
                </object>
              </property>
            </object>
          </child>
          <property name="content">
            <!-- sidebar -->
          </property>
        </object>
      </child>
      <child>
        <object class="AdapLeafletPage">
          <property name="navigatable">False</property>
          <property name="child">
            <object class="GtkSeparator"/>
          </property>
        </object>
      </child>
      <child>
        <object class="AdapToolbarView">
          <property name="hexpand">True</property>
          <child type="top">
            <object class="AdapHeaderBar">
              <property name="show-start-title-buttons"
                              bind-object="leaflet"
                              bind-property="folded"
                              bind-flags="sync-create"/>
              <child type="start">
                <object class="GtkButton">
                  <property name="visible"
                                  bind-object="leaflet"
                                  bind-property="folded"
                                  bind-flags="sync-create"/>
                  <property name="icon-name">go-previous-symbolic</property>
                </object>
              </child>
              <property name="title-widget">
                <object class="AdapWindowTitle">
                  <property name="title" translatable="yes">Content</property>
                </object>
              </property>
            </object>
          </child>
          <property name="content">
            <!-- content -->
          </property>
        </object>
      </child>
    </object>
  </property>
</object>
```

Replace `AdapLeaflet` with `AdapNavigationSplitView` and `AdapLeafletPage` with
[class@NavigationPage]. `AdapNavigationSplitView` requires all children to be
`AdapNavigationPage`, so if you didn't explicitly specify `AdapLeafletPage`,
wrap your child widgets into `AdapNavigationPage` instead.

Remove the separator child, `AdapNavigationSplitView` provides a separator as
part of its styling.

Stop binding window button visibility to the leaflet's folded state, as
`AdapHeaderBar` handles that automatically when inside `AdapNavigationSplitView`.

Use [property@NavigationPage:title] instead of manually adding titles to your
header bars. Even if you need a subtitle, [property@NavigationPage:title] should
always be set to something meaningful, as it will be used as a tooltip for the
back button, as well as read out by the screen reader.

Replace [property@LeafletPage:name] with [property@NavigationPage:tag] and
remove the back button from your content header bar, as [class@HeaderBar]
already provides one. If back buttons are unwanted, set
[property@HeaderBar:show-back-button] on your header bar to `FALSE`.

It's also possible to disable the back button, as well as related shortcuts and
actions on the content page by setting its [property@NavigationPage:can-pop]
property to `FALSE`.

Add a breakpoint with a `max-width` condition to your window. To accommodate
the Large Text setting, the width value should be using `sp` unit, e.g. `400sp`.
Add a setter to your breakpoint, setting the
[property@NavigationSplitView:collapsed] property to `TRUE`.

```xml
<object class="AdapWindow">
  <property name="width-request">360</property>
  <property name="height-request">200</property>
  <child>
    <object class="AdapBreakpoint">
      <condition>max-width: 400sp</condition>
      <setter object="split_view" property="collapsed">True</setter>
    </object>
  </child>
  <property name="content">
    <object class="AdapNavigationSplitView" id="split_view">
      <property name="sidebar">
        <object class="AdapNavigationPage">
          <property name="title" translatable="yes">Sidebar</property>
          <property name="child">
            <object class="AdapToolbarView">
              <child type="top">
                <object class="AdapHeaderBar"/>
              </child>
              <property name="content">
                <!-- sidebar -->
              </property>
            </object>
          </property>
        </object>
      </property>
      <property name="content">
        <object class="AdapNavigationPage">
          <property name="title" translatable="yes">Content</property>
          <property name="child">
            <object class="AdapToolbarView">
              <child type="top">
                <object class="AdapHeaderBar"/>
              </child>
              <property name="content">
                <!-- content -->
              </property>
            </object>
          </property>
        </object>
      </property>
    </object>
  </property>
</object>
```

Replace [property@Leaflet:visible-child] and/or
[property@Leaflet:visible-child-name] uses with
[property@NavigationSplitView:show-content]. It's also
possible to show content using using the `navigation.push` action with the
content page's tag as a parameter, or show sidebar using the `navigation.pop`
action.

By default, `AdapNavigationSplitView` will dynamically resize the sidebar
depending on its own width. If that behavior is unwanted, set
[property@NavigationSplitView:min-sidebar-width] and
[property@NavigationSplitView:max-sidebar-width] to 0.

For triple-pane layouts, see [the adaptive layouts page](adaptive-layouts.html#triple-pane-layouts).

## Other Uses

Other uses of `AdapLeaflet,` e.g. in vertical orientation, have no direct
replacement, but can be replicated using e.g. [class@Gtk.Box], the
[property@Gtk.Widget:visible] property and breakpoints.

# Replace `AdapFlap`

`AdapFlap` can be used for multiple different things, and is generally replaced
with [class@OverlaySplitView]. That widget has similar API to `AdapFlap` and its
common uses can be directly replaced.

## Utility pane

The main `AdapFlap` use case is
[utility panes](https://developer.gnome.org/hig/patterns/containers/utility-panes.html).

A common case looks as follows:

```xml
<object class="AdapWindow">
  <property name="content">
    <object class="AdapToolbarView">
      <property name="top-bar-style">raised</property>
      <child type="top">
        <object class="AdapHeaderBar">
          <object class="GtkToggleButton" id="toggle_pane_button">
            <property name="icon-name">sidebar-show-symbolic</property>
            <property name="active">True</property>
          </object>
        </object>
      </child>
      <property name="content">
        <object class="AdapFlap">
          <property name="reveal-flap"
                    bind-source="toggle_pane_button"
                    bind-property="active"
                    bind-flags="sync-create|bidirectional"/>
          <property name="flap">
            <!-- utility pane -->
          </property>
          <property name="separator">
            <object class="GtkSeparator"/>
          </property>
          <property name="content">
            <!-- main view -->
          </property>
        </object>

      </property>
    </object>
  </property>
</object>
```

Replace `AdapFlap` with `AdapOverlaySplitView`, the `flap` property with
[property@OverlaySplitView:sidebar] and the `reveal-flap` property with
[property@OverlaySplitView:show-sidebar].

Remove the separator child, `AdapOverlaySplitView` provides a separator as part
of its styling.

Add an ID to your split view if you're using UI files.

Add a breakpoint with a `max-width` condition to your window. To accommodate
the Large Text setting, the width value should be using `sp` unit, e.g. `400sp`.
Add a setter to your breakpoint, setting the
[property@OverlaySplitView:collapsed] property to `TRUE`.

```xml
<object class="AdapWindow">
  <property name="width-request">360</property>
  <property name="height-request">200</property>
  <child>
    <object class="AdapBreakpoint">
      <condition>max-width: 400sp</condition>
      <setter object="split_view" property="collapsed">True</setter>
    </object>
  </child>
  <property name="content">
    <object class="AdapToolbarView">
      <property name="top-bar-style">raised</property>
      <child type="top">
        <object class="AdapHeaderBar">
          <object class="GtkToggleButton" id="toggle_pane_button">
            <property name="icon-name">sidebar-show-symbolic</property>
            <property name="active">True</property>
          </object>
        </object>
      </child>
      <property name="content">
        <object class="AdapOverlaySplitView" id="split_view">
          <property name="show-sidebar"
                    bind-source="toggle_pane_button"
                    bind-property="active"
                    bind-flags="sync-create|bidirectional"/>
          <property name="sidebar">
            <!-- utility pane -->
          </property>
          <property name="content">
            <!-- main view -->
          </property>
        </object>

      </property>
    </object>
  </property>
</object>
```

The [property@Flap:locked] property can be replaced with
[property@OverlaySplitView:pin-sidebar].

## Sidebar

Sidebar-style `AdapFlap` use cases, with split header bars, are handled similarly
to utility panes. One additional difference is that, like in
`AdapNavigationSplitView`, `AdapHeaderBar` can manage window button visibility
automatically when using `AdapOverlaySplitView`, so there's no need to bind their
visibility to the [property@Flap:folded] property anymore.

While `AdapOverlaySplitView` doesn't require using [class@NavigationPage], it can
still be used to provide header bar titles insead of using [class@WindowTitle].

## Fullscreen Header Bar

A vertical `AdapFlap` can also be used to handle header bars in fullscreen,
specifically for either overlaying a header bar above content in fullscreen,
or showing it next to the content otherwise.

This can be replaced with [class@ToolbarView] and the
[property@ToolbarView:extend-content-to-top-edge] property.

## Other Uses

Other use cases of `AdapFlap` have no direct replacement but can be replicated
using e.g. [class@Gtk.Box], [class@Gtk.Revealer], the
[property@Gtk.Widget:visible] property, and breakpoints.

# Replace `AdapViewSwitcherTitle`

`AdapViewSwitcherTitle` can be replaced by using [class@ViewSwitcher] together
with a breakpoint.

A typical `AdapViewSwitcherTitle` use looks as follows:

```xml
<object class="AdapWindow">
  <property name="width-request">360</property>
  <property name="height-request">294</property>
  <property name="content">
    <object class="AdapToolbarView">
      <child type="top">
        <object class="AdapHeaderBar">
          <property name="centering-policy">strict</property>
          <property name="title-widget">
            <object class="AdapViewSwitcherTitle" id="title">
              <property name="stack">stack</property>
              <property name="title" translatable="yes">Title</property>
            </object>
          </property>
        </object>
      </child>
      <property name="content">
        <object class="AdapViewStack" id="stack">
          <!-- ... -->
        </object>
      </property>
      <child type="bottom">
        <object class="AdapViewSwitcherBar">
          <property name="stack">stack</property>
          <property name="reveal"
                    bind-source="title"
                    bind-property="title-visible"
                    bind-flags="sync-create"/>
        </object>
      </child>
    </object>
  </property>
</object>
```

Replace your `AdapViewSwitcherTitle` with a regular [class@ViewSwitcher] and set
the `ADAP_VIEW_SWITCHER_POLICY_WIDE` policy on it.

Also remove the [property@ViewSwitcherBar:reveal] property binding on your
switcher bar, and stop setting [property@HeaderBar:centering-policy] on your
header bar.

Add an ID to your header bar and switcher bar if you're using UI files.

```xml
<object class="AdapHeaderBar" id="header_bar">
  <!-- ... -->
  <property name="title-widget">
    <object class="AdapViewSwitcher">
      <property name="policy">wide</property>
      <property name="stack">stack</property>
    </object>
  </property>
  <!-- ... -->
</object>
<!-- ... -->
<object class="AdapViewSwitcherBar" id="switcher_bar">
  <property name="stack">stack</property>
</object>
```

Add a breakpoint with a `max-width` condition to your window. To accommodate
the Large Text setting, the width value should be using `sp` unit.

Recommended width values depending on the number of pages:

- 2 pages: 400sp
- 3 pages: 450sp
- 4 pages: 550sp

You may need to tweak the value to match your application's layout. It needs to
be large enough that the view switcher is not yet ellipsized at that width, but
small enough that the view switcher doesn't move to the bottom yet at common
window sizes.

The rest of this section will assume 550sp as the threshold.

Add two setters to your breakpoint, unsetting the header bar's titlebar widget,
and setting the switcher bar's `reveal` property to `TRUE`.

Specify the header bar's title using [property@Gtk.Window:title] or
[property@NavigationPage:title].

```xml
<object class="AdapWindow">
  <property name="title" translatable="yes">Title</property>
  <child>
    <object class="AdapBreakpoint">
      <condition>max-width: 550sp</condition>
      <setter object="header_bar" property="title-widget"/>
      <setter object="switcher_bar" property="reveal">True</setter>
    </object>
  </child>
  <property name="child">
    <object class="AdapToolbarView">
      <child type="top">
        <object class="AdapHeaderBar" id="header_bar">
          <property name="title-widget">
            <object class="AdapViewSwitcher">
              <property name="policy">wide</property>
              <property name="stack">stack</property>
            </object>
          </property>
        </object>
      </child>
      <property name="content">
        <object class="AdapViewStack" id="stack">
          <!-- ... -->
        </object>
      </property>
      <child type="bottom">
        <object class="AdapViewSwitcherBar" id="switcher_bar">
          <property name="stack">stack</property>
        </object>
      </child>
    </object>
  </property>
</object>
```

## Subtitle

If you were using [property@ViewSwitcherTitle:subtitle], use a [class@Gtk.Stack]
containing an `AdapViewSwitcher` and [class@WindowTitle], and switch the stack's
visible page from your breakpoint.

```xml
<object class="AdapWindow">
  <child>
    <object class="AdapBreakpoint">
      <condition>max-width: 550sp</condition>
      <setter object="title_stack" property="visible-child">window_title</setter>
      <setter object="switcher_bar" property="reveal">True</setter>
    </object>
  </child>
  <property name="child">
    <object class="AdapToolbarView">
      <child type="top">
        <object class="AdapHeaderBar">
          <property name="title-widget">
            <object class="GtkStack" id="title_stack">
              <child>
                <object class="AdapViewSwitcher">
                  <property name="policy">wide</property>
                  <property name="stack">stack</property>
                </object>
              </child>
              <child>
                <object class="AdapWindowTitle" id="window_title">
                  <property name="title" translatable="yes">Title</property>
                  <property name="subtitle" translatable="yes">Subtitle</property>
                </object>
              </child>
            </object>
          </property>
        </object>
      </child>
      <property name="content">
        <object class="AdapViewStack" id="stack">
          <!-- ... -->
        </object>
      </property>
      <child type="bottom">
        <object class="AdapViewSwitcherBar" id="switcher_bar">
          <property name="stack">stack</property>
        </object>
      </child>
    </object>
  </property>
</object>
```

# Replace `AdapSqueezer`

`AdapSqueezer` can be replaced with breakpoints.

For example, to migrate the following example:

```xml
<object class="AdapSqueezer">
  <property name="homogeneous">False</property>
  <child>
    <object class="GtkBox">
      <property name="spacing">6</property>
      <child>
        <object class="GtkButton">
          <property name="label">Button 1</property>
        </object>
      </child>
      <child>
        <object class="GtkButton">
          <property name="label">Button 2</property>
        </object>
      </child>
    </object>
  </child>
  <child>
    <object class="GtkBox">
      <property name="orientation">vertical</property>
      <property name="spacing">6</property>
      <child>
        <object class="GtkButton">
          <property name="label">Button 1</property>
        </object>
      </child>
      <child>
        <object class="GtkButton">
          <property name="label">Button 2</property>
        </object>
      </child>
    </object>
  </child>
</object>
```

use a single [class@Gtk.Box], and a breakpoint toggling the box's
[property@Gtk.Orientable:orientation] property:

```xml
<object class="GtkBox" id="box">
  <property name="spacing">6</property>
  <child>
    <object class="GtkButton">
      <property name="label">Button 1</property>
    </object>
  </child>
  <child>
    <object class="GtkButton">
      <property name="label">Button 2</property>
    </object>
  </child>
</object>

<!-- ... -->

<object class="AdapBreakpoint">
  <condition>max-width: 400sp</condition>
  <setter object="box" property="orientation">vertical</setter>
</object>
```

Alternatively, a [class@Gtk.Stack] or the [property@Gtk.Widget:visible]
property can be used to switch between separate widgets, like in `AdapSqueezer`.

# Migrate `AdapPreferencesWindow` Subpages

`AdapPreferencesWindow` can now use `AdapNavigationView` for its subpages instead
of `AdapLeaflet`. As such, [method@PreferencesWindow.present_subpage] and
[method@PreferencesWindow.close_subpage] have been deprecated in favor of
[method@PreferencesWindow.push_subpage] and
[method@PreferencesWindow.pop_subpage].

Wrap your subpages into [class@NavigationPage], stop setting the header bar
titles manually and use [property@NavigationPage:title] instead, remove your
back buttons if you had any. If back buttons are unwanted, set
[property@HeaderBar:show-back-button] on your header bar to `FALSE`.

`AdapPreferencesWindow` always has back gestures and shortcuts enabled for the
new subpage API. To disable them (as well as the back button) on a specific
page, set its [property@NavigationPage:can-pop] property to `FALSE`.

Unlike [method@PreferencesWindow.present_subpage], repeatedly calling
[method@PreferencesWindow.push_subpage] with different pages won't replace the
current page, so if you had an [class@Leaflet] inside your subpage, you can use
`adap_preferences_window_push_subpage()` directly instead.
