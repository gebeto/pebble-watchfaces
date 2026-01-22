#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_layer;

// Write the current hours and minutes into a buffer
static struct tm *s_tick_time;

static GBitmap *s_hours_bitmap;
static GBitmap *s_minutes_bitmap;

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

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);

  int hours_rotation = s_tick_time->tm_hour * 30 + s_tick_time->tm_min * 0.5;
  int minutes_rotation = s_tick_time->tm_min * 6;

  GPoint screen_center = GPoint(layer_bounds.size.w / 2, layer_bounds.size.h / 2);
  GRect minutes_bounds = gbitmap_get_bounds(s_minutes_bitmap);
  GRect hours_bounds = gbitmap_get_bounds(s_hours_bitmap);

  // Get the bounds of the image
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_rotated_bitmap(ctx, s_minutes_bitmap, GPoint(minutes_bounds.size.w / 2, minutes_bounds.size.h / 2), DEG_TO_TRIGANGLE(minutes_rotation), screen_center);
  graphics_draw_rotated_bitmap(ctx, s_hours_bitmap, GPoint(hours_bounds.size.w / 2, hours_bounds.size.h / 2), DEG_TO_TRIGANGLE(hours_rotation), screen_center);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_circle(ctx, screen_center, hours_bounds.size.w / 2 + 1);
  graphics_draw_circle(ctx, screen_center, minutes_bounds.size.w / 2 + 1);
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
  s_hours_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOURS);
  s_minutes_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTES);
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void)
{
  gbitmap_destroy(s_hours_bitmap);
  gbitmap_destroy(s_minutes_bitmap);
  window_destroy(s_window);
}

int main(void)
{
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
