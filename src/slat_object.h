#include <pebble.h>


/**/


#define MAX_SLAT_PIECES 64
#define MAX_SCREEN_WIDTH 144
#define MAX_SCREEN_HEIGHT 168
#define SLAT_COUNT 2


/**/


typedef struct {
	GBitmap *text_bitmap;
	GBitmap *bitmaps[MAX_SLAT_PIECES];
	BitmapLayer *layers[MAX_SLAT_PIECES];
	PropertyAnimation *animations[MAX_SLAT_PIECES];
} Slat;

typedef struct {
	Slat slat[SLAT_COUNT];
	const char *text;
	GFont font;
	GTextAlignment text_alignment;
	GColor text_color;
	GColor background_color;
	GPoint origin;
	GRect rect;
	GTextOverflowMode overflow_mode;
	int slat_piece_start;
	int slat_piece_count;
	int current_slat_index;
	int next_slat_index;
	bool dirty;
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
void slat_object_mark_dirty(SlatObject *slat_object);

void layer_add_slat_object(Layer *layer, SlatObject *slat_object);


