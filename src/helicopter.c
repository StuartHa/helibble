#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static GBitmap *image, *image_tilt;
static Layer* layer;
int y, v;
int y_int, v_int;
uint8_t height[16];
uint8_t ground_bottom[16];
uint8_t obstacle[16];
bool rockets;
bool up = true;
AppTimer* timer;
bool title = true;
bool collision;
bool freeze;
int score;
char score_str[20];
int best_score;
char best_score_str[20];

#define REFRESH_RATE 50
#define MIN_HEIGHT 70

int c = 1;

static void down_click_handler(ClickRecognizerRef rec, void* ctx) {
  if(title) {
    title = false;
    y = 50;
    v = 0;
    y_int = 500;
    v_int = 0;
    c = 1;
   memset(height, 143, 16);
   memset(ground_bottom, 15, 16);
   memset(obstacle, 0, 16);
   if(score > best_score)
     best_score = score;

   score = 0;
  }
  rockets = true;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  rockets = false;
}

void shift_array(uint8_t* p) {
  for(int i = 0; i < 15; i++)
    p[i] = p[i + 1];
}

void update_ground(void) {
  // trending -- if ground is going up, it more likely to go up next round 
  int r = rand() % 3;
  c++;
  if(up && r == 0) {
    up = false;
  }
  else if(!up && r == 0) {
    up = true; 
  }

  shift_array(obstacle);
  obstacle[15] = 0;
  shift_array(ground_bottom);
  if(c % 16 == 0) {
    obstacle[15] = 154 - ground_bottom[15] - height[15] + rand() % (height[15] - 25);
  }

  // height changing
  int rr = rand() % 7;
  if(rr == 0 && height[15] > 75) {
    shift_array(height); 
    height[15]--;
  }

  //ground changing

  if(up) {
    if(ground_bottom[15] < 154 - height[15] - 5)
    ground_bottom[15] += 5;
  }
  else {
    if(ground_bottom[15] > 5)
    ground_bottom[15] -= 5;
  }

  // see if helicopter intersects at block 1
  if( y + 23 >= 154 - ground_bottom[2]) {
    collision = true;
    layer_mark_dirty(layer);
  } 
  else if( y <= 154 - height[2] - ground_bottom[2] + 1) {
    collision = true;
    layer_mark_dirty(layer);
  }
  else if(obstacle[5] != 0 && y <= obstacle[5] + 20 && y >= obstacle[5] - 25) {
    collision = true;
    layer_mark_dirty(layer);
  }
    else if(obstacle[4] != 0 && y <= obstacle[4] + 20 && y >= obstacle[4] - 25) {
    collision = true;
    layer_mark_dirty(layer);
  }
  else if(obstacle[3] != 0 && y <= obstacle[3] + 20 && y >= obstacle[3] - 25) {
    collision = true;
    layer_mark_dirty(layer);
  }
  else if(obstacle[2] != 0 && y <= obstacle[2] + 20 && y >= obstacle[2] - 25) {
    collision = true;
    layer_mark_dirty(layer);
  }
}

void unfreeze_cb(void* g) {
  freeze = false;
  collision = false;
  title = true;
   y = 50;
    v = 0;
    y_int = 500;
    v_int = 0;
    c = 1;
    memset(obstacle, 0, 16);
    layer_mark_dirty(layer);
}

static void layer_update_callback(Layer* me, GContext* ctx) {

  GRect bounds = image->bounds;
  if(title && !freeze) {
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, "Helibble", fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK), (GRect) {.origin = {0,0}, .size = {144, 50}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
    graphics_draw_bitmap_in_rect(ctx, image, (GRect) {.origin = {15, y}, .size = bounds.size});
    graphics_draw_text(ctx, "PRESS UP", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) {.origin = {0,105}, .size = {144, 50}}, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL); 
  } 
  else {
  if(!rockets) {
  graphics_draw_bitmap_in_rect(ctx, image, (GRect) {.origin = {15, y}, .size = bounds.size});
  } else {
    graphics_draw_bitmap_in_rect(ctx, image_tilt, (GRect) {.origin = {15, y}, .size = bounds.size});
  }
  graphics_context_set_fill_color(ctx, GColorWhite);
  for(int i = 0; i < 16; i++) {
    graphics_fill_rect(ctx, (GRect) { .origin = {i * 9, 154 - ground_bottom[i]}, .size = {9, ground_bottom[i]}}, 0, GCornerNone); 
    graphics_fill_rect(ctx, (GRect) { .origin = {i * 9, 0}, .size = {9, 154 - ground_bottom[i] - height[i]}}, 0, GCornerNone);
    if(obstacle[i] != 0) {
      graphics_fill_rect(ctx, (GRect) { .origin = {i * 9, obstacle[i]}, .size = {9, 25}}, 0, GCornerNone); 
    }
  }
  graphics_fill_rect(ctx, (GRect) { .origin = {0, 154}, .size = {144, 14}}, 0, GCornerNone); 
  graphics_context_set_text_color(ctx, GColorBlack);
  snprintf(score_str, 20, "%i", score);
  graphics_draw_text(ctx, score_str, fonts_get_system_font(FONT_KEY_GOTHIC_14), (GRect) {.origin = {0, 154}, .size={144,14}}, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  if(best_score != 0) {
    snprintf(best_score_str, 20, "Best: %i", best_score);
    graphics_draw_text(ctx, best_score_str, fonts_get_system_font(FONT_KEY_GOTHIC_14), (GRect) {.origin = {0, 154}, .size={144,14}}, GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
  }
  if(collision) {
    graphics_draw_circle(ctx, (GPoint) {35, y+10}, 20);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_circle(ctx, (GPoint) {35, y+10}, 19);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_circle(ctx, (GPoint) {35, y+10}, 18);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_circle(ctx, (GPoint) {35, y+10}, 17);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_circle(ctx, (GPoint) {35, y+10}, 16);
    freeze = true;
    app_timer_register(1000, unfreeze_cb, NULL);
  }
  }
}

static void click_config_provider(void *context) {
  window_raw_click_subscribe(BUTTON_ID_UP, down_click_handler, up_click_handler, NULL);
}

int cnt;
void timer_cb(void* data) {
  // apply the gravity
  if(title || freeze || collision) {
    app_timer_register(REFRESH_RATE, timer_cb, NULL);
    return; 
  }
  score += 1;
  int t = 7, a = 1;
  if(rockets) {
    a = -1;
  }
  else {
    a = 1;
  }if(cnt % 3 != 0)
    update_ground();
 if(title || freeze || collision) {
    app_timer_register(REFRESH_RATE, timer_cb, NULL);
    return; 
  }
  y_int += t * t / a + v * t ;  
  y = y_int / 10;
  v_int += (t + 1) / a;
  v = v_int / 10;

    cnt++;

  layer_mark_dirty(layer);
  app_timer_register(REFRESH_RATE, timer_cb, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);
  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HELI);
  image_tilt = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HELI_TILT);
  y = 50;
  memset(height, 143, 16);
  memset(ground_bottom, 15, 16);
  app_timer_register(REFRESH_RATE, timer_cb, NULL);
  srand(time(NULL));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_set_fullscreen(window, true);
  window_stack_push(window, animated);
  light_enable(true);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
