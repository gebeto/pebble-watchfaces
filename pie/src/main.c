#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_layer;

// Write the current hours and minutes into a buffer
static struct tm *s_tick_time;

static void update_time()
{
  time_t temp = time(NULL);
  s_tick_time = localtime(&temp);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time();

  layer_mark_dirty(s_layer);
}

static void canvas_update_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_antialiased(ctx, true);

  GRect layer_bounds = layer_get_bounds(layer);

  int hours_rotation = s_tick_time->tm_hour * 15;
  // int hours_rotation = 350;
  // int minutes_rotation = s_tick_time->tm_min * 6;
  int minutes_rotation = 145;

  // background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);

  // minutes
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_radial(ctx, GRect(1, 1, 178, 178), GOvalScaleModeFitCircle, 50, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(minutes_rotation));

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(90, 90), 50);

  // hours
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_radial(ctx, GRect(41, 41, 99, 99), GOvalScaleModeFitCircle, 50, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(hours_rotation));
}

static void prv_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  update_time();
  layer_set_update_proc(s_layer, canvas_update_proc);
  layer_add_child(window_layer, s_layer);
  layer_mark_dirty(s_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void prv_window_unload(Window *window)
{
  text_layer_destroy(s_text_layer);
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
