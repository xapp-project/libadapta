Introduction
============

libAdapta is libAdwaita with theme support and a few extra.

It provides the same features and the same look as libAdwaita by default.

In desktop environments which provide theme selection, libAdapta apps follow the theme and use the proper window controls.

libAdwaita also provides a compatibility header which makes it easy for developers to switch between libAdwaita and libAdapta without requiring code changes.

Is it a fork of libAdwaita?
---------------------------

Yes and no. It's a 'soft' fork. It can be rebased on newer versions.

It only intends to add features, not to deviate long-term.

Why not contribute to libAdwaita directly?
------------------------------------------

LibAdwaita has the right to be what it wants to be and to not support what it doesn't want to support.

LibAdwaita identifies as a GNOME-only platform library. It refused requests to support theming or desktop features which are not present in GNOME.

The idea of an extension (similar to libGranite) was also rejected by libAdwaita.

Shall I use libAdwaita or libAdapta?
------------------------------------

If you want to make an official GNOME application -> libAdwaita. It's the official platform library, if you want to be featured in GNOME Circle and be recognized officially as a GNOME applications, this is the way.

If you want to make an app which works in GNOME -> either libraries. They both look the same. LibAdwaita will evolve more rapidly. This is both a pro and a cons. You'll have to keep up with it in your code, but you'll get new features faster than if you wait for libAdapta rebases.

If you want to make apps which work outside of GNOME, in any desktop and any distribution -> libAdapta.

Can I switch easily between libAdwaita and libAdapta?
-----------------------------------------------------

Yes. LibAdapta includes a compatibility header so you don't have to make any code changes.

In C, simply include the compatibility header:

```
#include <libadapta-1/adapta.h>
#include <libadapta-1/adw-compat.h>
```

In Python, just import the Adapta module as "Adw":

```
import gi
gi.require_version('Adap', '1')
from gi.repository include Adap as Adw
```

The rest of your code can use Adwaita function and class names, whether you're building against libAdwaita or libAdapta.

Versions
========

libAdapta 1.5 is based on libAdwaita 1.5 and compatible with `libadapta-1.5` themes.

Theme support
=============

libAdapta finds the name and directory of the current GTK theme. If the theme provides a `libadapta-1.5` subdirectory, it uses the theme's stylesheets. Otherwise it fallsback to the library's own stylesheet, which looks exactly the same as upstream libadwaita.

CSS stylesheet
--------------

Inside the theme's `libadapta-1.5` directory, there should be the following CSS files:

- `defaults-light.css` defines the colors in Light mode
- `defaults-dark.css` defines the colors in dark mode
- `base.css` defines the widgets style
- `base-hc.css` defines the widgets style in high-contrast mode
- `assets` provide pictures used by the stylesheet

You can find these files in `/usr/share/themes/LibAdapta-Example/libadapta-1.5` after installing the `libadapta-1-examples` package.

To minimize potential issues it is recommended to only change the colors and the style of the window controls.

The best place to modify the window controls is at the bottom of `base.css`.

SASS stylesheet
---------------

If your theme uses SASS you can work from the SASS files directly and get greater control.

In this case, you can find the stylesheet here in the `src/stylesheet` directory.

Examples
========

You can find examples of libAdapta applications at https://github.com/xapp-project/libadapta-examples

This repository also includes themes and utilities.

Building
========

```sh
meson setup _build
ninja -C _build
ninja -C _build install
```

For build options see [meson_options.txt](./meson_options.txt). E.g. to enable documentation:

```sh
meson setup _build -Dgtk_doc=true
ninja -C _build
```

Demo
====

There's a C example:

```sh
_build/run _build/demo/adapta-1-demo
```

Documentation
=============

https://gnome.pages.gitlab.gnome.org/libadwaita/doc/1.5/

Getting in Touch
================

https://matrix.to/#/#xapp:matrix.org
