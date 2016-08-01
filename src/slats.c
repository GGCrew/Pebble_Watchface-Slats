#include <pebble.h>
#include "slat_object.h"


/**/


static Window *window;

static TextLayer *time_text_layer;

static SlatObject *slat_object;


/**/


void time_text_layer_update_proc(Layer *layer, GContext *ctx)
{
	slat_object_set_text(slat_object, text_layer_get_text(time_text_layer));
	slat_object_render(slat_object, ctx);
}


void update_display_time(struct tm *tick_time) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  char *time_format;

	/**/

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

	text_layer_set_text(time_text_layer, time_text);
}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_display_time(tick_time);
	slat_object_animate(slat_object);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	/**/

	time_text_layer = text_layer_create(bounds);
	text_layer_set_font(time_text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_text_color(time_text_layer, GColorWhite);
	text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
	text_layer_set_background_color(time_text_layer, GColorClear);
	layer_set_update_proc(text_layer_get_layer(time_text_layer), time_text_layer_update_proc);
	layer_add_child(window_layer, text_layer_get_layer(time_text_layer));

	slat_object = slat_object_create((GRect){{0, -14}, {MAX_SCREEN_WIDTH, 49}});  // Just tall enough for the text
	slat_object_set_origin(slat_object, (GPoint){0, 50});
	slat_object_set_font(slat_object, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	slat_object_set_text_alignment(slat_object, GTextAlignmentCenter);
	slat_object_set_text_color(slat_object, GColorWhite);
	slat_object_set_background_color(slat_object, GColorBlack);
	slat_object_set_overflow_mode(slat_object, GTextOverflowModeWordWrap);
	layer_add_slat_object(window_layer, slat_object);
}


static void window_unload(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_unload...");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "text_layer_destroy(time_text_layer)...");
	text_layer_destroy(time_text_layer);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_destroy(slat_object)...");
	slat_object_destroy(slat_object);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_unload complete!");
}


static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  window_set_background_color(window, GColorBlack);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}


static void deinit(void) {
	tick_timer_service_unsubscribe();

  window_destroy(window);
}


int main(void) {
  init();

  app_event_loop();

  deinit();
}
