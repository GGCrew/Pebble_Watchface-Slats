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


SlatObject* slat_object_create();

void slat_object_destroy(SlatObject *slat_object);
void slat_object_update_time(SlatObject *slat_object, TextLayer *time_text_layer, GContext *ctx);
void slat_object_animate(SlatObject *slat_object);

void layer_add_slat_object(Layer *layer, SlatObject *slat_object);


