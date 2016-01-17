// the STM32F103 version of the Adafruit_GFX library is included in stm32duino
#include <Adafruit_GFX_AS.h>    // Core graphics library
// the STM32F103 version of the ST7735 library: https://github.com/KenjutsuGH/Adafruit-ST7735-Library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <RTClock.h>

// ST7735 based LCD setup using SPI1, pins below
// PA5 - SPI1 CLK
// PA7 - SPI1 MOSI
#define TFT_CS     PA4
#define TFT_RST    PC13
#define TFT_DC     PA3
// other LCD pins: GND and 3.3V

// Serial1 - PA10=RX PA9=TX (when Serial USB support is enabled, otherwise use the object "Serial")
#define SERIALPORT Serial1

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
RTClock rt (RTCSEL_LSE); // use the Low Speed External oscillator/pins PC14/PC15 - not on every board

void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(0);
  tft.setTextSize(1);
  tft.setTextWrap(false);
  tft.setTextColor(ST7735_WHITE);
  SERIALPORT.begin(9600);

  if(rt.getTime() < 1452739058) { // if the RTC hasn't been setup yet, ask for the current time
    tft.setCursor(0, 0);
    tft.print("Set Time on serial");
    
    SERIALPORT.println("Enter time in unix seconds"); 
  
    uint32_t now = 0;
    char input;
    do {
      input = SERIALPORT.read();
      if(input >= '0' && input <= '9') {
        now = now * 10 + (input - '0');
      }
    } while(input != '\n');
    SERIALPORT.print("time was ");
    SERIALPORT.println(rt.getTime());
    SERIALPORT.print("time is now ");
    SERIALPORT.println(now);
    rt.setTime(now);
  }
}

void refresh_text() {
  tft.setCursor(0, 68);
  tft.print("YEAR   MONTH    DAY");
  tft.setCursor(0, 150);
  tft.print("HOUR   MINUTE  SECOND");
}

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

void time_print(struct tm *now) {
  static uint8_t runonce = 1;
  if(runonce) { // first time through, clear the screen and print the text
    tft.fillScreen(ST7735_BLACK);
    refresh_text();
  }
  if(runonce || now->tm_sec == 0) { // reprint everything at the top of the minute
    runonce = 0;

    tens(1900+now->tm_year, 3, 8, 8);
    ones(1900+now->tm_year, 26, 8);
    tens(1+now->tm_mon, 0, 48, 8);
    ones(1+now->tm_mon, 66, 8);
    tens(now->tm_mday, 1, 96, 8);
    ones(now->tm_mday, 114, 8);
     
    tens(now->tm_hour, 1, 8, 90);
    ones(now->tm_hour, 26, 90);
    tens(now->tm_min, 2, 48, 90);
    ones(now->tm_min, 66, 90);
    tens(now->tm_sec, 2, 96, 90);
    ones(now->tm_sec, 114, 90);   
  } else {
    if(now->tm_sec % 10) {
      // only the ones digit in seconds changed
    } else { 
      // both the ones and tens digit changed
      tens(now->tm_sec, 2, 96, 90);
    }
    ones(now->tm_sec, 114, 90);
  }
  SERIALPORT.println(now->tm_sec);
}

void loop() {
  struct tm *now = NULL;
  time_t now_t = 1, last_t = 0;
  uint32_t start;
  
  while(1) {
    do {
      now_t = rt.getTime();
    } while(now_t == last_t); // loop until the clock changes
    start = millis();
    last_t = now_t;
    now = gmtime(&now_t); // TODO: timezone
    time_print(now);
    SERIALPORT.print("took ");
    SERIALPORT.print(millis()-start);
    SERIALPORT.println(" ms");
  }
}
