#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0xD2, 0x2B, 0xAF, 0x20, 0x39, 0xCA, 0x4F, 0x37, 0xB9, 0xF1, 0x1E, 0xDE, 0x03, 0x64, 0xE3, 0x22 }
PBL_APP_INFO(MY_UUID, "SimpleWeek", "hhurz", 2, 0 /* App version */, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;

TextLayer text_day_layer;
TextLayer text_week_layer;
TextLayer text_date_layer;
TextLayer text_time_layer;

Layer line_layer;

static const char *day_names[] = {
  "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"
};

static const char *month_names[] = {
  "Januar", "Februar", "März", "April", "Mai", "Juni",
  "Juli", "August", "September", "Oktober", "November", "Dezember"
};

void line_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  graphics_context_set_stroke_color(ctx, GColorWhite);

  graphics_draw_line(ctx, GPoint(8, 97), GPoint(131, 97));
  graphics_draw_line(ctx, GPoint(8, 98), GPoint(131, 98));

}

void update_display(PblTm *tick_time) {
  // Need to be static because they're used by the system later.
  static char time_text[]     = "00:00";
  static char day_text[]      = "                  ";
  static char week_text[]     = "KW00";
  static char date_text[]     = "                  ";
  static char new_date_text[] = "                  ";

  int month = tick_time->tm_mon;
  string_format_time(new_date_text, sizeof(date_text), "%e", tick_time);
  strcat(new_date_text, ". ");
  strcat(new_date_text, month_names[month]);

  // Only update the date/day/week strings when they're changed.
  if (strncmp(new_date_text, date_text, sizeof(date_text)) != 0) {
    string_format_time(week_text, sizeof(week_text), "KW%V", tick_time);
    text_layer_set_text(&text_week_layer, week_text);

    int dayofweek = tick_time->tm_wday;
    strcpy(day_text, day_names[dayofweek]);
    text_layer_set_text(&text_day_layer, day_text);

    strncpy(date_text, new_date_text, sizeof(date_text));
    text_layer_set_text(&text_date_layer, date_text);
  }

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }
  string_format_time(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(&text_time_layer, time_text);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "SimpleWeek");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);

  // week
  text_layer_init(&text_week_layer, window.layer.frame);
  text_layer_set_text_color(&text_week_layer, GColorWhite);
  text_layer_set_background_color(&text_week_layer, GColorClear);
  layer_set_frame(&text_week_layer.layer, GRect(90, 8, 144-90, 168-8));
  text_layer_set_font(&text_week_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(&window.layer, &text_week_layer.layer);

  // weekday
  text_layer_init(&text_day_layer, window.layer.frame);
  text_layer_set_text_color(&text_day_layer, GColorWhite);
  text_layer_set_background_color(&text_day_layer, GColorClear);
  layer_set_frame(&text_day_layer.layer, GRect(8, 44, 144-8, 168-44));
  text_layer_set_font(&text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(&window.layer, &text_day_layer.layer);

  // day/month
  text_layer_init(&text_date_layer, window.layer.frame);
  text_layer_set_text_color(&text_date_layer, GColorWhite);
  text_layer_set_background_color(&text_date_layer, GColorClear);
  layer_set_frame(&text_date_layer.layer, GRect(8, 68, 144-8, 168-68));
  text_layer_set_font(&text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_21)));
  layer_add_child(&window.layer, &text_date_layer.layer);

  // time
  text_layer_init(&text_time_layer, window.layer.frame);
  text_layer_set_text_color(&text_time_layer, GColorWhite);
  text_layer_set_background_color(&text_time_layer, GColorClear);
  layer_set_frame(&text_time_layer.layer, GRect(7, 92, 144-7, 168-92));
  text_layer_set_font(&text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49)));
  layer_add_child(&window.layer, &text_time_layer.layer);

  // line
  layer_init(&line_layer, window.layer.frame);
  line_layer.update_proc = &line_layer_update_callback;
  layer_add_child(&window.layer, &line_layer);

  // Avoid blank display on launch
  PblTm tick_time;
  get_time(&tick_time);
  update_display(&tick_time);
}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  update_display(t->tick_time);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
