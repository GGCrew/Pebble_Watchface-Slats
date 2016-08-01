#include "slat_object.h"


/**/


#define TIME_X_ORIGIN 0
#define TIME_Y_ORIGIN 50
#define ANIMATION_DURATION 1000
#define ANIMATION_DELAY 10


/**/


// Declarations for private internal functions

void slat_object_render_text_bitmap(SlatObject *slat_object, GContext *ctx);
void slat_object_set_slat_bitmaps(SlatObject *slat_object);
void slat_animation_stopped(Animation *animation, bool finished, void *property_animation);


/**/


// Public functions declared in "slat_object.h"

SlatObject* slat_object_create(GRect rect) {
	int slat_counter;
	//GSize bitmap_size;
	GBitmapFormat bitmap_format;
	
	SlatObject *slat_object = malloc(sizeof(SlatObject));

	/**/

	slat_object->rect = rect;
	bitmap_format = GBitmapFormat1Bit;	// for Aplite
	slat_object->text_bitmap = gbitmap_create_blank(slat_object->rect.size, bitmap_format);

	for(slat_counter = 0; slat_counter < slat_object->rect.size.h; slat_counter++) {
		slat_object->slat_layers[slat_counter] = bitmap_layer_create(GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN + slat_counter, slat_object->rect.size.w, 1));
		slat_object->slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(slat_object->text_bitmap, GRect(slat_object->rect.origin.x, slat_counter, slat_object->rect.size.w, 1));
		bitmap_layer_set_bitmap(slat_object->slat_layers[slat_counter], slat_object->slat_bitmaps[slat_counter]);
	}
	
	return slat_object;
}


void slat_object_destroy(SlatObject *slat_object) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_destroy()...");
	int slat_counter;

	/**/

	for(slat_counter = 0; slat_counter < slat_object->rect.size.h; slat_counter++) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_counter: %d", slat_counter);
		gbitmap_destroy(slat_object->slat_bitmaps[slat_counter]);
		layer_destroy(bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]));
	}

	APP_LOG(APP_LOG_LEVEL_DEBUG, "gbitmap_destroy(slat_object->text_bitmap)...");
	gbitmap_destroy(slat_object->text_bitmap);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "free(slat_object)...");
	free(slat_object);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_destroy() complete!");
}


void slat_object_set_text(SlatObject *slat_object, const char *text) {
	slat_object->text = text;
}


void slat_object_set_font(SlatObject *slat_object, GFont font) {slat_object->font = font;}
void slat_object_set_text_alignment(SlatObject *slat_object, GTextAlignment text_alignment) {slat_object->text_alignment = text_alignment;}
void slat_object_set_text_color(SlatObject *slat_object, GColor color) {slat_object->text_color = color;}
void slat_object_set_background_color(SlatObject *slat_object, GColor color) {slat_object->background_color = color;}
void slat_object_set_size(SlatObject *slat_object, GSize size) {slat_object->rect.size = size;}
void slat_object_set_overflow_mode(SlatObject *slat_object, GTextOverflowMode overflow_mode) {slat_object->overflow_mode = overflow_mode;}


void slat_object_render(SlatObject *slat_object, GContext *ctx) {
	GBitmap *screen_bitmap;
	GBitmapFormat screen_bitmap_format;
	uint8_t *screen_bitmap_data;
	uint16_t screen_bitmap_row_size;

	GBitmapFormat text_bitmap_format;
	uint8_t *text_bitmap_data;
	uint16_t text_bitmap_row_size;

	/**/

	// Get buffer data (which also locks the buffer)
	screen_bitmap = graphics_capture_frame_buffer(ctx);
	screen_bitmap_data = gbitmap_get_data(screen_bitmap);
	screen_bitmap_format = gbitmap_get_format(screen_bitmap);
	screen_bitmap_row_size = gbitmap_get_bytes_per_row(screen_bitmap);
	// Unlock the buffer
	graphics_release_frame_buffer(ctx, screen_bitmap);

	// Get time bitmap data
	text_bitmap_data = gbitmap_get_data(slat_object->text_bitmap);
	text_bitmap_format = gbitmap_get_format(slat_object->text_bitmap);
	text_bitmap_row_size = gbitmap_get_bytes_per_row(slat_object->text_bitmap);

	// Point the screen bitmap (aka frame buffer) to the time bitmap (so we can render "offscreen")
	// This trick pulled from 2015 Pebble Developer Retreat presentation on Graphics by Matthew Hungerford
	gbitmap_set_data(screen_bitmap, text_bitmap_data, text_bitmap_format, text_bitmap_row_size, false);

	slat_object_render_text_bitmap(slat_object, ctx);

	// Point the screen bitmap (aka frame buffer) back to it's original data
	gbitmap_set_data(screen_bitmap, screen_bitmap_data, screen_bitmap_format, screen_bitmap_row_size, false);

	slat_object_set_slat_bitmaps(slat_object);
}


void slat_object_animate(SlatObject *slat_object) {
	int slat_counter;
	int animation_counter;
	int delay;
	int start_position_x_offset;

	GRect start_position;
	GRect stop_position;

	/**/
	
	for(slat_counter = 0; slat_counter < slat_object->rect.size.h; slat_counter++) {
		// Move offscreen to hide the slat until the scheduled animation kicks in
		layer_set_frame(bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]), GRect(144, TIME_Y_ORIGIN + slat_counter, slat_object->rect.size.w, 1));

		// Animate slats
		start_position_x_offset = (((slat_counter & 1) == 1) ? 144 : -slat_object->rect.size.w);  // Interleave effect
		start_position = GRect(start_position_x_offset, TIME_Y_ORIGIN + slat_counter, slat_object->rect.size.w, 1);
		stop_position = GRect(TIME_X_ORIGIN, TIME_Y_ORIGIN + slat_counter, slat_object->rect.size.w, 1);
		
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


void layer_add_slat_object(Layer *layer, SlatObject *slat_object) {
	int slat_counter;

	/**/

	for(slat_counter = 0; slat_counter < slat_object->rect.size.h; slat_counter++) {
		layer_add_child(layer, bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]));
	}
}


/**/


// Private internal functions declared at top of this file

void slat_object_render_text_bitmap(SlatObject *slat_object, GContext *ctx) {
	// Set background
	graphics_context_set_fill_color(ctx, slat_object->background_color);
	graphics_fill_rect(ctx, slat_object->rect, 0, GCornerNone);

	// Render text
	graphics_context_set_text_color(ctx, slat_object->text_color);
	graphics_draw_text(ctx, 
											slat_object->text,
											slat_object->font,
											slat_object->rect,
											slat_object->overflow_mode,
											slat_object->text_alignment,
											NULL);
}


void slat_object_set_slat_bitmaps(SlatObject *slat_object) {
	int slat_counter;

	/**/

	for(slat_counter = 0; slat_counter < slat_object->rect.size.h; slat_counter++) {
		// Could use memcpy() here instead of destroy/create ???
		gbitmap_destroy(slat_object->slat_bitmaps[slat_counter]);
		slat_object->slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(slat_object->text_bitmap, GRect(0, slat_counter, slat_object->rect.size.w, 1));
		bitmap_layer_set_bitmap(slat_object->slat_layers[slat_counter], slat_object->slat_bitmaps[slat_counter]);
	}
}


void slat_animation_stopped(Animation *animation, bool finished, void *property_animation) {
	if(finished){property_animation_destroy(property_animation);}
}


