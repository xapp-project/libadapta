# Adding a screenshot to the docs

1. Create an `IMAGE.ui` file in the `data/` directory.
2. Put the widget to screenshot inside with the `widget` id. For example:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <requires lib="libadapta" version="1.0"/>
  <object class="GtkButton" id="widget">
    <property name="label">Example</property>
  </object>
</interface>
```

If a widget needs to be hovered - for example, a list item - put the `hover` id
onto it.

If the widget needs special treatment - for example, it's a `GtkPopover` - it
should be special-cased in `screenshot.c` based on its type.

3. From the build directory, run:

```
./doc/tools/screenshot ../doc/tools/data/ ../doc/images/ -i IMAGE
```

4. The generator will create `IMAGE.png` and `IMAGE-dark.png` images. Add them
to `libadapta.toml.in`.
5. Use them in the docs as follows:

```html
<picture>
  <source srcset="IMAGE-dark.png" media="(prefers-color-scheme: dark)">
  <img src="IMAGE.png" alt="IMAGE">
</picture>
```

# Regenerating screenshots

Make sure your system has Cantarell and Noto Sans Mono fonts installed on your
system, otherwise the screenshots may have wrong fonts.

To regenerate all screenshots, run:

```c
./doc/tools/screenshot ../doc/tools/data/ ../doc/images/
```

from the build directory.
