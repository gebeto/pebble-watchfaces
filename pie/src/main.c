#include <pebble.h>
#include "config.h"

#if DARK_MODE == 1
#define BACKGROUND_COLOR GColorWhite
#define FOREGROUND_COLOR GColorBlack
#else
#define BACKGROUND_COLOR GColorBlack
#define FOREGROUND_COLOR GColorWhite
#endif

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
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);

  // minutes
  graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
  graphics_fill_radial(ctx, GRect(1, 1, layer_bounds.size.w - 2, layer_bounds.size.h - 2), GOvalScaleModeFitCircle, 50, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(minutes_rotation));

  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_circle(ctx, GPoint(layer_bounds.size.w / 2, layer_bounds.size.h / 2), 50);

  // hours
  graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
  int rad = 50;
  int size = rad * 2 - 1;
  int _x = layer_bounds.size.w / 2 - rad + 1;
  int _y = layer_bounds.size.h / 2 - rad + 1;
  graphics_fill_radial(ctx, GRect(_x, _y, size, size), GOvalScaleModeFitCircle, rad, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(hours_rotation));
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
