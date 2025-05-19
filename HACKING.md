## Building

For build instructions see the README.md

## Pull Requests

Before filing a pull request run the tests:

```sh
ninja -C _build test
```

Use descriptive commit messages, see

   https://wiki.gnome.org/Git/CommitMessages

and check

   https://wiki.openstack.org/wiki/GitCommitMessages

for good examples.

## Coding Style

We mostly use kernel style but

* Use spaces, never tabs
* Use 2 spaces for indentation

### GTK Style Function Argument Indentation

Use GTK style function argument indentation. It's harder for renames but it's
what GNOME upstream projects do.

*Good*:

```c
static gboolean
key_press_event_cb (GtkWidget *widget,
                    GdkEvent  *event,
                    gpointer   data)
```

*Bad*:

```c
static gboolean
key_press_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
```

### Function Prototypes

Function prototypes should be grouped together in logical groups, e.g. all
constructors, or all accessords to a given property.
There should be no empty line in a group, and groups should be separated from
each other by an empty line.

The function's attributes and returned type should be on the same line as the
function's name.
Have only one parameter per line.
If a function takes no parameters, explicit it by using `void` as the parameter.

In a group, align function names, the opening parentheses, and parameter names
together.
Use the least possible amount of spaces while preserving alignment.
Asterisks from the returned type should be stuck to the function's name.
Similarly, asterisks from a parameter's type should be stuck to the parameter's
name.

Getters should come before setters.

In public headers, set the function's availability or deprecation state in a
line preceding its prototype.

*Good*:

```c
ADAP_AVAILABLE_IN_ALL
AdapFoo *adap_foo_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
AdapBar *adap_foo_get_bar (AdapFoo *self);
ADAP_AVAILABLE_IN_ALL
void    adap_foo_set_bar (AdapFoo *self,
                         AdapBar *bar);

ADAP_AVAILABLE_IN_ALL
gboolean adap_foo_get_bit (AdapFoo   *self);
ADAP_AVAILABLE_IN_ALL
void     adap_foo_set_bit (AdapFoo   *self,
                          gboolean  bit);

ADAP_AVAILABLE_IN_ALL
void adap_foo_add_baz    (AdapFoo *self,
                         AdapBaz *baz);
ADAP_AVAILABLE_IN_ALL
void adap_foo_remove_baz (AdapFoo *self,
                         AdapBaz *baz);

ADAP_AVAILABLE_IN_ALL
void adap_foo_frobnicate (AdapFoo *self);
```

If the function transfers a new handle to a resource, like a reference, a
floating reference, a file handle, or any other kind of handle that would result
into a resource leak if ignored, add `G_GNUC_WARN_UNUSED_RESULT` after the
closing parenthesis.
No need to add `G_GNUC_WARN_UNUSED_RESULT` when the caller is guaranteed to have
a handle to the resource, e.g. in methods incrementing a reference counter.

*Good*:

```c
ADAP_AVAILABLE_IN_ALL
AdapFoo *adap_foo_new (void) G_GNUC_WARN_UNUSED_RESULT;
ADAP_AVAILABLE_IN_ALL
AdapFoo *adap_foo_ref (AdapFoo *self);

ADAP_AVAILABLE_IN_ALL
char *adap_foo_to_string (AdapFoo *self) G_GNUC_WARN_UNUSED_RESULT;
```

*Bad*:

```c
ADAP_AVAILABLE_IN_ALL
AdapFoo *adap_foo_new (void);
ADAP_AVAILABLE_IN_ALL
AdapFoo *adap_foo_ref (AdapFoo *self) G_GNUC_WARN_UNUSED_RESULT;

ADAP_AVAILABLE_IN_ALL
char *adap_foo_to_string (AdapFoo *self);
```

### Braces

Everything besides functions and structs have the opening curly brace on the same line.

*Good*:

```c
if (i < 0) {
    ...
}
```

*Bad*:

```c
if (i < 0)
{
    ...
}
```

Single line `if` or `else` statements don't need braces but if either `if` or
`else` have braces both get them:

*Good*:

```c
if (i < 0)
  i++;
else
  i--;
```

```c
if (i < 0) {
  i++;
  j++;
} else {
  i--;
}
```

```c
if (i < 0) {
  i++;
} else {
  i--;
  j--;
}
```

*Bad*:

```c
if (i < 0) {
  i++;
} else {
  i--;
}
```

```c
if (i < 0) {
  i++;
  j++;
} else
  i--;
```

```c
if (i < 0)
  i++;
else {
  i--;
  j--;
}
```

Function calls have a space between function name and invocation:

*Good*:

```c
visible_child_name = gtk_stack_get_visible_child_name (GTK_STACK (self->stack));
```

*Bad*:

```c
visible_child_name = gtk_stack_get_visible_child_name(GTK_STACK(self->stack));
```

### Header Inclusion Guards

Guard header inclusion with `#pragma once` rather than the traditional
`#ifndef`-`#define`-`#endif` trio.

Internal headers (for consistency, whether they need to be installed or not)
should contain the following guard to prevent users from directly including
them:
```c
#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)
#error "Only <adapta.h> can be included directly."
#endif
```

Only after these should you include headers.

### Signals

Prefix signal enum names with *SIGNAL_*.

*Good*:

```c
enum {
  SIGNAL_SUBMITTED = 0,
  SIGNAL_DELETED,
  SIGNAL_SYMBOL_CLICKED,
  SIGNAL_LAST_SIGNAL,
};
```

Also note that the last element ends with a comma to reduce diff noise when
adding further signals.

### Properties

Prefix property enum names with *PROP_*.

*Good*:

```c
enum {
  PROP_0 = 0,
  PROP_NUMBER,
  PROP_SHOW_ACTION_BUTTONS,
  PROP_COLUMN_SPACING,
  PROP_ROW_SPACING,
  PROP_RELIEF,
  PROP_LAST_PROP,
};
```

Also note that the last element ends with a comma to reduce diff noise when
adding further properties.

### Comment Style

In comments use full sentences with proper capitalization and punctuation.

*Good*:

```c
/* Make sure we don't overflow. */
```

*Bad:*

```c
/* overflow check */
```

### Callbacks

Signal callbacks have a *_cb* suffix.

*Good*:

```c
g_signal_connect(self, "clicked", G_CALLBACK (button_clicked_cb), NULL);
```

*Bad*:

```c
g_signal_connect(self, "clicked", G_CALLBACK (handle_button_clicked), NULL);
```

### Static Functions

Static functions don't need the class prefix.  E.g. with a type foo_bar:

*Good*:

```c
static void
selection_changed_cb (AdapViewSwitcher   *self,
                      guint              position,
                      guint              n_items)
```

*Bad*:

```c
static void
adap_view_switcher_selection_changed_cb (AdapViewSwitcher   *self,
                                        guint              position,
                                        guint              n_items)
```

Note however that virtual methods like
*<class_name>_{init,constructed,finalize,dispose}* do use the class prefix.
These functions are usually never called directly but only assigned once in
*<class_name>_constructed* so the longer name is kind of acceptable. This also
helps to distinguish virtual methods from regular private methods.

### Self Argument

The first argument is usually the object itself so call it *self*. E.g. for a
non public function:

*Good*:

```c
static gboolean
expire_cb (FooButton *self)
{
  g_return_val_if_fail (BAR_IS_FOO_BUTTON (self), FALSE);
  ...
  return FALSE;
}
```

And for a public function:

*Good*:

```c
int
foo_button_get_state (FooButton *self)
{
  FooButtonPrivate *priv = bar_foo_get_instance_private(self);

  g_return_val_if_fail (BAR_IS_FOO_BUTTON (self), -1);
  return priv->state;
}
```

### User Interface Files

User interface files should end in *.ui*. If there are multiple ui
files put them in a ui/ subdirectory below the sources
(e.g. *src/ui/main-window.ui*).

#### Properties

Use minus signs instead of underscores in property names:

*Good*:

```xml
<property name="margin-start">12</property>
```

*Bad*:

```xml
<property name="margin_start">12</property>
```
