#!/bin/sh
echo "Installing Arduino Firmware"
echo "Pleace reset pins on Arduino (press enter when done)"
read
sudo dfu-programmer atmega16u2 erase
sudo dfu-programmer atmega16u2 flash ./arduinoMacroKeyboard/Arduino-usbserial-uno.hex && sudo dfu-programmer atmega16u2 reset
