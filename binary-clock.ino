#include <ESP8266WiFi.h>

// TODO: is a special version needed anymore?
#include <Adafruit_GFX.h>    // Core graphics library
// the ESP8266 version of the ST7735 library: https://github.com/ddrown/Adafruit-ST7735-Library
#include <Adafruit_ST7735.h> // Hardware-specific library
// millisecond precision clock library: https://github.com/ddrown/Arduino_Clock
#include <Clock.h>
// Timezone library ported to ESP8266: https://github.com/ddrown/Timezone
#include <Timezone.h>
// ESP8266 NTP client with millisecond precision: https://github.com/ddrown/Arduino_NTPClient
#include <NTPClient.h>

#include "settings.h" // see this file to change the settings

#include "icons/off.h"
#include "icons/on_i.h"
#include "icons/blueon.h"
#include "icons/redon.h"
#include "icons/birthday.h"
#include "icons/haloween.h"
#include "icons/independance_day.h"
#include "icons/thanksgiving.h"
#include "icons/valentines.h"
#include "icons/candy_cane.h"
#include "icons/christmas_tree.h"
#include "icons/santa.h"
#include "icons/firework.h"

struct holiday_icon {
  const uint16_t *icon;
  uint8_t month;
  uint8_t day;
  boolean enabled;
};

// rules like "the fourth thursday" are encoded as WEEKDAY_RULE + DAY
// where DAY is the day of the month if the first day was Sunday
#define WEEKDAY_RULE 40
#define FIRST_SUNDAY WEEKDAY_RULE+1
#define FIRST_MONDAY WEEKDAY_RULE+2
#define FIRST_TUESDAY WEEKDAY_RULE+3
#define FIRST_WEDNESDAY WEEKDAY_RULE+4
#define FIRST_THURSDAY WEEKDAY_RULE+5
#define FIRST_FRIDAY WEEKDAY_RULE+6
#define FIRST_SATURDAY WEEKDAY_RULE+7
#define SECOND_THURSDAY WEEKDAY_RULE+5+7
#define THIRD_THURSDAY WEEKDAY_RULE+5+14
#define FOURTH_THURSDAY WEEKDAY_RULE+5+21
#define FIFTH_SUNDAY WEEKDAY_RULE+1+28

// TODO: settings to turn these off/on and configure day
struct holiday_icon icons[] = {
  { .icon = on_i, .month = 0, .day = 0, .enabled = true},
  { .icon = blueon, .month = 0, .day = 0, .enabled = false},
  { .icon = redon, .month = 0, .day = 0, .enabled = false},
  { .icon = firework, .month = 1, .day = 1, .enabled = true},
  { .icon = valentines, .month = 2, .day = 14, .enabled = true},
  { .icon = birthday, .month = 9, .day = 28, .enabled = true},
  { .icon = independance_day, .month = 7, .day = 4, .enabled = true},
  { .icon = haloween, .month = 10, .day = 31, .enabled = true},
  { .icon = thanksgiving, .month = 11, .day = FOURTH_THURSDAY, .enabled = true},
  { .icon = christmas_tree, .month = 12, .day = 24, .enabled = true},
  { .icon = santa, .month = 12, .day = 25, .enabled = true},
  { .icon = candy_cane, .month = 12, .day = 25, .enabled = false},
};
uint8_t icons_index = 0;

// local port to listen for UDP packets, TODO: randomize and switch
#define LOCAL_NTP_PORT 2390
IPAddress timeServerIP;
NTPClient ntp;

// full screen+text: ~72ms, full screen: ~11ms, tens seconds: ~2ms, ones seconds: ~1ms
#define SPI1_CLK   D5
#define SPI1_MISO  D6
#define SPI1_MOSI  D7
#define TFT_CS     D8
#define TFT_RST    D2
#define TFT_DC     D1
#define SERIALPORT Serial

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

void tft_setup() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(0);
  tft.setTextSize(1);
  tft.setTextWrap(false);
  tft.setTextColor(ST7735_WHITE);  
}

void wifi_setup() {
  tft.println("Connecting to SSID");
  tft.println(ssid);
  SERIALPORT.println("Connecting to SSID");
  SERIALPORT.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
    SERIALPORT.print(".");
  }
  tft.println();
  SERIALPORT.println();

  // TODO: show wifi strength
  tft.println("WiFi connected");
  SERIALPORT.println("WiFi connected");
  tft.println("IP address: ");
  SERIALPORT.println("IP address: ");
  tft.println(WiFi.localIP());
  SERIALPORT.println(WiFi.localIP());
}

void ntp_setup() {
  tft.println("Starting NTP/UDP");
  SERIALPORT.println("Starting NTP/UDP");
  ntp.begin(LOCAL_NTP_PORT);

  WiFi.hostByName(ntpServerName, timeServerIP); // TODO: lookup failures and re-lookup DNS pools

  tft.print("NTP IP: ");
  SERIALPORT.print("NTP IP: ");
  tft.println(timeServerIP.toString());
  SERIALPORT.println(timeServerIP.toString());
}

void setup() {
  tft_setup();

  SERIALPORT.begin(115200);
  SERIALPORT.println();
  SERIALPORT.println();

  tft.setTextWrap(true);
  tft.setCursor(0, 0);
  wifi_setup();
  ntp_setup();

  tft.println("Delayed start");
  SERIALPORT.println("Delayed start");
  for(uint8_t i = 0; i < 5; i++) {
    tft.print(".");
    SERIALPORT.print(".");
    delay(1000);
  }
  SERIALPORT.println();
  tft.fillScreen(ST7735_BLACK);
  tft.setTextWrap(false);
}

void refresh_text(const char *tzname) {
  tft.setCursor(0, 68);
  tft.print("YEAR   MONTH    DAY");
  tft.setCursor(0, 150);
  tft.print("HOUR   MINUTE  SECOND");
}

void draw_bits(uint16_t value, uint16_t maxten, uint8_t x, uint8_t y) {
  for(int8_t i = maxten; i >= 0; i--) {
    if(value & (1 << i)) {
      tft.PixelArray(x, y + (3-i)*16, 16, 16, icons[icons_index].icon);
    } else {
      tft.PixelArray(x, y + (3-i)*16, 16, 16, off);
    }
  }
}

void tens(uint16_t value, uint16_t maxten, uint8_t x, uint8_t y) {
  value = (value / 10) % 10;
  draw_bits(value, maxten, x, y);
}

void ones(uint16_t value, uint8_t x, uint8_t y) {
  value = value % 10;
  draw_bits(value, 3, x, y);
}

void set_icons_index(time_t now) {
  for(uint8_t i = 0; i < sizeof(icons)/sizeof(icons[0]); i++) {
    if(icons[i].enabled && icons[i].month == month(now)) {
      if(icons[i].day > WEEKDAY_RULE) { // happens on a set day of the week
        uint8_t find_day = icons[i].day - WEEKDAY_RULE;  // find_day is a month starting on a sunday
        uint8_t want_wday = (find_day - 1) % 7; // sunday = 0, saturday = 6
        uint8_t want_week = (find_day - 1) / 7;
        uint8_t this_week = (day(now)-1) / 7;
        if(this_week == want_week && want_wday == weekday(now)-1) {
          icons_index = i;
          return;
        }
      } else if(icons[i].day == day(now)) { // happens on a set day
        icons_index = i;
        return;
      }
    }
  }
}

void time_print(time_t now, const char *tzname) {
  static uint8_t runonce = 1;
  if(runonce) {
    tft.fillScreen(ST7735_BLACK);
    refresh_text(tzname); // TODO: refresh the tzname?
  }
  if(runonce || second(now) == 0) { // reprint everything at the top of the minute
    runonce = 0;

    set_icons_index(now);
    
    // update seconds first because they're the most critical
    ones(second(now), 106, 82);   
    tens(second(now), 2, 88, 82);
    ones(minute(now), 62, 82);
    tens(minute(now), 2, 44, 82);
    ones(hour(now), 18, 82); // TODO: 12/24 hour modes
    tens(hour(now), 1, 0, 82);

    tens(day(now), 1, 88, 0);
    ones(day(now), 106, 0);
    tens(month(now), 0, 44, 0);
    ones(month(now), 62, 0);
    tens(year(now), 3, 0, 0);
    ones(year(now), 18, 0);     
  } else {
    if(second(now) % 10) {
      // only the ones digit in seconds changed
    } else { 
      // both the ones and tens digit changed
      tens(second(now), 2, 88, 82);
    }
    ones(second(now), 106, 82);
  }
  //SERIALPORT.println(second(now));
}

struct timems startTS;
void ntp_loop(bool ActuallySetTime) {
  PollStatus NTPstatus;
  
  ntp.sendNTPpacket(timeServerIP);

  while((NTPstatus = ntp.poll_reply(ActuallySetTime)) == NTP_NO_PACKET) { // wait until we get a response
    delay(1); // allow the background threads to run
  }
  
  if (NTPstatus == NTP_TIMEOUT) {
    SERIALPORT.println("NTP client timeout");
  } else if(NTPstatus == NTP_PACKET) {
    struct timems nowTS;
    uint32_t ms_delta;
    int32_t ppm_error;
    now_ms(&nowTS);
    
    int32_t offset = ntp.getLastOffset();
    uint32_t rtt = ntp.getLastRTT();
    SERIALPORT.print("now=");
    SERIALPORT.print(nowTS.tv_sec);
    SERIALPORT.print(" rtt=");
    SERIALPORT.print(rtt);

    ms_delta = (nowTS.tv_sec - startTS.tv_sec) * 1000 + (nowTS.tv_msec - startTS.tv_msec);
    if(ms_delta == 0) {
      ms_delta = 1;
    }
    ppm_error = offset * 1000000 / ms_delta;
    SERIALPORT.print(" drift=");
    SERIALPORT.print(offset);
    SERIALPORT.print(" ppm=");
    SERIALPORT.print(ppm_error);
    SERIALPORT.print(" delta=");
    SERIALPORT.println(ms_delta);
  }
}

void loop() {
  TimeChangeRule *tcr;
  time_t now_t = 1, last_t = 0, local, next_ntp;

  ntp_loop(true); // set time, TODO: what if this fails?
  next_ntp = now() + NTP_INTERVAL;
  now_ms(&startTS);
  adjustClockSpeed(21, -1); // TODO: do this properly.  sets to 47.619ppm slow
  
  while(1) {
    //uint32_t startms, endms;
    do {
      delay(5); // TODO: sleep properly instead of spinning fast
      now_t = now();
    } while(now_t == last_t); // loop until the clock changes
    last_t = now_t;
    //startms = millis();
    // TODO: configure timezone
    local = TIMEZONE.toLocal(now_t, &tcr);
    time_print(local, tcr->abbrev);
    /*endms = millis();
    SERIALPORT.print("LCD took ");
    SERIALPORT.println(endms-startms); */

    if((now_t > next_ntp) && ((second(local) % 10) != 0)) { // repoll on seconds not ending in 0
      ntp_loop(false);
      next_ntp = now_t + NTP_INTERVAL;
    }
  }
}
