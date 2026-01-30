#include <pebble.h>

static Window *s_window;
static Layer *s_before_layer;
static Layer *s_text_layer;
static Layer *s_after_layer;

// Write the current hours and minutes into a buffer
static struct tm *tick_time;
static char s_time_buffer[8] = "00:00";
static char s_minutes_buffer[8] = "00";

static GRect wrapped_bounds;
static GRect unwrapped_bounds;

static Animation *s_unwrap_animation;
static Animation *s_wrap_animation;

int hour = 20;

static bool initial_ticked = false;

static void update_time()
{
  time_t temp = time(NULL);
  tick_time = localtime(&temp);

  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  strftime(s_minutes_buffer, sizeof(s_minutes_buffer), "%M", tick_time);
}

static void unwrap_animation(int delay_ms)
{
  PropertyAnimation *unwrap_prop_anim = property_animation_create_layer_frame(s_before_layer, &wrapped_bounds, &unwrapped_bounds);
  s_unwrap_animation = property_animation_get_animation(unwrap_prop_anim);
  animation_set_curve(s_unwrap_animation, AnimationCurveEaseOut);
  animation_set_duration(s_unwrap_animation, 500);
  animation_set_delay(s_unwrap_animation, delay_ms);
  animation_schedule(s_unwrap_animation);
}

static void wrap_animation(int delay_ms)
{
  PropertyAnimation *wrap_prop_anim = property_animation_create_layer_frame(s_before_layer, &unwrapped_bounds, &wrapped_bounds);
  s_wrap_animation = property_animation_get_animation(wrap_prop_anim);
  animation_set_curve(s_wrap_animation, AnimationCurveEaseOut);
  animation_set_duration(s_wrap_animation, 500);
  animation_set_delay(s_wrap_animation, delay_ms);
  animation_schedule(s_wrap_animation);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time();

  layer_mark_dirty(s_before_layer);
  layer_mark_dirty(s_text_layer);
  layer_mark_dirty(s_after_layer);

  if (initial_ticked)
  {
    animation_set_reverse(s_wrap_animation, true);
    wrap_animation(0);
    unwrap_animation(1000);
  }
  initial_ticked = true;
}

static void canvas_update_before_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_antialiased(ctx, true);
  GRect layer_bounds = layer_get_bounds(layer);

  int hour = tick_time->tm_hour;
  int top_gap = 0;
  for (int i = 0; i < hour + 1; i++)
  {
    GRect card_bounds_border = GRect(1, 2 + top_gap, layer_bounds.size.w - 2, 70);
    GRect card_bounds_inner = GRect(3, 4 + top_gap, card_bounds_border.size.w - 4, 70 - 4);

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, card_bounds_border, 10, GCornersAll);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, card_bounds_inner, 8, GCornersAll);

    top_gap += 5;
  }
}

static void canvas_update_after_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_antialiased(ctx, true);
  GRect layer_bounds = layer_get_bounds(layer);

  int hour = tick_time->tm_hour;
  int top_gap = 42 + 5 * hour;
  for (int i = hour; i < 23; i++)
  {
    GRect card_bounds_border = GRect(1, 2 + top_gap, layer_bounds.size.w - 2, 70);
    GRect card_bounds_inner = GRect(3, 4 + top_gap, card_bounds_border.size.w - 4, 70 - 4);

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, card_bounds_border, 10, GCornersAll);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, card_bounds_inner, 8, GCornersAll);

    top_gap += 5;
  }
}

static void canvas_update_text_proc(Layer *layer, GContext *ctx)
{

  GRect layer_bounds = layer_get_bounds(layer);
  GFont font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);

  int hour = tick_time->tm_hour;
  int top_gap = 0 + 5 * hour - 2;
  GRect card_bounds_inner = GRect(3, 4 + top_gap, layer_bounds.size.w - 2 - 4, 70 - 4);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(
      ctx, s_time_buffer, font,
      GRect(card_bounds_inner.origin.x, card_bounds_inner.origin.y - 5, card_bounds_inner.size.w, card_bounds_inner.size.h),
      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

// static void accel_tap_handler(AccelAxisType axis, int32_t direction)
// {
//   wrap_animation(0);
//   unwrap_animation(1000);
// }

static void prv_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  wrapped_bounds = GRect(bounds.origin.x, bounds.origin.y + 42 - 5, bounds.size.w, bounds.size.h);
  unwrapped_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);

  s_before_layer = layer_create(wrapped_bounds);
  s_text_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  s_after_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));

  update_time();

  layer_set_update_proc(s_before_layer, canvas_update_before_proc);
  layer_set_update_proc(s_text_layer, canvas_update_text_proc);
  layer_set_update_proc(s_after_layer, canvas_update_after_proc);

  layer_add_child(window_layer, s_before_layer);
  layer_add_child(s_before_layer, s_text_layer);
  layer_add_child(window_layer, s_after_layer);

  layer_mark_dirty(s_before_layer);
  layer_mark_dirty(s_text_layer);

  unwrap_animation(300);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // accel_tap_service_subscribe(accel_tap_handler);
}

static void prv_window_unload(Window *window)
{
  layer_destroy(s_before_layer);
}

static void prv_init(void)
{
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = prv_window_load,
                                           .unload = prv_window_unload,
                                       });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void)
{
  window_destroy(s_window);
}

int main(void)
{
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
