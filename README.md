# binary-clock

Depends on the esp8266 Arduino environment

Depends on these libraries:
[the ESP8266 version of the ST7735 library](https://github.com/ddrown/Adafruit-ST7735-Library)
[millisecond precision clock library](https://github.com/ddrown/Arduino_Clock)
[Timezone library ported to ESP8266](https://github.com/ddrown/Timezone)
[ESP8266 NTP client with millisecond precision](https://github.com/ddrown/Arduino_NTPClient)

Hardware:
* ESP8266 - processor
* ST7735 based LCD board

Pins (NodeMCU names):

* D5 - LCD SPI CLK
* D7 - LCD SPI MOSI
* D8 - LCD CS
* D2 - LCD RES
* D1 - LCD RS/DC
* 3.3V - LCD VCC
* GND - LCD GND

LCD + processor uses around 100mA on startup, ~25mA while running

[pictures/videos](https://blog.dan.drown.org/binary-clock/)
