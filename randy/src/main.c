#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_layer;
static struct tm *s_tick_time;
static GBitmap *s_background_bitmap;

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

GPoint sum_point(GPoint start, GPoint sum)
{
  return GPoint(start.x + sum.x, start.y + sum.y);
}

static void canvas_update_proc(Layer *layer, GContext *ctx)
{
  graphics_context_set_antialiased(ctx, true);

  GRect layer_bounds = layer_get_bounds(layer);
  GPoint center = GPoint(layer_bounds.size.w / 2, layer_bounds.size.h / 2);
  GFont font = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_bounds, 0, GCornerNone);

  // Get the bounds of the image
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, s_background_bitmap, layer_bounds);

  int seconds = s_tick_time->tm_sec;
  int minutes = s_tick_time->tm_min;
  int hours = s_tick_time->tm_hour;

  int seconds_angle = seconds * 6;
  int minute_angle = minutes * 6;
  int hours_angle = hours % 12 * 30;

  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  int32_t minute_angle_rad = TRIG_MAX_ANGLE * minute_angle / 360;
  int MINUTE_HAND_LENGTH = 48;
  graphics_draw_line(
      ctx,
      sum_point(
          center,
          GPoint(
              -sin_lookup(minute_angle_rad) * 10 / TRIG_MAX_RATIO,
              cos_lookup(minute_angle_rad) * 10 / TRIG_MAX_RATIO)),
      sum_point(
          center,
          GPoint(
              sin_lookup(minute_angle_rad) * MINUTE_HAND_LENGTH / TRIG_MAX_RATIO,
              -cos_lookup(minute_angle_rad) * MINUTE_HAND_LENGTH / TRIG_MAX_RATIO)));

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 3);
  int32_t hours_angle_rad = TRIG_MAX_ANGLE * hours_angle / 360;
  int HOUR_HAND_LENGTH = 30;
  graphics_draw_line(
      ctx,
      sum_point(
          center,
          GPoint(
              -sin_lookup(hours_angle_rad) * 10 / TRIG_MAX_RATIO,
              cos_lookup(hours_angle_rad) * 10 / TRIG_MAX_RATIO)),
      sum_point(
          center,
          GPoint(
              sin_lookup(hours_angle_rad) * HOUR_HAND_LENGTH / TRIG_MAX_RATIO,
              -cos_lookup(hours_angle_rad) * HOUR_HAND_LENGTH / TRIG_MAX_RATIO)));

  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 1);
  int32_t seconds_angle_rad = TRIG_MAX_ANGLE * seconds_angle / 360;
  int SECONDS_HAND_LENGTH = 70;
  graphics_draw_line(
      ctx,
      sum_point(
          center,
          GPoint(
              -sin_lookup(seconds_angle_rad) * 14 / TRIG_MAX_RATIO,
              cos_lookup(seconds_angle_rad) * 14 / TRIG_MAX_RATIO)),
      sum_point(
          center,
          GPoint(
              sin_lookup(seconds_angle_rad) * SECONDS_HAND_LENGTH / TRIG_MAX_RATIO,
              -cos_lookup(seconds_angle_rad) * SECONDS_HAND_LENGTH / TRIG_MAX_RATIO)));

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, 5);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, 2);

  // graphics_draw_rotated_bitmap(ctx, s_hours_bitmap, GPoint(90, 90), DEG_TO_TRIGANGLE(hours_rotation), GPoint(90, 90));
  // graphics_draw_rotated_bitmap(ctx, s_minutes_bitmap, GPoint(65, 65), DEG_TO_TRIGANGLE(minutes_rotation), GPoint(90, 90));
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

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
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
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void)
{
  gbitmap_destroy(s_background_bitmap);
  window_destroy(s_window);
}

int main(void)
{
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
