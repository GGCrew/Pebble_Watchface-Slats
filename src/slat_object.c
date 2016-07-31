#include "constants.h"
#include "slat_object.h"


/**/


void slat_animation_stopped(Animation *animation, bool finished, void *property_animation);
void update_slat_bitmaps(SlatObject *slat_object);
void slat_object_update_time_bitmap(SlatObject *slat_object, TextLayer *time_text_layer, GContext *ctx);


/**/


SlatObject* slat_object_create() {
	int slat_counter;
	GSize bitmap_size;
	GBitmapFormat bitmap_format;
	
	SlatObject *slat_object = malloc(sizeof(SlatObject));

	/**/
	
	bitmap_size = GSize(144, SLAT_COUNT);
	bitmap_format = GBitmapFormat1Bit;	// for Aplite
	slat_object->time_bitmap = gbitmap_create_blank(bitmap_size, bitmap_format);

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		slat_object->slat_layers[slat_counter] = bitmap_layer_create(GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN + slat_counter, 144, 1));
		slat_object->slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(slat_object->time_bitmap, GRect(0, slat_counter, 144, 1));
		bitmap_layer_set_bitmap(slat_object->slat_layers[slat_counter], slat_object->slat_bitmaps[slat_counter]);
	}
	
	return slat_object;
}

void slat_object_destroy(SlatObject *slat_object) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_destroy()...");
	int slat_counter;

	/**/

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_counter: %d", slat_counter);
		gbitmap_destroy(slat_object->slat_bitmaps[slat_counter]);
		layer_destroy(bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]));
	}

	APP_LOG(APP_LOG_LEVEL_DEBUG, "gbitmap_destroy(slat_object->time_bitmap)...");
	gbitmap_destroy(slat_object->time_bitmap);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "free(slat_object)...");
	free(slat_object);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_destroy() complete!");
}


void slat_object_update_time(SlatObject *slat_object, TextLayer *time_text_layer, GContext *ctx) {
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
	time_bitmap_data = gbitmap_get_data(slat_object->time_bitmap);
	time_bitmap_format = gbitmap_get_format(slat_object->time_bitmap);
	time_bitmap_row_size = gbitmap_get_bytes_per_row(slat_object->time_bitmap);

	// Point the screen bitmap (aka frame buffer) to the time bitmap (so we can render "offscreen")
	// This trick pulled from 2015 Pebble Developer Retreat presentation on Graphics by Matthew Hungerford
	gbitmap_set_data(screen_bitmap, time_bitmap_data, time_bitmap_format, time_bitmap_row_size, false);

	slat_object_update_time_bitmap(slat_object, time_text_layer, ctx);

	// Point the screen bitmap (aka frame buffer) back to it's original data
	gbitmap_set_data(screen_bitmap, screen_bitmap_data, screen_bitmap_format, screen_bitmap_row_size, false);

	update_slat_bitmaps(slat_object);
}


void layer_add_slat_object(Layer *layer, SlatObject *slat_object) {
	int slat_counter;

	/**/

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		layer_add_child(layer, bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]));
	}
}


void update_slat_bitmaps(SlatObject *slat_object) {
	int slat_counter;

	/**/

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		// Could use memcpy() here instead of destroy/create ???
		gbitmap_destroy(slat_object->slat_bitmaps[slat_counter]);
		slat_object->slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(slat_object->time_bitmap, GRect(0, slat_counter, 144, 1));
		bitmap_layer_set_bitmap(slat_object->slat_layers[slat_counter], slat_object->slat_bitmaps[slat_counter]);
	}
}


void slat_object_update_time_bitmap(SlatObject *slat_object, TextLayer *time_text_layer, GContext *ctx) {
	const char *time_text;
	GFont time_font;
	GRect time_rect = {{0, 0}, {144, SLAT_COUNT}};

	uint8_t text_color_ARGB8;
	uint8_t background_color_ARGB8;

	/**/

	// Get text data
	time_text = text_layer_get_text(time_text_layer);
	//time_font = text_layer_get_font(time_text_layer);  <--- Sure wish this function existed!
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


void slat_object_animate(SlatObject *slat_object) {
	int slat_counter;
	int animation_counter;
	int delay;
	int start_position_x_offset;

	GRect start_position;
	GRect stop_position;

	/**/
	
	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		// Move offscreen
		layer_set_frame(bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]), GRect(144, TIME_Y_ORIGIN + slat_counter, 144, 1));

		// Animate slats
		start_position_x_offset = (((slat_counter & 1) == 1) ? 144 : -144);  // Zipper effect
		start_position = GRect(start_position_x_offset, TIME_Y_ORIGIN + slat_counter, 144, 1);
		stop_position = GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN + slat_counter, 144, 1);
		
		// Slide in
		delay = slat_counter * ANIMATION_DELAY;
		animation_counter = slat_counter;
		slat_object->slat_animations[animation_counter] = property_animation_create_layer_frame(
			bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]), 
			&start_position, 
			&stop_position
		);
		animation_set_curve(property_animation_get_animation(slat_object->slat_animations[animation_counter]), AnimationCurveEaseOut);
		animation_set_delay(property_animation_get_animation(slat_object->slat_animations[animation_counter]), 0 + delay);
		animation_set_duration(property_animation_get_animation(slat_object->slat_animations[animation_counter]), ANIMATION_DURATION);
		animation_set_handlers(property_animation_get_animation(slat_object->slat_animations[animation_counter]), 
														(AnimationHandlers) {
															.started = NULL,
															.stopped = (AnimationStoppedHandler) slat_animation_stopped,
														},
														slat_object->slat_animations[animation_counter]);
		animation_schedule(property_animation_get_animation(slat_object->slat_animations[animation_counter]));
	}
}


void slat_animation_stopped(Animation *animation, bool finished, void *property_animation) {
	if(finished){property_animation_destroy(property_animation);}
}


