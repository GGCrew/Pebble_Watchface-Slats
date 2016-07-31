#include "constants.h"
#include "slat_object.h"


/**/


void slat_animation_stopped(Animation *animation, bool finished, void *property_animation);


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
		//layer_add_child(window_layer, bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]));
	}
	
	return slat_object;
}

void slat_object_destroy(SlatObject *slat_object) {
	free(slat_object);
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


void animate_slats(SlatObject *slat_object) {
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


