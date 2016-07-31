#include <pebble.h>
#include "constants.h"
#include "slat_object.h"


/**/


static Window *window;

static TextLayer *text_time_layer;

static SlatObject *slat_object;


/**/


void update_time_bitmap(GContext *ctx) {
	const char *time_text;
	GFont time_font;
	GRect time_rect = {{0, 0}, {144, SLAT_COUNT}};

	uint8_t text_color_ARGB8;
	uint8_t background_color_ARGB8;

	/**/

	// Get text data
	time_text = text_layer_get_text(text_time_layer);
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


void text_time_layer_update_proc(Layer *layer, GContext *ctx)
{
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

	// TODO: Move to slat_object.c
	// Get time bitmap data
	time_bitmap_data = gbitmap_get_data(slat_object->time_bitmap);
	time_bitmap_format = gbitmap_get_format(slat_object->time_bitmap);
	time_bitmap_row_size = gbitmap_get_bytes_per_row(slat_object->time_bitmap);

	// Point the screen bitmap (aka frame buffer) to the time bitmap (so we can render "offscreen")
	// This trick pulled from 2015 Pebble Developer Retreat presentation on Graphics by Matthew Hungerford
	gbitmap_set_data(screen_bitmap, time_bitmap_data, time_bitmap_format, time_bitmap_row_size, false);

	update_time_bitmap(ctx);

	// Point the screen bitmap (aka frame buffer) back to it's original data
	gbitmap_set_data(screen_bitmap, screen_bitmap_data, screen_bitmap_format, screen_bitmap_row_size, false);

	update_slat_bitmaps(slat_object);
}


void update_display_time(struct tm *tick_time) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  char *time_format;

	/**/

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


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_display_time(tick_time);
	animate_slats(slat_object);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	/**/

	text_time_layer = text_layer_create(bounds);
	text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_time_layer, GColorClear);
	layer_set_update_proc(text_layer_get_layer(text_time_layer), text_time_layer_update_proc);
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

	slat_object = slat_object_create();
	layer_add_slat_object(window_layer, slat_object);
}


static void window_unload(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "window_unload...");
	int slat_counter;

	/**/

	APP_LOG(APP_LOG_LEVEL_DEBUG, "text_layer_destroy(text_time_layer)...");
	text_layer_destroy(text_time_layer);

	for(slat_counter = 0; slat_counter < SLAT_COUNT; slat_counter++) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "slat_counter: %d", slat_counter);
		gbitmap_destroy(slat_object->slat_bitmaps[slat_counter]);
		layer_destroy(bitmap_layer_get_layer(slat_object->slat_layers[slat_counter]));
	}

	APP_LOG(APP_LOG_LEVEL_DEBUG, "gbitmap_destroy(time_bitmap)...");
	gbitmap_destroy(slat_object->time_bitmap);

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
