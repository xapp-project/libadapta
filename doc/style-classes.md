Title: Style Classes
Slug: style-classes

# Style Classes

The Adapta stylesheet provides a number of style classes. They can be applied
to widgets to change their appearance.

# Buttons

The following style classes can be applied to [class@Gtk.Button] to change its
appearance.

## Suggested Action

<picture>
  <source srcset="buttons-suggested-action-dark.png" media="(prefers-color-scheme: dark)">
  <img src="buttons-suggested-action.png" alt="buttons-suggested-action">
</picture>

The `.suggested-action` style class makes the button use accent colors. It can
be used to denote important buttons, for example, the affirmative button in an
action dialog.

It can be used in combination with [`.circular`](#circular) or [`.pill`](#pill).

Can also be used with [class@Gtk.MenuButton] or [class@SplitButton].

## Destructive Action

<picture>
  <source srcset="buttons-destructive-action-dark.png" media="(prefers-color-scheme: dark)">
  <img src="buttons-destructive-action.png" alt="buttons-destructive-action">
</picture>

The `.destructive-action` style class makes the button use destructive colors.
It can be used to draw attention to the potentially damaging consequences of
using a button. This style acts as a warning to the user.

It can be used in combination with [`.circular`](#circular) or [`.pill`](#pill).

Can also be used with [class@Gtk.MenuButton] or [class@SplitButton].

## Flat

<picture>
  <source srcset="buttons-flat-dark.png" media="(prefers-color-scheme: dark)">
  <img src="buttons-flat.png" alt="buttons-flat">
</picture>

The `.flat` style class makes the button use flat appearance, looking like a
label or an icon until hovered.

Button inside [toolbars and similar widgets](#toolbars) appear flat by default.

It can be used in combination with [`.circular`](#circular) or [`.pill`](#pill).

Can also be used with [class@Gtk.MenuButton] or [class@SplitButton].

Can be set via [property@Gtk.Button:has-frame] and
[property@Gtk.MenuButton:has-frame].

## Raised

<picture>
  <source srcset="buttons-raised-dark.png" media="(prefers-color-scheme: dark)">
  <img src="buttons-raised.png" alt="buttons-raised">
</picture>

The `.raised` style class makes the button use the regular appearance instead of
the flat one.

This style class is only useful inside [toolbars and similar widgets](#toolbars).

It can be used in combination with [`.circular`](#circular) or [`.pill`](#pill).

Can also be used with [class@Gtk.MenuButton] or [class@SplitButton].

## Opaque

<picture>
  <source srcset="buttons-opaque-dark.png" media="(prefers-color-scheme: dark)">
  <img src="buttons-opaque.png" alt="buttons-opaque">
</picture>

The `.opaque` style class gives the button an opaque background. It's intended
to be used together with custom styles that override `background-color` and
`color`, to create buttons with an appearance similar to
[`.suggested-action`](#suggested-action) and
[`.destructive-action`](#destructive-action), but with custom colors.

For example, `.suggested-action` and `.destructive-action` are equivalent to
using the `.opaque` style
class with the following CSS:

```css
#custom-suggested-action-button {
  background-color: @accent_bg_color;
  color: @accent_fg_color;
}

#custom-destructive-action-button {
  background-color: @destructive_bg_color;
  color: @destructive_fg_color;
}
```

It can be used in combination with [`.circular`](#circular) or [`.pill`](#pill).

Can also be used with [class@Gtk.MenuButton] or [class@SplitButton].

## Circular

<picture>
  <source srcset="buttons-circular-dark.png" media="(prefers-color-scheme: dark)">
  <img src="buttons-circular.png" alt="buttons-circular">
</picture>

The `.circular` style class makes the button round. It can be used with buttons
containing icons or short labels (1-2 characters).

It can be used in combination with [`.suggested-action`](#suggested-action),
[`.destructive-action`](#destructive-action), [`.flat`](#flat),
[`.raised`](#raised), [`.opaque`](#opaque) or [`.osd`](#overlay-buttons).

Can also be used with [class@Gtk.MenuButton].

## Pill

<picture>
  <source srcset="buttons-pill-dark.png" media="(prefers-color-scheme: dark)">
  <img src="buttons-pill.png" alt="buttons-pill">
</picture>

The `.pill` style class makes the button appear as a pill. It's often used for
important standalone buttons, for example, inside a [class@StatusPage].

It can be used in combination with [`.suggested-action`](#suggested-action),
[`.destructive-action`](#destructive-action), [`.flat`](#flat),
[`.raised`](#raised), [`.opaque`](#opaque) or [`.osd`](#overlay-buttons).

# Linked Controls

<picture>
  <source srcset="linked-controls-dark.png" media="(prefers-color-scheme: dark)">
  <img src="linked-controls.png" alt="linked-controls">
</picture>

The `.linked` style class can be applied to a [class@Gtk.Box] to make its
children appear as a single group. The box must have no spacing.

Linked boxes can be both horizontal and vertical.

The following widgets can be linked:

* [class@Gtk.Button]
* [class@Gtk.MenuButton]
* [class@Gtk.DropDown]
* [class@Gtk.ColorDialogButton]
* [class@Gtk.FontDialogButton]
* [class@TabButton]
* [class@Gtk.Entry]
* [class@Gtk.PasswordEntry]
* [class@Gtk.SearchEntry]
* [class@Gtk.SpinButton]

Linked styles will not work correctly for buttons with the following style
classes:

* [`.flat`](#flat)
* [`.suggested-action`](#suggested-action)
* [`.destructive-action`](#destructive-action)
* [`.opaque`](#opaque)

If a linked box is contained within a [toolbar or a similar widget](#toolbars),
buttons inside it won't get the flat appearance.

# Toolbars

<picture>
  <source srcset="toolbar-dark.png" media="(prefers-color-scheme: dark)">
  <img src="toolbar.png" alt="toolbar">
</picture>

The `.toolbar` style class can be applied to a horizontal [class@Gtk.Box]. The
same appearance is also used by [class@HeaderBar], [class@Gtk.HeaderBar],
[class@Gtk.ActionBar] and [class@Gtk.SearchBar] automatically.

It changes the appearance of buttons inside it to make them flat when possible,
according to the following rules:

The following buttons get flat appearance:

<picture>
  <source srcset="toolbar-flat-dark.png" media="(prefers-color-scheme: dark)">
  <img src="toolbar-flat.png" alt="toolbar-flat">
</picture>

* Icon-only buttons;
* Buttons with an icon and a label (using [class@ButtonContent]);
* Menu buttons containing an arrow;
* [class@SplitButton];
* Any other button with the [`.flat`](#flat) style class.

The following buttons keep default appearance:

<picture>
  <source srcset="toolbar-raised-dark.png" media="(prefers-color-scheme: dark)">
  <img src="toolbar-raised.png" alt="toolbar-raised">
</picture>

* Text-only buttons;
* Buttons with other content;
* Buttons within widgets with the [`.linked`](#linked-controls) style
  class;
* Buttons with the [`.suggested-action`](#suggested-action),
  [`.destructive-action`](#destructive-action) or [`.opaque`](#opaque) style
  classes.
* Buttons with the [`.raised`](#raised) style class.

It also ensures 6px margins and spacing between widgets. The
[`.spacer`](#spacers) style class can be useful to separate groups of widgets.

::: important
    The [property@Gtk.Button:has-frame] property will **not** be set to `FALSE`
    when a button gets the flat appearance automatically. It also cannot be set
    to `TRUE` to make a button raised, the style class should be used directly
    instead.

# Spacers

<picture>
  <source srcset="toolbar-spacer-dark.png" media="(prefers-color-scheme: dark)">
  <img src="toolbar-spacer.png" alt="toolbar-spacer">
</picture>

The `.spacer` style class can be applied to a [class@Gtk.Separator] to make it
appear invisible and act as whitespace. This can be useful with [toolbars and
similar widgets](#toolbars) to separate groups of widgets from each other.

# Dim Labels

<picture>
  <source srcset="dim-label-dark.png" media="(prefers-color-scheme: dark)">
  <img src="dim-label.png" alt="dim-label">
</picture>

The `.dim-label` style class makes the widget it's applied to partially
transparent.

The level of transparency differs between regular and high contrast styles. As
such, it's highly recommended to be used instead of changing opacity manually.

# Typography Styles

These style classes can be applied to any widgets, but are mostly used for
[class@Gtk.Label] or other widgets that contain them.

<picture>
  <source srcset="typography-titles-dark.png" media="(prefers-color-scheme: dark)">
  <img src="typography-titles.png" alt="typography-titles">
</picture>

The `.title-1`, `.title-2`, `.title-3`, `.title-4` classes provide four levels
of title styles, indicating hierarchy. The specific use heavily depends on
context. Generally, the larger styles are intended to be used in bigger views
with plenty of whitespace around them.

<picture>
  <source srcset="typography-heading-dark.png" media="(prefers-color-scheme: dark)">
  <img src="typography-heading.png" alt="typography-heading">
</picture>

The `.heading` style class is the standard style for UI headings using the
default text size, such as window titles or boxed list labels.

<picture>
  <source srcset="typography-body-dark.png" media="(prefers-color-scheme: dark)">
  <img src="typography-body.png" alt="typography-body">
</picture>

The `.body` style class is the default text style.

<picture>
  <source srcset="typography-captions-dark.png" media="(prefers-color-scheme: dark)">
  <img src="typography-captions.png" alt="typography-captions">
</picture>

The `.caption-heading` and `.caption` style classes make text smaller. They
are intended to be used to differentiate sub-text which accompanies text in
the regular body style.

<picture>
  <source srcset="typography-monospace-dark.png" media="(prefers-color-scheme: dark)">
  <img src="typography-monospace.png" alt="typography-monospace">
</picture>

The `.monospace` style class makes the widget use a monospace font. This can be
useful when displaying code, logs or shell commands.

For [class@EntryRow], it only makes the editable part monospace, but not title
or any extra widgets. To make everything in the row monospace, apply
`.monospace` to the [class@Gtk.ListBox] around the row.

<picture>
  <source srcset="typography-numeric-dark.png" media="(prefers-color-scheme: dark)">
  <img src="typography-numeric.png" alt="typography-numeric">
</picture>

The `.numeric` style class makes the widget use tabular figures. This is
equivalent to using [struct@Pango.AttrFontFeatures] with `"tnum=1"` features.
This style is useful in situations where multiple labels are vertically aligned,
or when displaying time, an operation progress or another number that can
quickly change.

# Colors

<picture>
  <source srcset="style-colors-dark.png" media="(prefers-color-scheme: dark)">
  <img src="style-colors.png" alt="style-colors">
</picture>

The following style classes change widget colors:

Class             | Color
----------------- | -------------------------------------------
<tt>.accent</tt>  | [accent color](named-colors.html#accent-colors)
<tt>.success</tt> | [success color](named-colors.html#success-colors)
<tt>.warning</tt> | [warning color](named-colors.html#warning-colors)
<tt>.error</tt>   | [error color](named-colors.html#error-colors)

They can be applied to any widget.

The `.error`, `.warning` and `.success` style classes can be applied to
[class@Gtk.Entry]. In that case, they can be used to indicate input validation
state.

# Boxed Lists & Cards

<picture>
  <source srcset="boxed-lists-dark.png" media="(prefers-color-scheme: dark)">
  <img src="boxed-lists.png" alt="boxed-lists">
</picture>

The `.boxed-list` style class can be applied to a [class@Gtk.ListBox] to make it
a [boxed list](boxed-lists.html). The list box should have
[property@Gtk.ListBox:selection-mode] set to `GTK_SELECTION_NONE`.

<picture>
  <source srcset="cards-dark.png" media="(prefers-color-scheme: dark)">
  <img src="cards.png" alt="cards">
</picture>

The `.card` style class can be applied to any other widget to give it a similar
appearance.

If a widget with the `.card` style class also has the `.activatable` style
class, it will also have hover and active states similar to an activatable row
inside a boxed list.

If the `.card` style class is applied to a [class@Gtk.Button], it will get these
states automatically, without needing the `.activatable` class.

# Sidebars

<picture>
  <source srcset="navigation-sidebar-dark.png" media="(prefers-color-scheme: dark)">
  <img src="navigation-sidebar.png" alt="navigation-sidebar">
</picture>

The `.navigation-sidebar` style class can be applied to a [class@Gtk.ListBox]
or [class@Gtk.ListView] to make it look like a sidebar: it makes the
items rounded and padded, makes selected items use a neutral color instead of
accent, and removes the default list background.

# App Icons

<picture>
  <source srcset="app-icons-dark.png" media="(prefers-color-scheme: dark)">
  <img src="app-icons.png" alt="app-icons">
</picture>

GNOME application icons require a shadow to be legible on a light background.
The `.icon-dropshadow` and `.lowres-icon` style classes provide it when used
with [class@Gtk.Image] or any other widget that contains an image.

`.lowres-icon` should be used for 32×32 or smaller icons, and `.icon-dropshadow`
should be used otherwise.

# Selection Mode Check Buttons

<picture>
  <source srcset="selection-mode-checks-dark.png" media="(prefers-color-scheme: dark)">
  <img src="selection-mode-checks.png" alt="selection-mode-checks">
</picture>

The `.selection-mode` style class can be added to a [class@Gtk.CheckButton] to
give it a larger and round appearance. These check buttons are intended to be
used for selecting items from a list or a grid.

# OSD

<picture>
  <source srcset="osd-dark.png" media="(prefers-color-scheme: dark)">
  <img src="osd.png" alt="osd">
</picture>

The `.osd` style class has a number of loosely related purposes depending on
what widget it's applied to.

Usually, it makes the widget background dark and partially transparent, and
makes its accent color white.

However, it has different effects in a few specific cases.

## Overlay Buttons

<picture>
  <source srcset="osd-buttons-dark.png" media="(prefers-color-scheme: dark)">
  <img src="osd-buttons.png" alt="osd-buttons">
</picture>

When used with [class@Gtk.Button], `.osd` can be used to create large standalone
buttons that overlap content, for example, the previous/next page arrows in an
image viewer. They appear dark and slightly larger than regular buttons.

It can be used in combination with [`.circular`](#circular) or [`.pill`](#pill).

## Floating Toolbars

<picture>
  <source srcset="osd-toolbar-dark.png" media="(prefers-color-scheme: dark)">
  <img src="osd-toolbar.png" alt="osd-toolbar">
</picture>

When used along with the [`.toolbar`](#toolbars) style class, `.osd` gives the
box additional padding and round corners. This can be used to create floating
toolbars, such as video player controls.

## Progress Bars

<picture>
  <source srcset="osd-progress-bar-dark.png" media="(prefers-color-scheme: dark)">
  <img src="osd-progress-bar.png" alt="osd-progress-bar">
</picture>

When used with [class@Gtk.ProgressBar], `.osd` makes the progress bar thinner
and removes its visible trough.

OSD progress bars are intended to be used as [class@Gtk.Overlay] children,
attached to the top of the window.

# Background

<picture>
  <source srcset="style-background-dark.png" media="(prefers-color-scheme: dark)">
  <img src="style-background.png" alt="style-background">
</picture>

The `.background` style class can be used with any widget to give it the default
[window](named-colors.html#window-colors) background and foreground colors.

This can be useful when a widget needs an opaque background.

It's equivalent to using the following CSS:

```css
.background {
  background-color: @window_bg_color;
  color: @window_fg_color;
}
```

# View

<picture>
  <source srcset="style-view-dark.png" media="(prefers-color-scheme: dark)">
  <img src="style-view.png" alt="style-view">
</picture>

The `.view` style class can be used with any widget to give it the default
[view](named-colors.html#window-colors) background and foreground colors.

It's equivalent to using the following CSS:

```css
.view {
  background-color: @view_bg_color;
  color: @view_fg_color;
}
```

# Frame

<picture>
  <source srcset="style-frame-dark.png" media="(prefers-color-scheme: dark)">
  <img src="style-frame.png" alt="style-frame">
</picture>

The `.frame` style class can be used with any widget to give it the default
border.

It's equivalent to using the following CSS:

```css
.frame {
  border: 1px solid @borders;
}
```

# Compact Status Page

<picture>
  <source srcset="status-page-compact-dark.png" media="(prefers-color-scheme: dark)">
  <img src="status-page-compact.png" alt="status-page-compact">
</picture>

The `.compact` style class can be used with a [class@StatusPage] to make it take
less space. This is usually used with sidebars or popovers.

# Menu Popovers

<picture>
  <source srcset="popover-menu-list-dark.png" media="(prefers-color-scheme: dark)">
  <img src="popover-menu-list.png" alt="popover-menu-list">
</picture>

The `.menu` style class can be used with a [class@Gtk.Popover] to give it a
menu-like appearance if it has a [class@Gtk.ListBox] or a [class@Gtk.ListView]
inside it.

# Development Window

<picture>
  <source srcset="devel-window-dark.png" media="(prefers-color-scheme: dark)">
  <img src="devel-window.png" alt="devel-window">
</picture>

The `.devel` style class can be used with [class@Gtk.Window]. This will give
any [class@HeaderBar] or [class@Gtk.HeaderBar] inside that window a striped
appearance.

This style class is typically used to indicate unstable or nightly applications.

# Inline Tab Bars & Search Bars

<picture>
  <source srcset="search-bar-inline-dark.png" media="(prefers-color-scheme: dark)">
  <img src="search-bar-inline.png" alt="search-bar-inline">
</picture>

<picture>
  <source srcset="tab-bar-inline-dark.png" media="(prefers-color-scheme: dark)">
  <img src="tab-bar-inline.png" alt="tab-bar-inline">
</picture>

By default [class@Gtk.SearchBar] and [class@TabBar] look like a part of an
[class@HeaderBar] or [class@Gtk.HeaderBar] and are intended to be used directly
attached to one. With the `.inline` style class they have neutral backgrounds
and can be used in different contexts instead.

# Undershoot Indicators

<picture>
  <source srcset="undershoot-dark.png" media="(prefers-color-scheme: dark)">
  <img src="undershoot.png" alt="undershoot">
</picture>

The `.undershoot-top`, `.undershoot-bottom`, `.undershoot-start` and
`.undershoot-end` style classes can be used with [class@Gtk.ScrolledWindow] to
add an undershoot indicator on the given edge, presented as a shadow.

`.undershoot-start` and `.undershoot-end` automatically follow text direction,
same as [property@Gtk.Widget:margin-start] and [property@Gtk.Widget:margin-end].

Most applications should use [class@ToolbarView] instead, as it manages
undershoots automatically based on presence and visibility of top and bottom
bars.

Since: 1.4

# Property Rows

<picture>
  <source srcset="property-row-dark.png" media="(prefers-color-scheme: dark)">
  <img src="property-row.png" alt="property-row">
</picture>

The `.property` style class can be used with [class@ActionRow] and
[class@ExpanderRow]. It deemphasizes the row title and emphasizes subtitle
instead, which is useful for displaying read-only properties, as follows:

```xml
<object class="AdapActionRow">
  <property name="title" translatable="yes">Property Name</property>
  <property name="subtitle">Value</property>
  <property name="subtitle-selectable">True</property>
  <style>
    <class name="property"/>
  </style>
</object>
```

Since: 1.4

# Deprecated Style Classes

The following style classes are deprecated and remain there for compatibility.
They shouldn't be used in new code.

## `.content`

<picture>
  <source srcset="boxed-lists-dark.png" media="(prefers-color-scheme: dark)">
  <img src="boxed-lists.png" alt="boxed-lists">
</picture>

The `.content` style class can be applied to a [class@Gtk.ListBox] to give it a
boxed list appearance. The [`.boxed-list`](#boxed-lists-cards) style class is
completely equivalent to it and should be used instead.

## `.sidebar`

<picture>
  <source srcset="deprecated-sidebar-dark.png" media="(prefers-color-scheme: dark)">
  <img src="deprecated-sidebar.png" alt="deprecated-sidebar">
</picture>

The `.sidebar` style class adds a border at the end of the widget (`border-right`
for left-to-right text direction, `border-left` for right-to-left) and removes
background from any [class@Gtk.ListBox] or [class@Gtk.ListView] inside it.

It can be replaced by using the [`.navigation-sidebar`](#sidebars) style class
on the list widget, combined with a [class@Gtk.Separator] to achieve the border.

## `.app-notification`

<picture>
  <source srcset="deprecated-app-notification-dark.png" media="(prefers-color-scheme: dark)">
  <img src="deprecated-app-notification.png" alt="deprecated-app-notification">
</picture>

The `.app-notification` style class is used with widgets like [class@Gtk.Box].
It adds [`.osd`](#osd) appearance to the widget and makes its bottom corners
round. When used together with a [class@Gtk.Overlay] and a [class@Gtk.Revealer],
it allows creating in-app notifications.

[class@ToastOverlay] can be used to replace it.

## `.large-title`

<picture>
  <source srcset="typography-large-title-dark.png" media="(prefers-color-scheme: dark)">
  <img src="typography-large-title.png" alt="typography-large-title">
</picture>

The `.large-title` style class makes text large and thin. It's the largest
style, infrequently used for display headings in greeters or assistants. It
should only be used in conjunction with large amounts of whitespace.

The [`.title-1`](#typography-styles) style class should be used instead.

Deprecated since: 1.2

# Flat Header Bar

<picture>
  <source srcset="flat-header-bar-dark.png" media="(prefers-color-scheme: dark)">
  <img src="flat-header-bar.png" alt="flat-header-bar">
</picture>

The `.flat` style class can be used with an [class@HeaderBar] or
[class@Gtk.HeaderBar] to give it a flat appearance.

Use [class@ToolbarView] instead.

Deprecated since: 1.4
