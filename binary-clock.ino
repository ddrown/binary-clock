#include <ESP8266WiFi.h>

// TODO: is a special version needed anymore?
#include <Adafruit_GFX_AS.h>    // Core graphics library
// the ESP8266 version of the ST7735 library: https://github.com/ddrown/Adafruit-ST7735-Library
#include <Adafruit_ST7735.h> // Hardware-specific library
// millisecond precision clock library: https://github.com/ddrown/Arduino_Clock
#include <Clock.h>
// Timezone library ported to ESP8266: https://github.com/ddrown/Timezone
#include <Timezone.h>
// ESP8266 NTP client with millisecond precision: https://github.com/ddrown/Arduino_NTPClient
#include <NTPClient.h>

#include "settings.h" // see this file to change the settings

// local port to listen for UDP packets, TODO: randomize and switch
#define LOCAL_NTP_PORT 2390
IPAddress timeServerIP;
NTPClient ntp;

// full screen: ~300-400ms, tens seconds: ~52ms, ones seconds: ~30ms
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
  for(uint8_t i = 0; i < 10; i++) {
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

// TODO: use bitmaps?
void draw_bits(uint16_t value, uint16_t maxten, uint8_t x, uint8_t y) {
  for(int8_t i = maxten; i >= 0; i--) {
    tft.drawCircle(x, y + (3-i)*16, 8, ST7735_WHITE);
    if(value & (1 << i)) {
      tft.fillCircle(x, y + (3-i)*16, 6, ST7735_GREEN);
    } else {
      tft.fillCircle(x, y + (3-i)*16, 6, ST7735_BLACK);
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

void time_print(time_t now, const char *tzname) {
  static uint8_t runonce = 1;
  if(runonce) {
    tft.fillScreen(ST7735_BLACK);
    refresh_text(tzname); // TODO: refresh the tzname?
  }
  if(runonce || second(now) == 0) { // reprint everything at the top of the minute
    runonce = 0;
    
    // update seconds first because otherwise they are updated ~300ms late
    ones(second(now), 114, 90);   
    tens(second(now), 2, 96, 90);
    ones(minute(now), 70, 90);
    tens(minute(now), 2, 52, 90);
    ones(hour(now), 26, 90);
    tens(hour(now), 1, 8, 90);

    tens(day(now), 1, 96, 8);
    ones(day(now), 114, 8);
    tens(month(now), 0, 52, 8);
    ones(month(now), 70, 8);
    tens(year(now), 3, 8, 8);
    ones(year(now), 26, 8);     
  } else {
    if(second(now) % 10) {
      // only the ones digit in seconds changed
    } else { 
      // both the ones and tens digit changed
      tens(second(now), 2, 96, 90);
    }
    ones(second(now), 114, 90);
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
    do {
      delay(5); // TODO: sleep properly instead of spinning fast
      now_t = now();
    } while(now_t == last_t); // loop until the clock changes
    last_t = now_t;
    // TODO: configure timezone
    local = TIMEZONE.toLocal(now_t, &tcr);
    time_print(local, tcr->abbrev);

    if((now_t > next_ntp) && ((second(local) % 10) != 0)) { // repoll on seconds not ending in 0
      ntp_loop(false);
      next_ntp = now_t + NTP_INTERVAL;
    }
  }
}
