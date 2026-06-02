#include <pebble.h>
#include "config.h"

#if DARK_MODE
#define BACKGROUND_COLOR GColorBlack
#define FOREGROUND_COLOR GColorWhite
#else
#define BACKGROUND_COLOR GColorWhite
#define FOREGROUND_COLOR GColorBlack
#endif

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_layer;

GFont font_big;
GFont font_small;

// Write the current hours and minutes into a buffer
static struct tm *tick_time;
static char s_week_buffer[20] = "2026\nJune";
static char s_hours_buffer[8] = "00:00";
static char s_date_buffer[20] = "Tuesday\n12";

static bool debug = false;

static void update_time()
{
  time_t temp = time(NULL);
  tick_time = localtime(&temp);

  strftime(s_date_buffer, sizeof(s_date_buffer), "%Y%n%b", tick_time);
  strftime(s_hours_buffer, sizeof(s_hours_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  strftime(s_week_buffer, sizeof(s_week_buffer), "%a%n%e", tick_time);
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

  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_rect(ctx, layer_bounds, layer_bounds.size.w / 2, GCornerNone);

  graphics_context_set_text_color(ctx, FOREGROUND_COLOR);

  if (debug)
    graphics_draw_rect(ctx, GRect(0, layer_bounds.size.h / 2 - 1, layer_bounds.size.w, 2));

  GRect date_rect = GRect(0, layer_bounds.size.h / 2 - 54 / 2 - 50, layer_bounds.size.w, 55);
  graphics_draw_text(ctx, s_date_buffer, font_small, date_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  if (debug)
    graphics_draw_rect(ctx, date_rect);

  GRect hours_rect = GRect(0, layer_bounds.size.h / 2 - 54 / 2 - 9, layer_bounds.size.w, 55);
  graphics_draw_text(ctx, s_hours_buffer, font_big, hours_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  if (debug)
    graphics_draw_rect(ctx, GRect(hours_rect.origin.x, hours_rect.origin.y + 8, hours_rect.size.w, hours_rect.size.h));

  GRect week_rect = GRect(0, layer_bounds.size.h / 2 - 54 / 2 + 66, layer_bounds.size.w, 55);
  graphics_draw_text(ctx, s_week_buffer, font_small, week_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  if (debug)
    graphics_draw_rect(ctx, week_rect);
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
  font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_FONT_55));
  font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LECO_FONT_16));
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
