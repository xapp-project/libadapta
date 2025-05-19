#include "adap-demo-page-animations.h"

#include <glib/gi18n.h>

struct _AdapDemoPageAnimations
{
  AdapBin parent_instance;

  GtkStack *animation_preferences_stack;
  AdapAnimation *timed_animation;
  GtkWidget *timed_animation_sample;
  GtkWidget *timed_animation_button_box;
  AdapSpinRow *timed_animation_repeat_count;
  AdapSwitchRow *timed_animation_reverse;
  AdapSwitchRow *timed_animation_alternate;
  AdapSpinRow *timed_animation_duration;
  AdapComboRow *timed_animation_easing;
  AdapAnimation *spring_animation;
  AdapSpinRow *spring_animation_velocity;
  AdapSpinRow *spring_animation_damping;
  AdapSpinRow *spring_animation_mass;
  AdapSpinRow *spring_animation_stiffness;
  AdapSpinRow *spring_animation_epsilon;
  AdapSwitchRow *spring_animation_clamp_switch;
};

enum {
  PROP_0,
  PROP_TIMED_ANIMATION,
  PROP_SPRING_ANIMATION,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdapDemoPageAnimations, adap_demo_page_animations, ADAP_TYPE_BIN)

static AdapAnimation *
get_current_animation (AdapDemoPageAnimations *self)
{
  const char *current_animation;

  current_animation = gtk_stack_get_visible_child_name (self->animation_preferences_stack);

  if (!g_strcmp0 (current_animation, "Timed")) {
    return self->timed_animation;
  } else if (!g_strcmp0 (current_animation, "Spring")) {
    return self->spring_animation;
  } else {
    g_assert_not_reached ();
  }
}

static char *
animations_easing_name (AdapEnumListItem *value,
                        gpointer         user_data)
{
  switch (adap_enum_list_item_get_value (value)) {
  case ADAP_LINEAR:
    return g_strdup (_("Linear"));
  case ADAP_EASE_IN_QUAD:
    return g_strdup (_("Ease-in (Quadratic)"));
  case ADAP_EASE_OUT_QUAD:
    return g_strdup (_("Ease-out (Quadratic)"));
  case ADAP_EASE_IN_OUT_QUAD:
    return g_strdup (_("Ease-in-out (Quadratic)"));
  case ADAP_EASE_IN_CUBIC:
    return g_strdup (_("Ease-in (Cubic)"));
  case ADAP_EASE_OUT_CUBIC:
    return g_strdup (_("Ease-out (Cubic)"));
  case ADAP_EASE_IN_OUT_CUBIC:
    return g_strdup (_("Ease-in-out (Cubic)"));
  case ADAP_EASE_IN_QUART:
    return g_strdup (_("Ease-in (Quartic)"));
  case ADAP_EASE_OUT_QUART:
    return g_strdup (_("Ease-out (Quartic)"));
  case ADAP_EASE_IN_OUT_QUART:
    return g_strdup (_("Ease-in-out (Quartic)"));
  case ADAP_EASE_IN_QUINT:
    return g_strdup (_("Ease-in (Quintic)"));
  case ADAP_EASE_OUT_QUINT:
    return g_strdup (_("Ease-out (Quintic)"));
  case ADAP_EASE_IN_OUT_QUINT:
    return g_strdup (_("Ease-in-out (Quintic)"));
  case ADAP_EASE_IN_SINE:
    return g_strdup (_("Ease-in (Sine)"));
  case ADAP_EASE_OUT_SINE:
    return g_strdup (_("Ease-out (Sine)"));
  case ADAP_EASE_IN_OUT_SINE:
    return g_strdup (_("Ease-in-out (Sine)"));
  case ADAP_EASE_IN_EXPO:
    return g_strdup (_("Ease-in (Exponential)"));
  case ADAP_EASE_OUT_EXPO:
    return g_strdup (_("Ease-out (Exponential)"));
  case ADAP_EASE_IN_OUT_EXPO:
    return g_strdup (_("Ease-in-out (Exponential)"));
  case ADAP_EASE_IN_CIRC:
    return g_strdup (_("Ease-in (Circular)"));
  case ADAP_EASE_OUT_CIRC:
    return g_strdup (_("Ease-out (Circular)"));
  case ADAP_EASE_IN_OUT_CIRC:
    return g_strdup (_("Ease-in-out (Circular)"));
  case ADAP_EASE_IN_ELASTIC:
    return g_strdup (_("Ease-in (Elastic)"));
  case ADAP_EASE_OUT_ELASTIC:
    return g_strdup (_("Ease-out (Elastic)"));
  case ADAP_EASE_IN_OUT_ELASTIC:
    return g_strdup (_("Ease-in-out (Elastic)"));
  case ADAP_EASE_IN_BACK:
    return g_strdup (_("Ease-in (Back)"));
  case ADAP_EASE_OUT_BACK:
    return g_strdup (_("Ease-out (Back)"));
  case ADAP_EASE_IN_OUT_BACK:
    return g_strdup (_("Ease-in-out (Back)"));
  case ADAP_EASE_IN_BOUNCE:
    return g_strdup (_("Ease-in (Bounce)"));
  case ADAP_EASE_OUT_BOUNCE:
    return g_strdup (_("Ease-out (Bounce)"));
  case ADAP_EASE_IN_OUT_BOUNCE:
    return g_strdup (_("Ease-in-out (Bounce)"));
  default:
    return NULL;
  }
}

static void
timed_animation_measure (GtkWidget      *widget,
                         GtkOrientation  orientation,
                         int             for_size,
                         int            *minimum,
                         int            *natural,
                         int            *minimum_baseline,
                         int            *natural_baseline)
{
  GtkWidget *child = gtk_widget_get_first_child (widget);

  if (!child)
    return;

  gtk_widget_measure (child, orientation, for_size, minimum, natural,
                      minimum_baseline, natural_baseline);
}

static void
timed_animation_allocate (GtkWidget *widget,
                          int        width,
                          int        height,
                          int        baseline)
{
  AdapDemoPageAnimations *self =
    ADAP_DEMO_PAGE_ANIMATIONS (gtk_widget_get_ancestor (widget, ADAP_TYPE_DEMO_PAGE_ANIMATIONS));
  GtkWidget *child = gtk_widget_get_first_child (widget);
  AdapAnimation *animation = get_current_animation (self);
  double progress;
  int child_width, offset;

  if (!child)
    return;

  progress = adap_animation_get_value (animation);

  gtk_widget_measure (child, GTK_ORIENTATION_HORIZONTAL, -1,
                      &child_width, NULL, NULL, NULL);

  offset = (int) ((width - child_width) * (progress - 0.5));

  gtk_widget_allocate (child, width, height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (offset, 0)));
}

static void
timed_animation_reset (AdapDemoPageAnimations *self)
{
  adap_animation_reset (self->timed_animation);
  adap_animation_reset (self->spring_animation);
}

static void
timed_animation_play_pause (AdapDemoPageAnimations *self)
{
  AdapAnimation *animation = get_current_animation (self);

  switch (adap_animation_get_state (animation)) {
  case ADAP_ANIMATION_IDLE:
  case ADAP_ANIMATION_FINISHED:
    adap_animation_play (animation);
    break;
  case ADAP_ANIMATION_PAUSED:
    adap_animation_resume (animation);
    break;
  case ADAP_ANIMATION_PLAYING:
    adap_animation_pause (animation);
    break;
  default:
    g_assert_not_reached ();
  }
}

static void
timed_animation_skip (AdapDemoPageAnimations *self)
{
  adap_animation_skip (self->timed_animation);
  adap_animation_skip (self->spring_animation);
}

static char *
get_play_pause_icon_name (gpointer          user_data,
                          AdapAnimationState timed_state,
                          AdapAnimationState spring_state)
{
  gboolean playing = timed_state  == ADAP_ANIMATION_PLAYING ||
                     spring_state == ADAP_ANIMATION_PLAYING;

  return g_strdup (playing ? "media-playback-pause-symbolic" : "media-playback-start-symbolic");
}

static gboolean
timed_animation_can_reset (gpointer          user_data,
                           AdapAnimationState timed_state,
                           AdapAnimationState spring_state)
{
  return timed_state  != ADAP_ANIMATION_IDLE ||
         spring_state != ADAP_ANIMATION_IDLE;
}

static gboolean
timed_animation_can_skip (gpointer          user_data,
                          AdapAnimationState timed_state,
                          AdapAnimationState spring_state)
{
  return timed_state  != ADAP_ANIMATION_FINISHED &&
         spring_state != ADAP_ANIMATION_FINISHED;
}

static void
timed_animation_cb (double     value,
                    GtkWidget *self)
{
  gtk_widget_queue_allocate (self);
}

static void
notify_spring_params_change (AdapDemoPageAnimations *self)
{
  AdapSpringParams *spring_params =
    adap_spring_params_new_full (adap_spin_row_get_value (self->spring_animation_damping),
                                adap_spin_row_get_value (self->spring_animation_mass),
                                adap_spin_row_get_value (self->spring_animation_stiffness));

  adap_spring_animation_set_spring_params (ADAP_SPRING_ANIMATION (self->spring_animation), spring_params);

  adap_spring_params_unref (spring_params);
}

static void
adap_demo_page_animations_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  AdapDemoPageAnimations *self = ADAP_DEMO_PAGE_ANIMATIONS (object);

  switch (prop_id) {
  case PROP_TIMED_ANIMATION:
    g_value_set_object (value, self->timed_animation);
    break;

  case PROP_SPRING_ANIMATION:
    g_value_set_object (value, self->spring_animation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_demo_page_animations_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  AdapDemoPageAnimations *self = ADAP_DEMO_PAGE_ANIMATIONS (object);

  switch (prop_id) {
  case PROP_TIMED_ANIMATION:
    g_set_object (&self->timed_animation, g_value_get_object (value));
    break;

  case PROP_SPRING_ANIMATION:
    g_set_object (&self->spring_animation, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adap_demo_page_animations_dispose (GObject *object)
{
  AdapDemoPageAnimations *self = ADAP_DEMO_PAGE_ANIMATIONS (object);

  g_clear_object (&self->timed_animation);
  g_clear_object (&self->spring_animation);

  G_OBJECT_CLASS (adap_demo_page_animations_parent_class)->dispose (object);
}

static void
adap_demo_page_animations_class_init (AdapDemoPageAnimationsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = adap_demo_page_animations_set_property;
  object_class->get_property = adap_demo_page_animations_get_property;
  object_class->dispose = adap_demo_page_animations_dispose;

  props[PROP_TIMED_ANIMATION] =
    g_param_spec_object ("timed-animation", NULL, NULL,
                         ADAP_TYPE_ANIMATION,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  props[PROP_SPRING_ANIMATION] =
    g_param_spec_object ("spring-animation", NULL, NULL,
                         ADAP_TYPE_ANIMATION,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adapta1/Demo/ui/pages/animations/adap-demo-page-animations.ui");
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, animation_preferences_stack);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, timed_animation_sample);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, timed_animation_button_box);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, timed_animation_repeat_count);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, timed_animation_reverse);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, timed_animation_alternate);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, timed_animation_duration);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, timed_animation_easing);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, spring_animation_velocity);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, spring_animation_damping);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, spring_animation_mass);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, spring_animation_stiffness);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, spring_animation_epsilon);
  gtk_widget_class_bind_template_child (widget_class, AdapDemoPageAnimations, spring_animation_clamp_switch);
  gtk_widget_class_bind_template_callback (widget_class, animations_easing_name);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_reset);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_play_pause);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_skip);
  gtk_widget_class_bind_template_callback (widget_class, get_play_pause_icon_name);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_can_reset);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_can_skip);
  gtk_widget_class_bind_template_callback (widget_class, notify_spring_params_change);
}

static void
adap_demo_page_animations_init (AdapDemoPageAnimations *self)
{
  GtkLayoutManager *manager;
  AdapAnimationTarget *target;

  gtk_widget_init_template (GTK_WIDGET (self));

  target = adap_callback_animation_target_new ((AdapAnimationTargetFunc)
                                              timed_animation_cb,
                                              self->timed_animation_sample,
                                              NULL);

  self->timed_animation =
    adap_timed_animation_new (GTK_WIDGET (self->timed_animation_sample),
                             0, 1, 100, g_object_ref (target));

  self->spring_animation =
    adap_spring_animation_new (GTK_WIDGET (self->timed_animation_sample), 0, 1,
                              adap_spring_params_new_full (10, 1, 100),
                              target);

  notify_spring_params_change (self);

  g_object_bind_property (self->timed_animation_repeat_count, "value",
                          self->timed_animation, "repeat-count",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->timed_animation_reverse, "active",
                          self->timed_animation, "reverse",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->timed_animation_alternate, "active",
                          self->timed_animation, "alternate",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->timed_animation_duration, "value",
                          self->timed_animation, "duration",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->timed_animation_easing, "selected",
                          self->timed_animation, "easing",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property (self->spring_animation_velocity, "value",
                          self->spring_animation, "initial_velocity",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->spring_animation_epsilon, "value",
                          self->spring_animation, "epsilon",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->spring_animation_clamp_switch, "active",
                          self->spring_animation, "clamp",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  adap_timed_animation_set_easing (ADAP_TIMED_ANIMATION (self->timed_animation),
                                  ADAP_EASE_IN_OUT_CUBIC);

  adap_animation_set_follow_enable_animations_setting (self->timed_animation, FALSE);
  adap_animation_set_follow_enable_animations_setting (self->spring_animation, FALSE);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIMED_ANIMATION]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPRING_ANIMATION]);

  manager = gtk_custom_layout_new (NULL, timed_animation_measure,
                                   timed_animation_allocate);

  gtk_widget_set_layout_manager (self->timed_animation_sample, manager);

  gtk_widget_set_direction (self->timed_animation_button_box, GTK_TEXT_DIR_LTR);
}
