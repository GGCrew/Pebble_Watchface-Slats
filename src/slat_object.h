#include <pebble.h>


/**/


#define SLAT_COUNT 64


/**/


typedef struct {
	GBitmap *text_bitmap;
	GBitmap *slat_bitmaps[SLAT_COUNT];
	BitmapLayer *slat_layers[SLAT_COUNT];
	PropertyAnimation *slat_animations[SLAT_COUNT];
	const char *text;
	GFont font;
	GTextAlignment text_alignment;
	GColor text_color;
	GColor background_color;
	GSize size;
	GTextOverflowMode overflow_mode;
} SlatObject;


/**/


SlatObject* slat_object_create();

void slat_object_destroy(SlatObject *slat_object);
void slat_object_set_text(SlatObject *slat_object, const char *text);
void slat_object_set_font(SlatObject *slat_object, GFont font);
void slat_object_set_text_alignment(SlatObject *slat_object, GTextAlignment text_alignment);
void slat_object_set_text_color(SlatObject *slat_object, GColor color);
void slat_object_set_background_color(SlatObject *slat_object, GColor color);
void slat_object_set_size(SlatObject *slat_object, GSize size);
void slat_object_set_overflow_mode(SlatObject *slat_object, GTextOverflowMode overflow_mode);
void slat_object_render(SlatObject *slat_object, GContext *ctx);
void slat_object_animate(SlatObject *slat_object);

void layer_add_slat_object(Layer *layer, SlatObject *slat_object);


