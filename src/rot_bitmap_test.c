#include <pebble.h>

static Window *window;
static Layer *rootLayer, *layer;
static GPoint center = { 72, 84 };
static GBitmap *minuteHandBitmap, *hourHandBitmap;
static RotBitmapLayer *minuteHandLayer, *hourHandLayer;

static TextLayer *textLayer;
static char text[10];

#define R_MIN 70
static void updateLayer(Layer *layer, GContext *ctx) {
	int m;
	GPoint p;
	int32_t a, cosa, sina;
	GRect rect;
	
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorWhite);
	
	for (m=0; m<60; m++) {
		a = (m*TRIG_MAX_ANGLE/60 + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE;
		cosa = cos_lookup(a);
		sina = sin_lookup(a);

		p.x = center.x + R_MIN * cosa / TRIG_MAX_RATIO;
		p.y = center.y + R_MIN * sina / TRIG_MAX_RATIO;
		
		if (!(m%15)) {
			switch (m) {
				case 0:
					break;
					
				case 15:
					p.x -= 5;
					p.y -= 2;
					rect.origin = p;
					rect.size.w = 11;
					rect.size.h = 5;
					graphics_fill_rect(ctx, rect, 0, GCornerNone);
					break;
					
				case 30:
					p.x -= 2;
					p.y -= 5;
					rect.origin = p;
					rect.size.w = 5;
					rect.size.h = 11;
					graphics_fill_rect(ctx, rect, 0, GCornerNone);
					break;
					
				case 45:
					p.x -= 6;
					p.y -= 2;
					rect.origin = p;
					rect.size.w = 11;
					rect.size.h = 5;
					graphics_fill_rect(ctx, rect, 0, GCornerNone);
					break;
			}
		} else if (!(m%5)) {
			p.x -= 2;
			p.y -= 2;
			rect.origin = p;
			rect.size.w = rect.size.h = 5;
			graphics_fill_rect(ctx, rect, 0, GCornerNone);
		} else {
			p.x -= 1;
			p.y -= 1;
			rect.origin = p;
			rect.size.w = rect.size.h = 3;
			graphics_fill_rect(ctx, rect, 0, GCornerNone);
		}
	}
}

static void handleTick(struct tm *t, TimeUnits units_changed) {
	GRect r;
	int32_t minuteAngle = t->tm_sec * TRIG_MAX_ANGLE / 60;
	int32_t hourAngle = ((t->tm_min%12)*60 + t->tm_sec) * TRIG_MAX_ANGLE / 720;
	
	snprintf(text, sizeof(text), "%.2d:%.2d", (t->tm_min)%12, t->tm_sec);
	text_layer_set_text(textLayer, text);
	
	r = layer_get_frame((Layer *)minuteHandLayer);
	r.origin.x = 72 - r.size.w/2 + 56 * cos_lookup((minuteAngle + 3 * TRIG_MAX_ANGLE / 4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
	r.origin.y = 84 - r.size.h/2 + 56 * sin_lookup((minuteAngle + 3 * TRIG_MAX_ANGLE / 4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
	layer_set_frame((Layer *)minuteHandLayer, r);
	rot_bitmap_layer_set_angle(minuteHandLayer, minuteAngle);
	
	r = layer_get_frame((Layer *)hourHandLayer);
	r.origin.x = 72 - r.size.w/2 + 57 * cos_lookup((hourAngle + 3 * TRIG_MAX_ANGLE / 4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
	r.origin.y = 84 - r.size.h/2 + 57 * sin_lookup((hourAngle + 3 * TRIG_MAX_ANGLE / 4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
	layer_set_frame((Layer *)hourHandLayer, r);
	rot_bitmap_layer_set_angle(hourHandLayer, hourAngle);
}

static void init(void) {
	time_t t = time(NULL);
	
	window = window_create();
	window_set_background_color(window, GColorBlack);
	window_stack_push(window, true);
	rootLayer = window_get_root_layer(window);

	layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(layer, updateLayer);
	layer_add_child(rootLayer, layer);

	textLayer = text_layer_create(GRect(72-30, 84-10, 60, 20));
	text_layer_set_background_color(textLayer, GColorClear);
	text_layer_set_text_color(textLayer, GColorWhite);
	text_layer_set_text_alignment(textLayer, GTextAlignmentCenter);
	layer_add_child(rootLayer, text_layer_get_layer(textLayer));
	
	minuteHandBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAND_MINUTE);
	minuteHandLayer = rot_bitmap_layer_create(minuteHandBitmap);
	rot_bitmap_set_compositing_mode(minuteHandLayer, GCompOpOr);
	layer_add_child(rootLayer, (Layer *)minuteHandLayer);

	hourHandBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAND_HOUR);
	hourHandLayer = rot_bitmap_layer_create(hourHandBitmap);
	rot_bitmap_set_compositing_mode(hourHandLayer, GCompOpOr);
	layer_add_child(rootLayer, (Layer *)hourHandLayer);
	
	// Init rotations
	handleTick(localtime(&t), SECOND_UNIT|MINUTE_UNIT|HOUR_UNIT);
	
	tick_timer_service_subscribe(SECOND_UNIT, handleTick);
}

static void deinit(void) {
	tick_timer_service_unsubscribe();
	rot_bitmap_layer_destroy(minuteHandLayer);
	gbitmap_destroy(minuteHandBitmap);
	rot_bitmap_layer_destroy(hourHandLayer);
	gbitmap_destroy(hourHandBitmap);
	text_layer_destroy(textLayer);
	layer_destroy(layer);
	window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
