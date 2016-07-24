#include <pebble.h>


/**/


#define TIME_X_ORIGIN 0
#define TIME_Y_ORIGIN 50
#define SLAT_COUNT 64
#define ANIMATION_DURATION 500


/**/


static Window *window;

static TextLayer *text_time_layer;
//static BitmapLayer *slat_layers[SLAT_COUNT];
static BitmapLayer *bitmap_time_layer;

static GBitmap *time_bitmap;
//static GBitmap *slat_bitmaps[SLAT_COUNT];

//static PropertyAnimation *slat_animations[SLAT_COUNT];


//void slat_layer_update_proc(Layer *layer, GContext *ctx)
//{
//	graphics_draw_bitmap_in_rect(ctx, slat_bitmaps[SLAT_COUNT / 2], GRect(0,0,144,1));
//}


void text_time_layer_update_proc(Layer *layer, GContext *ctx)
{
	GBitmap *bitmap;
	GBitmapFormat bitmap_format;
	uint8_t *bitmap_data;
	//uint8_t bitmap_row_data[144];

	uint8_t text_color_ARGB8;
	uint8_t background_color_ARGB8;
	GRect layer_rect = {{0, 0}, {144, 168}};;
	const char *time_text;
	GFont time_font;
	GRect time_rect = {{0, 0 + 0}, {144, 60 + 0}};

	int slat_counter;

	// Get required data
	time_text = text_layer_get_text(text_time_layer);
	time_font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);	// This should be defined elsewhere

	// Hide other layers
//	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
//		layer_set_hidden(bitmap_layer_get_layer(slat_layers[slat_counter]), true);
//	}

	// Set background
	background_color_ARGB8 = GColorWhiteARGB8;
	graphics_context_set_fill_color(ctx, (GColor8){.argb=background_color_ARGB8});
	graphics_fill_rect(ctx, layer_rect, 0, GCornerNone);

	// Set time text
	text_color_ARGB8 = GColorBlackARGB8;
	graphics_context_set_text_color(ctx, (GColor8){.argb=text_color_ARGB8});
	graphics_draw_text(ctx, 
											time_text,
											time_font,
											time_rect,
											GTextOverflowModeWordWrap,
											GTextAlignmentCenter,
											NULL);

	// Get buffer data (which also locks the buffer)
	bitmap = graphics_capture_frame_buffer(ctx);
	bitmap_format = gbitmap_get_format(bitmap);
	bitmap_data = gbitmap_get_data(bitmap);

	// Best guess on origin of "20" value in following gbitmap_set_data() function:
	//  The row is 144 pixels wide.
	//  Because I'm developing/testing for Aplite (aka original Pebble), each pixel is represented by 1 bit.
	//  The "row_size" value must be in bytes.
	//  8 bits = 1 byte.
	//  144 bits = 18 bytes.
	//  The "row_size" value must be a multiple of 4.
	//  18 rounded up to the nearest multiple of 4 = 20.
	gbitmap_set_data(time_bitmap, bitmap_data, bitmap_format, 20, false);

//	APP_LOG(APP_LOG_LEVEL_DEBUG, "bitmap_format: %d", bitmap_format);

	// Update slat bitmaps
//	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
//	
//	}

//	// Create slats
//	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
//		// try gbitmap_create_as_sub_bitmap() instead of memcpy + gbitmap_create_with_data
//		memcpy(bitmap_row_data, bitmap_data + (slat_counter * 144), 144);
//		slat_bitmaps[slat_counter] = gbitmap_create_with_data(bitmap_row_data);
//	}

	// Commit changes to buffer (if any) and unlock the buffer
	graphics_release_frame_buffer(ctx, bitmap);	

	// Unhide other layers
//	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
//		layer_set_hidden(bitmap_layer_get_layer(slat_layers[slat_counter]), false);
//		layer_mark_dirty(bitmap_layer_get_layer(slat_layers[slat_counter]));
//	}
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


//void animation_stopped(Animation *animation, bool finished, void *property_animation) {
//	if(finished){property_animation_destroy(property_animation);}
//}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	int slat_counter;
	int delay;
	
	update_display_time(tick_time);

	/*  Temporarily commented out to focus on setting & reading frame_buffer
	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
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
	}
	*/
}


static void window_load(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_load...");
	int slat_counter;
	GSize bitmap_size;
	GBitmapFormat bitmap_format;
	GRect bitmap_frame;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	text_time_layer = text_layer_create(layer_get_frame(window_layer));
	text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_time_layer, GColorClear);
	layer_set_update_proc(text_layer_get_layer(text_time_layer), text_time_layer_update_proc);
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

	bitmap_size = GSize(144, 168);
	bitmap_format = GBitmapFormat1Bit;	// for Aplite
	time_bitmap = gbitmap_create_blank(bitmap_size, bitmap_format);

	bitmap_time_layer = bitmap_layer_create(GRect(0, 70, 144, 60));
	bitmap_layer_set_bitmap(bitmap_time_layer, time_bitmap);
	layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_time_layer));

/*
	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_counter: %d", slat_counter);
		
		//slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(time_bitmap, GRect(0, slat_counter, 144, 1));

		//APP_LOG(APP_LOG_LEVEL_DEBUG, "bitmap_layer_create...");
		slat_layers[slat_counter] = bitmap_layer_create(GRect(0 + slat_counter, slat_counter + 60, 144, 1));

		bitmap_frame = GRect(0, slat_counter, 144, 1);
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "gbitmap_create_as_sub_bitmap...");
		slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(time_bitmap, bitmap_frame);

		//APP_LOG(APP_LOG_LEVEL_DEBUG, "bitmap_layer_set_bitmap...");
		bitmap_layer_set_bitmap(slat_layers[slat_counter], slat_bitmaps[slat_counter]);

		// Create a new frame
		// Set the frame dimensions to 1 pixel high
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "layer_add_child...");
		layer_add_child(window_layer, bitmap_layer_get_layer(slat_layers[slat_counter]));
		//layer_set_frame(bitmap_layer_get_layer(slat_layers[slat_counter]), GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN+slat_counter + 64, 144, 1));
		//layer_set_update_proc(slat_layers[slat_counter], slat_layer_update_proc);
	}
*/
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_load complete!");
}


static void window_unload(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_unload...");
	int slat_counter;

	APP_LOG(APP_LOG_LEVEL_DEBUG, "text_layer_destroy(text_time_layer)...");
	text_layer_destroy(text_time_layer);

/*
	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_counter: %d", slat_counter);
		gbitmap_destroy(slat_bitmaps[slat_counter]);
		layer_destroy(bitmap_layer_get_layer(slat_layers[slat_counter]));
	}
*/

	APP_LOG(APP_LOG_LEVEL_DEBUG, "gbitmap_destroy(time_bitmap)...");
	gbitmap_destroy(time_bitmap);

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
