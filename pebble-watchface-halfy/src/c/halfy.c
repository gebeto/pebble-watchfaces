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
  // GFont font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  GFont font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
  // GFont font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);

  GSize text_size = graphics_text_layout_get_content_size("00", font, layer_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter);
  int text_gap = 16;

  int hours_sections = clock_is_24h_style() ? HOURS_PER_DAY : HOURS_PER_DAY / 2;
  double single_hour_section_height = layer_bounds.size.h * 1.0 / hours_sections;
  int hours = tick_time->tm_hour;
  int hours_top_spacing = single_hour_section_height * hours;
  GRect left_rect = GRect(0, hours_top_spacing, layer_bounds.size.w / 2, layer_bounds.size.h - hours_top_spacing);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, left_rect, 0, GCornerNone);

  graphics_context_set_text_color(ctx, GColorWhite);
  if (left_rect.size.h < text_size.h * 2 + text_gap)
  {
    left_rect.origin.y = left_rect.origin.y - text_size.h - text_gap - 4;
    graphics_context_set_text_color(ctx, GColorBlack);
  }
  else
  {
    left_rect.origin.y = left_rect.origin.y + text_gap;
  }
  graphics_draw_text(ctx, s_hours_buffer, font, left_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  int minutes_sections = MINUTES_PER_HOUR;
  double single_minutes_section_height = layer_bounds.size.h * 1.0 / minutes_sections;
  int minutes = tick_time->tm_min;
  // int minutes = 40;
  int minutes_top_spacing = single_minutes_section_height * minutes;
  GRect right_rect = GRect(layer_bounds.size.w / 2, minutes_top_spacing, layer_bounds.size.w / 2, layer_bounds.size.h - minutes_top_spacing);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, right_rect, 0, GCornerNone);
  graphics_context_set_text_color(ctx, GColorWhite);
  if (right_rect.size.h < text_size.h * 2 + text_gap)
  {
    right_rect.origin.y = right_rect.origin.y - text_size.h - text_gap - 4;
    graphics_context_set_text_color(ctx, GColorBlack);
  }
  else
  {
    right_rect.origin.y = right_rect.origin.y + text_gap;
  }
  graphics_draw_text(ctx, s_minutes_buffer, font, right_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, layer_bounds);
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
