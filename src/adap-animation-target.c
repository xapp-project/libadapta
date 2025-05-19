/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adap-animation-target-private.h"

/**
 * AdapAnimationTarget:
 *
 * Represents a value [class@Animation] can animate.
 */

/**
 * AdapCallbackAnimationTarget:
 *
 * An [class@AnimationTarget] that calls a given callback during the
 * animation.
 */

/**
 * AdapPropertyAnimationTarget:
 *
 * An [class@AnimationTarget] changing the value of a property of a
 * [class@GObject.Object] instance.
 *
 * Since: 1.2
 */

struct _AdapAnimationTarget
{
  GObject parent_instance;
};

struct _AdapAnimationTargetClass
{
  GObjectClass parent_class;

  void (*set_value) (AdapAnimationTarget *self,
                     double              value);
};

G_DEFINE_ABSTRACT_TYPE (AdapAnimationTarget, adap_animation_target, G_TYPE_OBJECT)

static void
adap_animation_target_class_init (AdapAnimationTargetClass *klass)
{
}

static void
adap_animation_target_init (AdapAnimationTarget *self)
{
}

void
adap_animation_target_set_value (AdapAnimationTarget *self,
                                double              value)
{
  g_return_if_fail (ADAP_IS_ANIMATION_TARGET (self));

  ADAP_ANIMATION_TARGET_GET_CLASS (self)->set_value (self, value);
}

struct _AdapCallbackAnimationTarget
{
  AdapAnimationTarget parent_instance;

  AdapAnimationTargetFunc callback;
  gpointer user_data;
  GDestroyNotify destroy_notify;
};

struct _AdapCallbackAnimationTargetClass
{
  AdapAnimationTargetClass parent_class;
};

G_DEFINE_FINAL_TYPE (AdapCallbackAnimationTarget, adap_callback_animation_target, ADAP_TYPE_ANIMATION_TARGET)

static void
adap_callback_animation_target_set_value (AdapAnimationTarget *target,
                                         double              value)
{
  AdapCallbackAnimationTarget *self = ADAP_CALLBACK_ANIMATION_TARGET (target);

  self->callback (value, self->user_data);
}

static void
adap_callback_animation_finalize (GObject *object)
{
  AdapCallbackAnimationTarget *self = ADAP_CALLBACK_ANIMATION_TARGET (object);

  if (self->destroy_notify)
    self->destroy_notify (self->user_data);

  G_OBJECT_CLASS (adap_callback_animation_target_parent_class)->finalize (object);
}

static void
adap_callback_animation_target_class_init (AdapCallbackAnimationTargetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  AdapAnimationTargetClass *target_class = ADAP_ANIMATION_TARGET_CLASS (klass);

  object_class->finalize = adap_callback_animation_finalize;

  target_class->set_value = adap_callback_animation_target_set_value;
}

static void
adap_callback_animation_target_init (AdapCallbackAnimationTarget *self)
{
}

/**
 * adap_callback_animation_target_new:
 * @callback: (scope notified) (not nullable): the callback to call
 * @user_data: the data to be passed to @callback
 * @destroy: (destroy user_data): the function to be called when the
 *   callback action is finalized
 *
 * Creates a new `AdapAnimationTarget` that calls the given @callback during
 * the animation.
 *
 * Returns: the newly created callback target
 */
AdapAnimationTarget *
adap_callback_animation_target_new (AdapAnimationTargetFunc callback,
                                   gpointer               user_data,
                                   GDestroyNotify         destroy)
{
  AdapCallbackAnimationTarget *self;

  g_return_val_if_fail (callback != NULL, NULL);

  self = g_object_new (ADAP_TYPE_CALLBACK_ANIMATION_TARGET, NULL);

  self->callback = callback;
  self->user_data = user_data;
  self->destroy_notify = destroy;

  return ADAP_ANIMATION_TARGET (self);
}

struct _AdapPropertyAnimationTarget
{
  AdapAnimationTarget parent_instance;

  GObject *object;
  GParamSpec *pspec;
};

struct _AdapPropertyAnimationTargetClass
{
  AdapAnimationTargetClass parent_class;
};

G_DEFINE_FINAL_TYPE (AdapPropertyAnimationTarget, adap_property_animation_target, ADAP_TYPE_ANIMATION_TARGET)

enum {
  PROPERTY_PROP_0,
  PROPERTY_PROP_OBJECT,
  PROPERTY_PROP_PSPEC,
  LAST_PROPERTY_PROP
};

static GParamSpec *property_props[LAST_PROPERTY_PROP];

static void
object_weak_notify (gpointer  data,
                    GObject  *object)
{
  AdapPropertyAnimationTarget *self = ADAP_PROPERTY_ANIMATION_TARGET (data);
  self->object = NULL;
}

static void
set_object (AdapPropertyAnimationTarget *self,
            GObject                    *object)
{
  if (self->object)
    g_object_weak_unref (self->object, object_weak_notify, self);
  self->object = object;
  g_object_weak_ref (self->object, object_weak_notify, self);
}

static void
adap_property_animation_target_set_value (AdapAnimationTarget *target,
                                         double              value)
{
  AdapPropertyAnimationTarget *self = ADAP_PROPERTY_ANIMATION_TARGET (target);
  GValue gvalue = G_VALUE_INIT;

  if (!self->object || !self->pspec)
    return;

  g_value_init (&gvalue, G_TYPE_DOUBLE);
  g_value_set_double (&gvalue, value);
  g_object_set_property (self->object, self->pspec->name, &gvalue);
}

static void
adap_property_animation_target_constructed (GObject *object)
{
  AdapPropertyAnimationTarget *self = ADAP_PROPERTY_ANIMATION_TARGET (object);

  G_OBJECT_CLASS (adap_property_animation_target_parent_class)->constructed (object);

  if (!self->object)
    g_error ("AdapPropertyAnimationTarget constructed without specifying a value "
             "for the 'object' property");

  if (!self->pspec)
    g_error ("AdapPropertyAnimationTarget constructed without specifying a value "
             "for the 'pspec' property");

  if (!g_type_is_a (G_OBJECT_TYPE (self->object), self->pspec->owner_type))
    g_error ("Cannot create AdapPropertyAnimationTarget: %s doesn't have the "
             "%s:%s property",
             G_OBJECT_TYPE_NAME (self->object),
             g_type_name (self->pspec->owner_type),
             self->pspec->name);
}

static void
adap_property_animation_target_dispose (GObject *object)
{
  AdapPropertyAnimationTarget *self = ADAP_PROPERTY_ANIMATION_TARGET (object);

  if (self->object)
    g_object_weak_unref (self->object, object_weak_notify, self);
  self->object = NULL;

  G_OBJECT_CLASS (adap_property_animation_target_parent_class)->dispose (object);
}

static void
adap_property_animation_target_finalize (GObject *object)
{
  AdapPropertyAnimationTarget *self = ADAP_PROPERTY_ANIMATION_TARGET (object);

  g_clear_pointer (&self->pspec, g_param_spec_unref);

  G_OBJECT_CLASS (adap_property_animation_target_parent_class)->finalize (object);
}

static void
adap_property_animation_target_get_property (GObject    *object,
                                            guint       prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
  AdapPropertyAnimationTarget *self = ADAP_PROPERTY_ANIMATION_TARGET (object);

  switch (prop_id) {
  case PROPERTY_PROP_OBJECT:
    g_value_set_object (value, adap_property_animation_target_get_object (self));
    break;

  case PROPERTY_PROP_PSPEC:
    g_value_set_param (value, adap_property_animation_target_get_pspec (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_property_animation_target_set_property (GObject      *object,
                                            guint         prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
  AdapPropertyAnimationTarget *self = ADAP_PROPERTY_ANIMATION_TARGET (object);

  switch (prop_id) {
  case PROPERTY_PROP_OBJECT:
    set_object (self, g_value_get_object (value));
    break;

  case PROPERTY_PROP_PSPEC:
    g_clear_pointer (&self->pspec, g_param_spec_unref);
    self->pspec = g_param_spec_ref (g_value_get_param (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_property_animation_target_class_init (AdapPropertyAnimationTargetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  AdapAnimationTargetClass *target_class = ADAP_ANIMATION_TARGET_CLASS (klass);

  object_class->constructed = adap_property_animation_target_constructed;
  object_class->dispose = adap_property_animation_target_dispose;
  object_class->finalize = adap_property_animation_target_finalize;
  object_class->get_property = adap_property_animation_target_get_property;
  object_class->set_property = adap_property_animation_target_set_property;

  target_class->set_value = adap_property_animation_target_set_value;

  /**
   * AdapPropertyAnimationTarget:object: (attributes org.gtk.Property.get=adap_property_animation_target_get_object)
   *
   * The object whose property will be animated.
   *
   * The `AdapPropertyAnimationTarget` instance does not hold a strong reference
   * on the object; make sure the object is kept alive throughout the target's
   * lifetime.
   *
   * Since: 1.2
   */
  property_props[PROPERTY_PROP_OBJECT] =
    g_param_spec_object ("object", NULL, NULL,
                         G_TYPE_OBJECT,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * AdapPropertyAnimationTarget:pspec: (attributes org.gtk.Property.get=adap_property_animation_target_get_pspec)
   *
   * The `GParamSpec` of the property to be animated.
   *
   * Since: 1.2
   */
  property_props[PROPERTY_PROP_PSPEC] =
    g_param_spec_param ("pspec", NULL, NULL,
                        G_TYPE_PARAM,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     LAST_PROPERTY_PROP,
                                     property_props);
}

static void
adap_property_animation_target_init (AdapPropertyAnimationTarget *self)
{
}

/**
 * adap_property_animation_target_new:
 * @object: an object to be animated
 * @property_name: the name of the property on @object to animate
 *
 * Creates a new `AdapPropertyAnimationTarget` for the @property_name property on
 * @object.
 *
 * Returns: the newly created `AdapPropertyAnimationTarget`
 *
 * Since: 1.2
 */
AdapAnimationTarget *
adap_property_animation_target_new (GObject    *object,
                                   const char *property_name)
{
  GParamSpec *pspec;

  g_return_val_if_fail (G_IS_OBJECT (object), NULL);
  g_return_val_if_fail (property_name != NULL, NULL);

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object), property_name);

  if (!pspec)
    g_error ("Type '%s' does not have a property named '%s'",
             G_OBJECT_TYPE_NAME (object), property_name);

  return adap_property_animation_target_new_for_pspec (object, pspec);
}

/**
 * adap_property_animation_target_new_for_pspec:
 * @object: an object to be animated
 * @pspec: the param spec of the property on @object to animate
 *
 * Creates a new `AdapPropertyAnimationTarget` for the @pspec property on
 * @object.
 *
 * Returns: new newly created `AdapPropertyAnimationTarget`
 *
 * Since: 1.2
 */
AdapAnimationTarget *
adap_property_animation_target_new_for_pspec (GObject    *object,
                                             GParamSpec *pspec)
{
  g_return_val_if_fail (G_IS_OBJECT (object), NULL);
  g_return_val_if_fail (G_IS_PARAM_SPEC (pspec), NULL);

  return g_object_new (ADAP_TYPE_PROPERTY_ANIMATION_TARGET,
                       "object", object,
                       "pspec", pspec,
                       NULL);
}

/**
 * adap_property_animation_target_get_object: (attributes org.gtk.Method.get_property=object)
 * @self: a property animation target
 *
 * Gets the object animated by @self.
 *
 * The `AdapPropertyAnimationTarget` instance does not hold a strong reference on
 * the object; make sure the object is kept alive throughout the target's
 * lifetime.
 *
 * Returns: (transfer none): the animated object
 *
 * Since: 1.2
 */
GObject *
adap_property_animation_target_get_object (AdapPropertyAnimationTarget *self)
{
  g_return_val_if_fail (ADAP_IS_PROPERTY_ANIMATION_TARGET (self), NULL);

  return self->object;
}

/**
 * adap_property_animation_target_get_pspec: (attributes org.gtk.Method.get_property=pspec)
 * @self: a property animation target
 *
 * Gets the `GParamSpec` of the property animated by @self.
 *
 * Returns: (transfer none): the animated property's `GParamSpec`
 *
 * Since: 1.2
 */
GParamSpec *
adap_property_animation_target_get_pspec (AdapPropertyAnimationTarget *self)
{
  g_return_val_if_fail (ADAP_IS_PROPERTY_ANIMATION_TARGET (self), NULL);

  return self->pspec;
}
