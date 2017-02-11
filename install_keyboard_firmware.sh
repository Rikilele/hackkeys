#!/bin/sh
echo "Installing Keyboard Firmware"
echo "Pleace reset pins on Arduino (press enter when done)"
read
sudo dfu-programmer atmega16u2 erase
sudo dfu-programmer atmega16u2 flash ./arduinoMacroKeyboard/Arduino-keyboard-0.3.hex && sudo dfu-programmer atmega16u2 reset
