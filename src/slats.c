#include <pebble.h>


/**/


#define TIME_X_ORIGIN 0
#define TIME_Y_ORIGIN 50
#define SLAT_COUNT 64
#define ANIMATION_DURATION 500

/**/


static Window *window;

static TextLayer *text_time_layer;

// Temporarily commented out to focus on setting & reading frame_buffer
//static Layer *slat_layers[SLAT_COUNT];

static PropertyAnimation *slat_animations[SLAT_COUNT];


void text_time_layer_update_proc(Layer *layer, GContext *ctx)
{
	uint8_t text_color_ARGB8;
	uint8_t background_color_ARGB8;
	GRect layer_rect;
	const char *time_text;
	GFont time_font;
	GRect time_rect = {{0, 45}, {144, 105}};

	// Get required data
	time_text = text_layer_get_text(text_time_layer);
	time_font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);	// This should be defined elsewhere

	// Set background
	background_color_ARGB8 = GColorBlackARGB8;
	graphics_context_set_fill_color(ctx, (GColor8){.argb=background_color_ARGB8});
	graphics_fill_rect(ctx, layer_rect, 0, GCornerNone);

	// Set time text
	text_color_ARGB8 = GColorWhiteARGB8;
	graphics_context_set_text_color(ctx, (GColor8){.argb=text_color_ARGB8});
	graphics_draw_text(ctx, 
											time_text,
											time_font,
											time_rect,
											GTextOverflowModeWordWrap,
											GTextAlignmentCenter,
											NULL);
}


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


void animation_stopped(Animation *animation, bool finished, void *property_animation) {
	if(finished){property_animation_destroy(property_animation);}
}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	int slat_counter;
	int delay;
	
	update_display_time(tick_time);

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
	/*  Temporarily commented out to focus on setting & reading frame_buffer
		// Move offscreen
		layer_set_frame(slat_layers[slat_counter], GRect(TIME_X_ORIGIN, 170, 144, 1));

		// Animate slats
		delay = slat_counter * 100;
		slat_animations[slat_counter] = property_animation_create_layer_frame(
			slat_layers[slat_counter], 
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
	*/
	}
}


static void window_load(Window *window) {
	int slat_counter;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	text_time_layer = text_layer_create(layer_get_frame(window_layer));
	text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_time_layer, GColorClear);
	layer_set_update_proc(text_layer_get_layer(text_time_layer), text_time_layer_update_proc);
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
	/*  Temporarily commented out to focus on setting & reading frame_buffer
		slat_layers[slat_counter] = layer_create(layer_get_frame(window_layer));

		// Create a new frame
		// Set the frame dimensions to 1 pixel high
		// Adjust the bounds to shift the layer's offset up
		layer_add_child(window_layer, slat_layers[slat_counter]);
		layer_set_frame(slat_layers[slat_counter], GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN+slat_counter, 144, 1));
	*/
	}
}


static void window_unload(Window *window) {
	int slat_counter;

	text_layer_destroy(text_time_layer);

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		// Temporarily commented out to focus on setting & reading frame_buffer
		//layer_destroy(slat_layers[slat_counter]);
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
