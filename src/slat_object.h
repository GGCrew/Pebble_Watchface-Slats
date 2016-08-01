#include <pebble.h>


/**/


#define MAX_SLAT_COUNT 64
#define MAX_SCREEN_WIDTH 144
#define MAX_SCREEN_HEIGHT 168


/**/


typedef struct {
	GBitmap *text_bitmap;
	GBitmap *bitmaps[MAX_SLAT_COUNT];
	BitmapLayer *layers[MAX_SLAT_COUNT];
	PropertyAnimation *animations[MAX_SLAT_COUNT];
} Slat;

typedef struct {
	Slat slat[2];
	const char *text;
	GFont font;
	GTextAlignment text_alignment;
	GColor text_color;
	GColor background_color;
	GRect rect;
	GTextOverflowMode overflow_mode;
	int slat_start;
	int slat_count;
	GPoint origin;
} SlatObject;


/**/


SlatObject* slat_object_create(GRect rect);

void slat_object_destroy(SlatObject *slat_object);
void slat_object_set_text(SlatObject *slat_object, const char *text);
void slat_object_set_font(SlatObject *slat_object, GFont font);
void slat_object_set_text_alignment(SlatObject *slat_object, GTextAlignment text_alignment);
void slat_object_set_text_color(SlatObject *slat_object, GColor color);
void slat_object_set_background_color(SlatObject *slat_object, GColor color);
void slat_object_set_overflow_mode(SlatObject *slat_object, GTextOverflowMode overflow_mode);
void slat_object_set_origin(SlatObject *slat_object, GPoint point);
void slat_object_render(SlatObject *slat_object, GContext *ctx);
void slat_object_animate(SlatObject *slat_object);

void layer_add_slat_object(Layer *layer, SlatObject *slat_object);


