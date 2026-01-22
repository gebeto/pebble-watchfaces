#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_layer;

GFont s_font_tiny;
GFont s_font_middle;
GFont s_font_clock;
GFont s_font_seconds;

static char s_time_buffer[8] = "00:00";

static void update_time()
{
  time_t temp = time(NULL);
  tm *tick_time = localtime(&temp);
  strftime(s_time_buffer, sizeof(s_time_buffer), "%H:%M", tick_time);
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

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);

  graphics_context_set_fill_color(ctx, GColorWhite);
  int screen_height = 66;
  GRect screen_bounds = GRect(1, layer_bounds.size.h / 2 - screen_height / 2, layer_bounds.size.w - 2, screen_height);
  graphics_fill_rect(ctx, screen_bounds, 8, GCornersAll);

  graphics_context_set_text_color(ctx, GColorBlack);
  GRect clock_bounds = GRect(layer_bounds.origin.x - 14, layer_bounds.size.h / 2 - 30, layer_bounds.size.w, 64);
  graphics_draw_text(
      ctx, s_time_buffer,
      // ctx, "00:00",
      s_font_clock,
      GRect(screen_bounds.origin.x + 8, screen_bounds.origin.y + 12, screen_bounds.size.w, screen_bounds.size.h),
      GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  // graphics_context_set_fill_color(ctx, GColorBlack);
  // graphics_fill_rect(ctx, GRect(screen_bounds.origin.x + 20, screen_bounds.origin.y + 16, 16, 6), 0, GCornerNone);

  graphics_draw_text(
      ctx, "45",
      s_font_seconds,
      GRect(screen_bounds.origin.x + 104, screen_bounds.origin.y + 24, screen_bounds.size.w, screen_bounds.size.h),
      GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(
      ctx, "TH      22",
      s_font_middle,
      GRect(screen_bounds.origin.x - 4, screen_bounds.origin.y, screen_bounds.size.w - 6, 24),
      GTextOverflowModeFill, GTextAlignmentRight, NULL);
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
  s_font_tiny = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CASIO_FONT_12));
  s_font_middle = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CASIO_FONT_24));
  s_font_seconds = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CASIO_FONT_30));
  s_font_clock = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CASIO_FONT_42));
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
