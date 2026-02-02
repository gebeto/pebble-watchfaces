#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_layer;

// Write the current hours and minutes into a buffer
static struct tm *tick_time;
static char s_hours_buffer[8] = "00";
static char s_minutes_buffer[8] = "00";

static void update_time()
{
  time_t temp = time(NULL);
  tick_time = localtime(&temp);

  strftime(s_hours_buffer, sizeof(s_hours_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
  strftime(s_minutes_buffer, sizeof(s_minutes_buffer), "%M", tick_time);
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
  GFont font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
  // GSize text_size = graphics_text_layout_get_content_size("00", font, layer_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(layer_bounds.size.w / 2, layer_bounds.size.h / 2), layer_bounds.size.w / 2 - 8);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(layer_bounds.size.w / 2, layer_bounds.size.h / 2), layer_bounds.size.w / 2 - 16);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(layer_bounds.size.w / 2, layer_bounds.size.h / 2), layer_bounds.size.w / 2 - 24);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, GPoint(layer_bounds.size.w / 2, layer_bounds.size.h / 2), layer_bounds.size.w / 2 - 32);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(layer_bounds.size.w / 2, layer_bounds.size.h / 2), layer_bounds.size.w / 2 - 40);

  // Clock
  graphics_context_set_text_color(ctx, GColorWhite);

  int hours_top_spacing = layer_bounds.size.h / 2 - 25;
  GRect left_rect = GRect(60, hours_top_spacing, layer_bounds.size.w / 2 - 60, 30);
  graphics_draw_text(ctx, s_hours_buffer, font, left_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  int minutes_top_spacing = layer_bounds.size.h / 2 - 0;
  GRect right_rect = GRect(layer_bounds.size.w / 2, minutes_top_spacing, layer_bounds.size.w / 2 - 60, 30);
  graphics_draw_text(ctx, s_minutes_buffer, font, right_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
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
