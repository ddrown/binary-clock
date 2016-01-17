# binary-clock

Depends on the stm32duino environment

Depends on this ST7735 library: https://github.com/KenjutsuGH/Adafruit-ST7735-Library

Hardware:
* STM32F103 - processor
* ST7735 based LCD board

Pins:

* PA5 - LCD SPI CLK
* PA7 - LCD SPI MOSI
* PA4 - LCD CS
* PC13 - LCD RES
* PA3 - LCD RS/DC
* 3.3V - LCD VCC
* GND - LCD GND
* PA10 - UART TX (just needed to set the time)
* PA9 - UART RX (just needed to set the time)

LCD + processor uses around 40mA

screenshot: https://www.youtube.com/watch?v=QE03CFNgCjA
