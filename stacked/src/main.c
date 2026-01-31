#include <pebble.h>

#define SETTINGS_KEY 1
typedef struct ClaySettings
{
  GColor BackgroundColor;
  GColor ForegroundColor;
} __attribute__((__packed__)) ClaySettings;

ClaySettings settings;

static void prv_default_settings()
{
  settings.BackgroundColor = GColorWhite;
  settings.ForegroundColor = GColorBlack;
}

// Read settings from persistent storage
static void prv_load_settings()
{
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings()
{
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  // prv_update_display();
}

static Window *s_window;
static Layer *s_before_layer;
static Layer *s_text_layer;
static Layer *s_after_layer;

// Write the current hours and minutes into a buffer
static struct tm *tick_time;
static char s_time_buffer[8] = "00:00";
// static char s_time_to_render[8] = clock_copy_time_string(s_time_buffer);

static Animation *s_open_animation;
static Animation *s_close_open_animation;

int hour = 0;

static bool initial_ticked = false;

static void update_time()
{
  time_t temp = time(NULL);
  tick_time = localtime(&temp);
  clock_copy_time_string(s_time_buffer, 8);
}

static void update_screen()
{
  layer_mark_dirty(s_before_layer);
  layer_mark_dirty(s_text_layer);
  layer_mark_dirty(s_after_layer);

  if (initial_ticked)
  {
    animation_schedule(animation_clone(s_close_open_animation));
  }
  initial_ticked = true;
}

static Animation *create_open_animation(int delay_ms, GRect opened_bounds, GRect closed_bounds)
{
  PropertyAnimation *open_prop_anim = property_animation_create_layer_frame(s_before_layer, &closed_bounds, &opened_bounds);
  Animation *s_open_animation = property_animation_get_animation(open_prop_anim);
  animation_set_curve(s_open_animation, AnimationCurveEaseOut);
  animation_set_duration(s_open_animation, 500);
  animation_set_delay(s_open_animation, delay_ms);
  return s_open_animation;
}

static Animation *create_close_open_sequence_animation(GRect opened_bounds, GRect closed_bounds)
{
  Animation *s_open_animation = create_open_animation(0, opened_bounds, closed_bounds);
  Animation *s_close_animation = create_open_animation(0, closed_bounds, opened_bounds);
  animation_set_delay(s_open_animation, 300);

  return animation_sequence_create(s_close_animation, s_open_animation, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time();
  update_screen();
}

void draw_card(GContext *ctx, GRect card_bounds_border)
{
  GRect card_bounds_inner = GRect(
      card_bounds_border.origin.x + 2,
      card_bounds_border.origin.y + 2,
      card_bounds_border.size.w - 4,
      70 - 4);
  graphics_context_set_fill_color(ctx, settings.ForegroundColor);
  graphics_fill_rect(ctx, card_bounds_border, 10, GCornersAll);
  graphics_context_set_fill_color(ctx, settings.BackgroundColor);
  graphics_fill_rect(ctx, card_bounds_inner, 8, GCornersAll);
}

static void canvas_update_window_proc(Layer *layer, GContext *ctx)
{
  GRect layer_bounds = layer_get_bounds(layer);

  // graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, settings.BackgroundColor);
  graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);
}
static void canvas_update_before_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_antialiased(ctx, true);
  GRect layer_bounds = layer_get_bounds(layer);

  int hour = tick_time->tm_hour;
  int top_gap = 0;
  for (int i = 0; i < hour + 1; i++)
  {
    draw_card(ctx, GRect(1, 5 + top_gap, layer_bounds.size.w - 2, 70));
    top_gap += 5;
  }
}

static void canvas_update_text_proc(Layer *layer, GContext *ctx)
{

  GRect layer_bounds = layer_get_bounds(layer);
  GFont font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  int hour = tick_time->tm_hour;
  int top_gap = 0 + 5 * hour - 2;
  GRect card_bounds_inner = GRect(3, 7 + top_gap, layer_bounds.size.w - 2 - 4, 70 - 4);

  // graphics_context_set_text_color(ctx, GColorWhite);
  graphics_context_set_text_color(ctx, settings.ForegroundColor);
  graphics_draw_text(
      ctx, s_time_buffer, font,
      GRect(card_bounds_inner.origin.x, card_bounds_inner.origin.y - 5, card_bounds_inner.size.w, card_bounds_inner.size.h),
      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void canvas_update_after_proc(Layer *layer, GContext *ctx)
{
  GRect layer_bounds = layer_get_bounds(layer);
  int hour = tick_time->tm_hour;
  int top_gap = 42 + 5 * hour;
  for (int i = hour; i < 23; i++)
  {
    draw_card(ctx, GRect(1, 5 + top_gap, layer_bounds.size.w - 2, 70));
    top_gap += 5;
  }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction)
{
  hour += 1;
  if (hour > 23)
  {
    hour = 0;
  }
  update_screen();
}

static void prv_window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GRect closed_bounds = GRect(bounds.origin.x, bounds.origin.y + 42 - 5, bounds.size.w, bounds.size.h);
  GRect opened_bounds = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);

  s_before_layer = layer_create(closed_bounds);
  s_text_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  s_after_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));

  update_time();

  layer_set_update_proc(window_layer, canvas_update_window_proc);
  layer_set_update_proc(s_before_layer, canvas_update_before_proc);
  layer_set_update_proc(s_text_layer, canvas_update_text_proc);
  layer_set_update_proc(s_after_layer, canvas_update_after_proc);

  layer_add_child(window_layer, s_before_layer);
  layer_add_child(s_before_layer, s_text_layer);
  layer_add_child(window_layer, s_after_layer);

  layer_mark_dirty(s_before_layer);
  layer_mark_dirty(s_text_layer);

  // create animations
  s_open_animation = create_open_animation(300, opened_bounds, closed_bounds);
  s_close_open_animation = create_close_open_sequence_animation(opened_bounds, closed_bounds);
  // end create animations

  // start loops
  animation_schedule(animation_clone(s_open_animation));
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // accel_tap_service_subscribe(accel_tap_handler);
}

static void prv_window_unload(Window *window)
{
  layer_destroy(s_before_layer);
}

// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context)
{
  // Background Color
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  if (bg_color_t)
  {
    settings.BackgroundColor = GColorFromHEX(bg_color_t->value->int32);
  }

  // Foreground Color
  Tuple *fg_color_t = dict_find(iter, MESSAGE_KEY_ForegroundColor);
  if (fg_color_t)
  {
    settings.ForegroundColor = GColorFromHEX(fg_color_t->value->int32);
  }

  prv_save_settings();
}

static void prv_init(void)
{
  prv_load_settings();
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

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
