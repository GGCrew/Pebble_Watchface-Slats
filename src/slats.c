#include <pebble.h>


static Window *window;

static TextLayer *text_time_layer;


void update_display_time(struct tm *tick_time) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  char *time_format;

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

  text_layer_set_text(text_time_layer, time_text);
}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_display_time(tick_time);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	text_time_layer = text_layer_create(bounds);
	text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_time_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
}


static void window_unload(Window *window) {
	text_layer_destroy(text_time_layer);
}


static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

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
