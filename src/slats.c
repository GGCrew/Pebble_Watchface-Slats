#include <pebble.h>


/**/


#define TIME_X_ORIGIN 0
#define TIME_Y_ORIGIN 50
#define SLAT_COUNT 64
#define ANIMATION_DURATION 1000
#define ANIMATION_DELAY 10


/**/


typedef struct {
	GBitmap *time_bitmap;
	GBitmap *slat_bitmaps[SLAT_COUNT];
	BitmapLayer *slat_layers[SLAT_COUNT];
	PropertyAnimation *slat_animations[SLAT_COUNT];
} SlatObject;

static Window *window;

static TextLayer *text_time_layer;

static SlatObject slat_object;


/**/


void update_time_bitmap(GContext *ctx) {
	const char *time_text;
	GFont time_font;
	GRect time_rect = {{0, 0}, {144, SLAT_COUNT}};

	uint8_t text_color_ARGB8;
	uint8_t background_color_ARGB8;

	/**/

	// Get text data
	time_text = text_layer_get_text(text_time_layer);
	time_font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);	// This should be defined elsewhere

	// Set background
	background_color_ARGB8 = GColorBlackARGB8;
	graphics_context_set_fill_color(ctx, (GColor8){.argb=background_color_ARGB8});
	graphics_fill_rect(ctx, time_rect, 0, GCornerNone);

	// Render text
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


void update_slat_bitmaps() {
	int slat_counter;

	/**/

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		// Could use memcpy() here instead of destroy/create ???
		gbitmap_destroy(slat_object.slat_bitmaps[slat_counter]);
		slat_object.slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(slat_object.time_bitmap, GRect(0, slat_counter, 144, 1));
		bitmap_layer_set_bitmap(slat_object.slat_layers[slat_counter], slat_object.slat_bitmaps[slat_counter]);
	}
}


void text_time_layer_update_proc(Layer *layer, GContext *ctx)
{
	GBitmap *screen_bitmap;
	GBitmapFormat screen_bitmap_format;
	uint8_t *screen_bitmap_data;
	uint16_t screen_bitmap_row_size;

	GBitmapFormat time_bitmap_format;
	uint8_t *time_bitmap_data;
	uint16_t time_bitmap_row_size;

	/**/

	// Get buffer data (which also locks the buffer)
	screen_bitmap = graphics_capture_frame_buffer(ctx);
	screen_bitmap_data = gbitmap_get_data(screen_bitmap);
	screen_bitmap_format = gbitmap_get_format(screen_bitmap);
	screen_bitmap_row_size = gbitmap_get_bytes_per_row(screen_bitmap);
	// Unlock the buffer
	graphics_release_frame_buffer(ctx, screen_bitmap);

	// Get time bitmap data
	time_bitmap_data = gbitmap_get_data(slat_object.time_bitmap);
	time_bitmap_format = gbitmap_get_format(slat_object.time_bitmap);
	time_bitmap_row_size = gbitmap_get_bytes_per_row(slat_object.time_bitmap);

	// Point the screen bitmap (aka frame buffer) to the time bitmap (so we can render "offscreen")
	// This trick pulled from 2015 Pebble Developer Retreat presentation on Graphics by Matthew Hungerford
	gbitmap_set_data(screen_bitmap, time_bitmap_data, time_bitmap_format, time_bitmap_row_size, false);

	update_time_bitmap(ctx);

	// Point the screen bitmap (aka frame buffer) back to it's original data
	gbitmap_set_data(screen_bitmap, screen_bitmap_data, screen_bitmap_format, screen_bitmap_row_size, false);

	update_slat_bitmaps();
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

	text_layer_set_text(text_time_layer, time_text);
}


void slat_animation_stopped(Animation *animation, bool finished, void *property_animation) {
	if(finished){property_animation_destroy(property_animation);}
}


void animate_slats() {
	int slat_counter;
	int animation_counter;
	int delay;
	int start_position_x_offset;

	GRect start_position;
	GRect stop_position;

	/**/
	
	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		// Move offscreen
		layer_set_frame(bitmap_layer_get_layer(slat_object.slat_layers[slat_counter]), GRect(144, TIME_Y_ORIGIN + slat_counter, 144, 1));

		// Animate slats
		start_position_x_offset = (((slat_counter & 1) == 1) ? 144 : -144);  // Zipper effect
		start_position = GRect(start_position_x_offset, TIME_Y_ORIGIN + slat_counter, 144, 1);
		stop_position = GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN + slat_counter, 144, 1);
		
		// Slide in
		delay = slat_counter * ANIMATION_DELAY;
		animation_counter = slat_counter;
		slat_object.slat_animations[animation_counter] = property_animation_create_layer_frame(
			bitmap_layer_get_layer(slat_object.slat_layers[slat_counter]), 
			&start_position, 
			&stop_position
		);
		animation_set_curve(property_animation_get_animation(slat_object.slat_animations[animation_counter]), AnimationCurveEaseOut);
		animation_set_delay(property_animation_get_animation(slat_object.slat_animations[animation_counter]), 0 + delay);
		animation_set_duration(property_animation_get_animation(slat_object.slat_animations[animation_counter]), ANIMATION_DURATION);
		animation_set_handlers(property_animation_get_animation(slat_object.slat_animations[animation_counter]), 
														(AnimationHandlers) {
															.started = NULL,
															.stopped = (AnimationStoppedHandler) slat_animation_stopped,
														},
														slat_object.slat_animations[animation_counter]);
		animation_schedule(property_animation_get_animation(slat_object.slat_animations[animation_counter]));
	}
}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_display_time(tick_time);
	animate_slats();
}


static void window_load(Window *window) {
	int slat_counter;
	GSize bitmap_size;
	GBitmapFormat bitmap_format;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	/**/

	text_time_layer = text_layer_create(bounds);
	text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_time_layer, GColorClear);
	layer_set_update_proc(text_layer_get_layer(text_time_layer), text_time_layer_update_proc);
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

	bitmap_size = GSize(144, SLAT_COUNT);
	bitmap_format = GBitmapFormat1Bit;	// for Aplite
	slat_object.time_bitmap = gbitmap_create_blank(bitmap_size, bitmap_format);

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		slat_object.slat_layers[slat_counter] = bitmap_layer_create(GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN + slat_counter, 144, 1));
		slat_object.slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(slat_object.time_bitmap, GRect(0, slat_counter, 144, 1));
		bitmap_layer_set_bitmap(slat_object.slat_layers[slat_counter], slat_object.slat_bitmaps[slat_counter]);
		layer_add_child(window_layer, bitmap_layer_get_layer(slat_object.slat_layers[slat_counter]));
	}
}


static void window_unload(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_unload...");
	int slat_counter;

	/**/

	APP_LOG(APP_LOG_LEVEL_DEBUG, "text_layer_destroy(text_time_layer)...");
	text_layer_destroy(text_time_layer);

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_counter: %d", slat_counter);
		gbitmap_destroy(slat_object.slat_bitmaps[slat_counter]);
		layer_destroy(bitmap_layer_get_layer(slat_object.slat_layers[slat_counter]));
	}

	APP_LOG(APP_LOG_LEVEL_DEBUG, "gbitmap_destroy(time_bitmap)...");
	gbitmap_destroy(slat_object.time_bitmap);

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
