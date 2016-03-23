# Usage: Install Arduino-Makefile and if necessary, update the 'include' path below.
# See https://github.com/sudar/Arduino-Makefile
# Install the libraries listed in ARDUINO_LIBS.

BOARD_TAG    = uno
MONITOR_PORT  = /dev/ttyUSB0
ARDUINO_LIBS = LCD_Display9696 Grove_Temperature_And_Humidity_Sensor Wire

include /usr/share/arduino/Arduino.mk
