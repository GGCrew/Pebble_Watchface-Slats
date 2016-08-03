#include "slat_object.h"


/**/


#define ANIMATION_DURATION 1000
#define ANIMATION_DELAY 10


/**/


// Declarations for private internal functions

void slat_object_render_text_bitmap(SlatObject *slat_object, GContext *ctx);
void slat_object_set_slat_bitmaps(SlatObject *slat_object);
int slat_object_get_next_slat_index(SlatObject *slat_object);
void slat_object_schedule_exit_animation(SlatObject *slat_object);
void slat_object_schedule_entrance_animation(SlatObject *slat_object);
void slat_animation_stopped(Animation *animation, bool finished, void *property_animation);
void entrance_animation_timer_callback(void *slat_object);

/**/


Animation *trigger_animation;


/**/


// Public functions declared in "slat_object.h"

SlatObject* slat_object_create(GRect rect) {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_create()...");

	int slat_counter;
	int slat_piece_counter;
	GBitmapFormat bitmap_format;
	
	SlatObject *slat_object = malloc(sizeof(SlatObject));

	/**/

	slat_object->rect = rect;
	slat_object->slat_piece_start = slat_object->rect.origin.y;
	slat_object->slat_piece_count = slat_object->rect.size.h;
	slat_object->current_slat_index = 0;
	slat_object->next_slat_index = slat_object_get_next_slat_index(slat_object);

	// Sanity checks
	if (slat_object->slat_piece_start >= MAX_SCREEN_HEIGHT) {
		// Image is entirely offscreen -- nothing to render!
		slat_object->slat_piece_count = -1;
		slat_object->slat_piece_start = 0;
	}
	
	if (slat_object->slat_piece_start < 0) {
		// Image is shifted off top of screen -- Adjust start and slat_count to render visible area.
		// If slat_count is negative, the image is entirely offscreen (and negative value is OK).
		slat_object->slat_piece_count += slat_object->slat_piece_start;
		slat_object->slat_piece_start = 0;
	}

	if (slat_object->slat_piece_count > MAX_SLAT_PIECES) {
		// Image is larger than we can handle.  Reduce slat_count to max capacity and ignore rest of image.
		slat_object->slat_piece_count = MAX_SLAT_PIECES;
	}

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_start: %d", slat_object->slat_piece_start);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_count: %d", slat_object->slat_piece_count);
	
	
	// Create blank text bitmaps
	bitmap_format = GBitmapFormat1Bit;	// for Aplite
	
	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		slat_object->slat[slat_counter].text_bitmap = gbitmap_create_blank(slat_object->rect.size, bitmap_format);

		for(slat_piece_counter = slat_object->slat_piece_start; slat_piece_counter < slat_object->slat_piece_count; slat_piece_counter++) {
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_piece_counter: %d", slat_piece_counter);
			slat_object->slat[slat_counter].layers[slat_piece_counter] = bitmap_layer_create(GRect(slat_object->origin.x, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1));
			slat_object->slat[slat_counter].bitmaps[slat_piece_counter] = gbitmap_create_as_sub_bitmap(slat_object->slat[slat_counter].text_bitmap, GRect(slat_object->rect.origin.x, slat_piece_counter, slat_object->rect.size.w, 1));
			bitmap_layer_set_bitmap(slat_object->slat[slat_counter].layers[slat_piece_counter], slat_object->slat[slat_counter].bitmaps[slat_piece_counter]);
		}
	}

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_create() complete!");
	return slat_object;
}


void slat_object_destroy(SlatObject *slat_object) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_destroy()...");
	int slat_counter;
	int slat_piece_counter;

	/**/

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		for(slat_piece_counter = slat_object->slat_piece_start; slat_piece_counter < slat_object->slat_piece_count; slat_piece_counter++) {
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_piece_counter: %d", slat_piece_counter);
			gbitmap_destroy(slat_object->slat[slat_counter].bitmaps[slat_piece_counter]);
			layer_destroy(bitmap_layer_get_layer(slat_object->slat[slat_counter].layers[slat_piece_counter]));
		}

		//APP_LOG(APP_LOG_LEVEL_DEBUG, "gbitmap_destroy(slat_object->slat[%d].text_bitmap)...", slat_counter);
		gbitmap_destroy(slat_object->slat[slat_counter].text_bitmap);
	}

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "free(slat_object)...");
	free(slat_object);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_destroy() complete!");
}


void slat_object_set_text(SlatObject *slat_object, const char *text) {slat_object->text = text;}
void slat_object_set_font(SlatObject *slat_object, GFont font) {slat_object->font = font;}
void slat_object_set_text_alignment(SlatObject *slat_object, GTextAlignment text_alignment) {slat_object->text_alignment = text_alignment;}
void slat_object_set_text_color(SlatObject *slat_object, GColor color) {slat_object->text_color = color;}
void slat_object_set_background_color(SlatObject *slat_object, GColor color) {slat_object->background_color = color;}
void slat_object_set_size(SlatObject *slat_object, GSize size) {slat_object->rect.size = size;}
void slat_object_set_overflow_mode(SlatObject *slat_object, GTextOverflowMode overflow_mode) {slat_object->overflow_mode = overflow_mode;}
void slat_object_set_origin(SlatObject *slat_object, GPoint point) {slat_object->origin = point;}


void slat_object_render(SlatObject *slat_object, GContext *ctx) {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_render()...");
	GBitmap *screen_bitmap;
	GBitmapFormat screen_bitmap_format;
	uint8_t *screen_bitmap_data;
	uint16_t screen_bitmap_row_size;

	GBitmapFormat text_bitmap_format;
	uint8_t *text_bitmap_data;
	uint16_t text_bitmap_row_size;

	/**/

	if(slat_object->dirty) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object marked dirty -- rendering");

		// Get buffer data (which also locks the buffer)
		screen_bitmap = graphics_capture_frame_buffer(ctx);
		screen_bitmap_data = gbitmap_get_data(screen_bitmap);
		screen_bitmap_format = gbitmap_get_format(screen_bitmap);
		screen_bitmap_row_size = gbitmap_get_bytes_per_row(screen_bitmap);
		// Unlock the buffer
		graphics_release_frame_buffer(ctx, screen_bitmap);

		// Get time bitmap data
		text_bitmap_data = gbitmap_get_data(slat_object->slat[slat_object->next_slat_index].text_bitmap);
		text_bitmap_format = gbitmap_get_format(slat_object->slat[slat_object->next_slat_index].text_bitmap);
		text_bitmap_row_size = gbitmap_get_bytes_per_row(slat_object->slat[slat_object->next_slat_index].text_bitmap);

		// Point the screen bitmap (aka frame buffer) to the time bitmap (so we can render "offscreen")
		// This trick pulled from 2015 Pebble Developer Retreat presentation on Graphics by Matthew Hungerford
		gbitmap_set_data(screen_bitmap, text_bitmap_data, text_bitmap_format, text_bitmap_row_size, false);

		slat_object_render_text_bitmap(slat_object, ctx);

		// Point the screen bitmap (aka frame buffer) back to it's original data
		gbitmap_set_data(screen_bitmap, screen_bitmap_data, screen_bitmap_format, screen_bitmap_row_size, false);

		slat_object_set_slat_bitmaps(slat_object);

		slat_object->current_slat_index = slat_object->next_slat_index;
		slat_object->next_slat_index = slat_object_get_next_slat_index(slat_object);

		slat_object->dirty = false;
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_render() complete!");
}


void slat_object_animate(SlatObject *slat_object) {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_animate()...");
//	int slat_piece_counter;
//	int delay;
//	int x_offset;
//
//	GRect start_position;
//	GRect stop_position;

	/**/

	slat_object_schedule_exit_animation(slat_object);
	
	// Set up trigger for entrance animations
	app_timer_register(2000, entrance_animation_timer_callback, slat_object);


//	for(slat_piece_counter = slat_object->slat_piece_start; slat_piece_counter < slat_object->slat_piece_count; slat_piece_counter++) {
//		// -- Exit animation -- 
//		x_offset = (((slat_piece_counter & 1) == 1) ? slat_object->rect.size.w : -slat_object->rect.size.w);  // Interleave effect
//		//start_position = NULL;
//		stop_position = GRect(slat_object->origin.x, slat_object->origin.y + slat_piece_counter + 50, slat_object->rect.size.w, 1);
//
//		delay = slat_piece_counter * ANIMATION_DELAY;
//		slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter] = property_animation_create_layer_frame(
//			bitmap_layer_get_layer(slat_object->slat[slat_object->current_slat_index].layers[slat_piece_counter]), 
//			NULL, 
//			&stop_position
//		);
//		animation_set_curve(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]), AnimationCurveEaseOut);
//		animation_set_delay(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]), 0 + delay);
//		animation_set_duration(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]), ANIMATION_DURATION);
//		animation_set_handlers(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]), 
//														(AnimationHandlers) {
//															.started = NULL,
//															.stopped = (AnimationStoppedHandler) slat_animation_stopped,
//														},
//														slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]);
//		animation_schedule(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]));
//
//		
//		// -- Entrance animation --
//		// Move offscreen to hide the slat until the scheduled animation kicks in
//		layer_set_frame(bitmap_layer_get_layer(slat_object->slat[slat_object->next_slat_index].layers[slat_piece_counter]), GRect(MAX_SCREEN_WIDTH, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1));
//
//		x_offset = (((slat_piece_counter & 1) == 1) ? slat_object->rect.size.w : -slat_object->rect.size.w);  // Interleave effect
//		start_position = GRect(x_offset, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1);
//		stop_position = GRect(slat_object->origin.x, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1);
//
//		delay = slat_piece_counter * ANIMATION_DELAY;
//		slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter] = property_animation_create_layer_frame(
//			bitmap_layer_get_layer(slat_object->slat[slat_object->next_slat_index].layers[slat_piece_counter]), 
//			&start_position, 
//			&stop_position
//		);
//		animation_set_curve(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]), AnimationCurveEaseOut);
//		animation_set_delay(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]), 0 + delay);
//		animation_set_duration(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]), ANIMATION_DURATION);
//		animation_set_handlers(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]), 
//														(AnimationHandlers) {
//															.started = NULL,
//															.stopped = (AnimationStoppedHandler) slat_animation_stopped,
//														},
//														slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]);
//		animation_schedule(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]));
//	}

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_animate() complete!");
}


void slat_object_schedule_exit_animation(SlatObject *slat_object) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_schedule_exit_animation()...");
	int slat_piece_counter;
	int delay;
	int x_offset;

	GRect start_position;
	GRect stop_position;

	/**/

	delay = ANIMATION_DELAY;

	for(slat_piece_counter = slat_object->slat_piece_start; slat_piece_counter < slat_object->slat_piece_count; slat_piece_counter++) {
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_piece_counter: %d", slat_piece_counter);

		layer_set_frame(bitmap_layer_get_layer(slat_object->slat[slat_object->current_slat_index].layers[slat_piece_counter]), (GRect){{0, -1}, {0, 0}});

		// -- Exit animation -- 
		x_offset = (((slat_piece_counter & 1) == 1) ? slat_object->rect.size.w : -slat_object->rect.size.w);  // Interleave effect
		start_position = GRect(slat_object->origin.x, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1);
		stop_position = GRect(x_offset, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1);

		delay = slat_piece_counter * ANIMATION_DELAY;
		slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter] = property_animation_create_layer_frame(
			bitmap_layer_get_layer(slat_object->slat[slat_object->current_slat_index].layers[slat_piece_counter]), 
			&start_position, 
			&stop_position
		);
		animation_set_curve(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]), AnimationCurveEaseOut);
		animation_set_delay(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]), 0 + delay);
		animation_set_duration(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]), ANIMATION_DURATION);
		animation_set_handlers(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]), 
														(AnimationHandlers) {
															.started = NULL,
															.stopped = (AnimationStoppedHandler) slat_animation_stopped,
														},
														slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]);
		animation_schedule(property_animation_get_animation(slat_object->slat[slat_object->current_slat_index].animations[slat_piece_counter]));
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_piece_counter: complete!");

	APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_schedule_exit_animation() complete!");

}


void slat_object_schedule_entrance_animation(SlatObject *slat_object) {
	int slat_piece_counter;
	int delay;
	int x_offset;

	GRect start_position;
	GRect stop_position;

	/**/

	for(slat_piece_counter = slat_object->slat_piece_start; slat_piece_counter < slat_object->slat_piece_count; slat_piece_counter++) {
		// -- Entrance animation --
		// Move offscreen to hide the slat until the scheduled animation kicks in
		layer_set_frame(bitmap_layer_get_layer(slat_object->slat[slat_object->next_slat_index].layers[slat_piece_counter]), GRect(MAX_SCREEN_WIDTH, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1));

		x_offset = (((slat_piece_counter & 1) == 1) ? slat_object->rect.size.w : -slat_object->rect.size.w);  // Interleave effect
		start_position = GRect(x_offset, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1);
		stop_position = GRect(slat_object->origin.x, slat_object->origin.y + slat_piece_counter, slat_object->rect.size.w, 1);

		delay = slat_piece_counter * ANIMATION_DELAY;
		slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter] = property_animation_create_layer_frame(
			bitmap_layer_get_layer(slat_object->slat[slat_object->next_slat_index].layers[slat_piece_counter]), 
			&start_position, 
			&stop_position
		);
		animation_set_curve(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]), AnimationCurveEaseOut);
		animation_set_delay(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]), 0 + delay);
		animation_set_duration(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]), ANIMATION_DURATION);
		animation_set_handlers(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]), 
														(AnimationHandlers) {
															.started = NULL,
															.stopped = (AnimationStoppedHandler) slat_animation_stopped,
														},
														slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]);
		animation_schedule(property_animation_get_animation(slat_object->slat[slat_object->next_slat_index].animations[slat_piece_counter]));
	}
}


void slat_object_mark_dirty(SlatObject *slat_object) {
	slat_object->dirty = true;
}


void layer_add_slat_object(Layer *layer, SlatObject *slat_object) {
	int slat_counter;
	int slat_piece_counter;

	/**/

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		for(slat_piece_counter = slat_object->slat_piece_start; slat_piece_counter < slat_object->slat_piece_count; slat_piece_counter++) {
			layer_add_child(layer, bitmap_layer_get_layer(slat_object->slat[slat_counter].layers[slat_piece_counter]));
		}
	}
}


/**/


// Private internal functions declared at top of this file

void slat_object_render_text_bitmap(SlatObject *slat_object, GContext *ctx) {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_render_text_bitmap()...");

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
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_render_text_bitmap() complete!");
}


void slat_object_set_slat_bitmaps(SlatObject *slat_object) {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_set_slat_bitmaps()...");

	int slat_piece_counter;

	/**/

	for(slat_piece_counter = slat_object->slat_piece_start; slat_piece_counter < slat_object->slat_piece_count; slat_piece_counter++) {
		// Could use memcpy() here instead of destroy/create ???

		gbitmap_destroy(slat_object->slat[slat_object->next_slat_index].bitmaps[slat_piece_counter]);
		slat_object->slat[slat_object->next_slat_index].bitmaps[slat_piece_counter] = gbitmap_create_as_sub_bitmap(slat_object->slat[slat_object->next_slat_index].text_bitmap, GRect(0, slat_piece_counter, slat_object->rect.size.w, 1));
		bitmap_layer_set_bitmap(slat_object->slat[slat_object->next_slat_index].layers[slat_piece_counter], slat_object->slat[slat_object->next_slat_index].bitmaps[slat_piece_counter]);
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_object_set_slat_bitmaps() complete!");
}


int slat_object_get_next_slat_index(SlatObject *slat_object) {
	return ((slat_object->current_slat_index + 1) % SLAT_COUNT);
}


void slat_animation_stopped(Animation *animation, bool finished, void *property_animation) {
	if(finished){property_animation_destroy(property_animation);}
}


void entrance_animation_timer_callback(void *slat_object) {
	slat_object_schedule_entrance_animation(slat_object);
}

