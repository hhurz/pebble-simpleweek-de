#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "my_math.h"
#include "suncalc.h"

//NOTE: longitude is positive for East and negative for West
#define LATITUDE  49.013524
#define LONGITUDE 8.404455
#define TIMEZONE +1

#define MY_UUID { 0xD2, 0x2B, 0xAF, 0x20, 0x39, 0xCA, 0x4F, 0x37, 0xB9, 0xF1, 0x1E, 0xDE, 0x03, 0x64, 0xE3, 0x22 }
PBL_APP_INFO(MY_UUID, "SimpleWeek", "hhurz", 3, 0 /* App version */, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);

Window window;

TextLayer text_sunrise_layer;
TextLayer text_sunset_layer;
TextLayer text_day_layer;
TextLayer text_week_layer;
TextLayer text_date_layer;
TextLayer text_time_layer;

Layer line_layer;

static const char *day_names[] =
{
  "Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"
};

static const char *month_names[] =
{
  "Januar", "Februar", "März", "April", "Mai", "Juni",
  "Juli", "August", "September", "Oktober", "November", "Dezember"
};

void adjustTimezone(float* time)
{
  *time += TIMEZONE;
  if (*time > 24) *time -= 24;
  if (*time < 0) *time += 24;
}

void InitTextLayer (TextLayer *pLayer, int x, int y, int FontId)
{
  text_layer_init ( pLayer, window.layer.frame );
  text_layer_set_text_color ( pLayer, GColorWhite );
  text_layer_set_background_color ( pLayer, GColorClear );
  layer_set_frame ( &pLayer->layer, GRect(x, y, 144-x, 168-y) );
  text_layer_set_font ( pLayer, fonts_load_custom_font(resource_get_handle(FontId)) );
  layer_add_child ( &window.layer, &pLayer->layer );
}

void line_layer_update_callback(Layer *me, GContext* ctx)
{
  (void)me;

  graphics_context_set_stroke_color(ctx, GColorWhite);

  graphics_draw_line(ctx, GPoint(8, 105), GPoint(131, 105));
  graphics_draw_line(ctx, GPoint(8, 106), GPoint(131, 106));
}

void update_display(PblTm *tick_time)
{
  // Need to be static because they're used by the system later.
  static char sunrise_text[]  = "00:00";
  static char sunset_text[]   = "00:00";
  static char time_text[]     = "00:00";
  static char day_text[]      = "                  ";
  static char week_text[]     = "KW00";
  static char date_text[]     = "                  ";
  static char new_date_text[] = "                  ";

  char *time_format = (clock_is_24h_style()) ? "%R" : "%I:%M";

  int month = tick_time->tm_mon;

  string_format_time(new_date_text, sizeof(date_text), "%e", tick_time);
  strcat(new_date_text, ". ");
  strcat(new_date_text, month_names[month]);

  // Only update the date/day/week strings when they're changed.
  if (strncmp(new_date_text, date_text, sizeof(date_text)) != 0)
  {
    float sunriseTime = calcSunRise(tick_time->tm_year, tick_time->tm_mon+1, tick_time->tm_mday, LATITUDE, LONGITUDE, 91.0f);
    float sunsetTime = calcSunSet(tick_time->tm_year, tick_time->tm_mon+1, tick_time->tm_mday, LATITUDE, LONGITUDE, 91.0f);

    adjustTimezone(&sunriseTime);
    adjustTimezone(&sunsetTime);

    if ( ! tick_time->tm_isdst )
    {
      sunriseTime += 1;
      sunsetTime += 1;
    }

    tick_time->tm_min = (int)(60*(sunriseTime-((int)(sunriseTime))));
    tick_time->tm_hour = (int)sunriseTime;
    string_format_time(sunrise_text, sizeof(sunrise_text), time_format, tick_time);
    text_layer_set_text(&text_sunrise_layer, sunrise_text);

    tick_time->tm_min = (int)(60*(sunsetTime-((int)(sunsetTime))));
    tick_time->tm_hour = (int)sunsetTime;
    string_format_time(sunset_text, sizeof(sunset_text), time_format, tick_time);
    text_layer_set_text(&text_sunset_layer, sunset_text);

    string_format_time(week_text, sizeof(week_text), "KW%V", tick_time);
    text_layer_set_text(&text_week_layer, week_text);

    int dayofweek = tick_time->tm_wday;
    strcpy(day_text, day_names[dayofweek]);
    text_layer_set_text(&text_day_layer, day_text);

    strncpy(date_text, new_date_text, sizeof(date_text));
    text_layer_set_text(&text_date_layer, date_text);
  }

  string_format_time(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0'))
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);

  text_layer_set_text(&text_time_layer, time_text);
}

void handle_init (AppContextRef ctx)
{
  (void)ctx;

  window_init(&window, "SimpleWeek");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);

  // sunrise
  InitTextLayer (&text_sunrise_layer, 8, 8, RESOURCE_ID_FONT_ROBOTO_CONDENSED_16);

  // sunset
  InitTextLayer (&text_sunset_layer, 8, 22, RESOURCE_ID_FONT_ROBOTO_CONDENSED_16);

  // week
  InitTextLayer (&text_week_layer, 94, 8, RESOURCE_ID_FONT_ROBOTO_CONDENSED_16);

  // weekday
  InitTextLayer (&text_day_layer, 8, 52, RESOURCE_ID_FONT_ROBOTO_CONDENSED_21);

  // day/month
  InitTextLayer (&text_date_layer, 8, 76, RESOURCE_ID_FONT_ROBOTO_CONDENSED_21);

  // time
  InitTextLayer (&text_time_layer, 7, 100, RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_49);

  // line
  layer_init(&line_layer, window.layer.frame);
  line_layer.update_proc = &line_layer_update_callback;
  layer_add_child(&window.layer, &line_layer);

  // Avoid blank display on launch
  PblTm tick_time;
  get_time(&tick_time);
  update_display(&tick_time);
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t)
{
  (void)ctx;
  PblTm tick_time;
  memcpy ( &tick_time, t->tick_time, sizeof(PblTm) );
  update_display(&tick_time);
}

void pbl_main(void *params)
{
  PebbleAppHandlers handlers =
  {
    .init_handler = &handle_init,
    .tick_info =
    {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
