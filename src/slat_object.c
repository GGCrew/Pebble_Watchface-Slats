#include <pebble.h>


/**/


#define SLAT_COUNT 64


/**/


typedef struct {
	GBitmap *time_bitmap;
	GBitmap *slat_bitmaps[SLAT_COUNT];
	BitmapLayer *slat_layers[SLAT_COUNT];
	PropertyAnimation *slat_animations[SLAT_COUNT];
} SlatObject;


/**/


static SlatObject slat_object;


/**/


static void update_slat_bitmaps() {
	int slat_counter;

	/**/

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		// Could use memcpy() here instead of destroy/create ???
		gbitmap_destroy(slat_object.slat_bitmaps[slat_counter]);
		slat_object.slat_bitmaps[slat_counter] = gbitmap_create_as_sub_bitmap(slat_object.time_bitmap, GRect(0, slat_counter, 144, 1));
		bitmap_layer_set_bitmap(slat_object.slat_layers[slat_counter], slat_object.slat_bitmaps[slat_counter]);
	}
}


