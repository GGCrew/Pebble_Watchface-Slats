#include <pebble.h>


/**/


#define TIME_X_ORIGIN 0
#define TIME_Y_ORIGIN 50
#define SLAT_COUNT 64
#define ANIMATION_DURATION 500

/**/


static Window *window;

static TextLayer *text_time_layers[SLAT_COUNT];

static PropertyAnimation *slat_animations[SLAT_COUNT];


void update_display_time(struct tm *tick_time) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  char *time_format;
	int slat_counter;

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

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		text_layer_set_text(text_time_layers[slat_counter], time_text);
	}
}


void animation_stopped(Animation *animation, bool finished, void *property_animation) {
	if(finished){property_animation_destroy(property_animation);}
}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	int slat_counter;
	int delay;
	
	update_display_time(tick_time);

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		// Move offscreen
		layer_set_frame(text_layer_get_layer(text_time_layers[slat_counter]), GRect(TIME_X_ORIGIN, 170, 144, 1));
		
		delay = slat_counter * 100;
		slat_animations[slat_counter] = property_animation_create_layer_frame(
			text_layer_get_layer(text_time_layers[slat_counter]), 
			NULL, 
			&GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN+slat_counter, 144, 1)
		);
		animation_set_curve(property_animation_get_animation(slat_animations[slat_counter]), AnimationCurveEaseOut);
		animation_set_delay(property_animation_get_animation(slat_animations[slat_counter]), 0 + delay);
		animation_set_duration(property_animation_get_animation(slat_animations[slat_counter]), ANIMATION_DURATION);
		animation_set_handlers(property_animation_get_animation(slat_animations[slat_counter]), 
														(AnimationHandlers) {
															.started = NULL,
															.stopped = (AnimationStoppedHandler) animation_stopped,
														},
														slat_animations[slat_counter]);
		animation_schedule(property_animation_get_animation(slat_animations[slat_counter]));	
	}
}


static void window_load(Window *window) {
	int slat_counter;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "%d, %d, %d, %d", slat_counter, TIME_Y_ORIGIN+slat_counter, -slat_counter, TIME_Y_ORIGIN+slat_counter-slat_counter);
		//text_time_layers[slat_counter] = text_layer_create(GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN+slat_counter, 144, SLAT_COUNT));
		text_time_layers[slat_counter] = text_layer_create(layer_get_frame(window_layer));
		text_layer_set_font(text_time_layers[slat_counter], fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
		text_layer_set_text_color(text_time_layers[slat_counter], GColorWhite);
		text_layer_set_text_alignment(text_time_layers[slat_counter], GTextAlignmentCenter);
		text_layer_set_background_color(text_time_layers[slat_counter], GColorClear);

		//layer_set_bounds(text_layer_get_layer(text_time_layers[slat_counter]), GRect(0, 0, 144, SLAT_COUNT));
		//layer_set_bounds(text_layer_get_layer(text_time_layers[slat_counter]), GRect(0, -slat_counter / 2, 144, SLAT_COUNT));

		layer_add_child(window_layer, text_layer_get_layer(text_time_layers[slat_counter]));
		layer_set_frame(text_layer_get_layer(text_time_layers[slat_counter]), GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN+slat_counter, 144, 1));
		layer_set_bounds(text_layer_get_layer(text_time_layers[slat_counter]), GRect(0, -slat_counter, 144, SLAT_COUNT));
		//bounds = layer_get_bounds(text_layer_get_layer(text_time_layers[slat_counter]));
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "%d, %d, %d, %d", bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
		
/*
		if(slat_counter == 0) {
			layer_add_child(window_layer, text_layer_get_layer(text_time_layers[slat_counter]));
			layer_set_frame(text_layer_get_layer(text_time_layers[slat_counter]), GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN+slat_counter, 72, SLAT_COUNT));
			layer_set_bounds(text_layer_get_layer(text_time_layers[slat_counter]), GRect(0, -slat_counter / 2, 144, SLAT_COUNT));
			bounds = layer_get_bounds(text_layer_get_layer(text_time_layers[slat_counter]));
			APP_LOG(APP_LOG_LEVEL_DEBUG, "%d, %d, %d, %d", bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
		}
		if(slat_counter == 24) {
			layer_add_child(window_layer, text_layer_get_layer(text_time_layers[slat_counter]));
			//layer_set_bounds(text_layer_get_layer(text_time_layers[slat_counter]), GRect(0, slat_counter, 144, SLAT_COUNT));
			bounds = layer_get_bounds(text_layer_get_layer(text_time_layers[slat_counter]));
			APP_LOG(APP_LOG_LEVEL_DEBUG, "%d, %d, %d, %d", bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
		}
*/
	}
}


static void window_unload(Window *window) {
	int slat_counter;
	
	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		text_layer_destroy(text_time_layers[slat_counter]);
	}
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
