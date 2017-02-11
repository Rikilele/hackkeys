#!/bin/sh
sudo dfu-programmer atmega16u2 erase
sudo dfu-programmer atmega16u2 flash ./arduinoMacroKeyboard/Arduino-keyboard-0.3.hex
sudo dfu-programmer atmega16u2 reset
