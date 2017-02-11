// All includes here
#include <hidboot.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

// Special Keys - pins
#define LAYER_SWITCH   0
#define PROGRAM_KEY    1
#define RESET_KEY      2

/*
 *     DB FUNCTIONS
 */

boolean addMacro(uint8_t mod, uint8_t key, uint16_t *modkey_array){};
uint16_t* readMacro(uint8_t mod, uint8_t key);
void deleteMacro(uint8_t mod, uint8_t key);
void deleteAllMacros(void);

uint8_t buf[8] = { 0 };
/*
 * From: https://learn.adafruit.com/introducing-bluefruit-ez-key-diy-bluetooth-hid-keyboard/sending-keys-via-serial
 *
 * buf[0] = modifer flags
 * buf[1] = 0x00
 * buf[2] = [keycode1]
 * buf[3] = [keycode2]
 * buf[4] = [keycode3]
 * buf[5] = [keycode4]
 * buf[6] = [keycode5]
 * buf[7] = [keycode6]
 */

/*
 *      KEYBOARD EVENTS HANDLED HERE IN DETAIL
 */

// Classes
class KbdRptParser : public KeyboardReportParser {
  protected:
    void OnKeyDown (uint8_t mod, uint8_t key);
    void OnKeyUp  (uint8_t mod, uint8_t key);
};

// GLOBAL Variables
boolean PROGRAMMABLE_LAYER;     // true is PROGRAMMABLE false is NOT PROGRAMMABLE
boolean RESET;            // reset flag is set when reset is pressed once

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  if(!PROGRAMMABLE_LAYER){
    buf[0] = mod;
    if(buf[2] == 0){ buf[2] = key; }
    else if(buf[3] == 0){ buf[3] = key; }
    else if(buf[4] == 0){ buf[4] = key; }
    else if(buf[5] == 0){ buf[5] = key; }
    else if(buf[6] == 0){ buf[6] = key; }
    else if(buf[7] == 0){ buf[7] = key; }
    Serial.write(buf, 8);
  }
  // TODO case PROGRAMMABLE
  // if in PROGRAM_MODE, then don't Serial.write
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key){
  buf[0] = 0;
  if(buf[2] == key){ buf[2] = 0; }
  else if(buf[3] == key){ buf[3] = 0; }
  else if(buf[4] == key){ buf[4] = 0; }
  else if(buf[5] == key){ buf[5] = 0; }
  else if(buf[6] == key){ buf[6] = 0; }
  else if(buf[7] == key){ buf[7] = 0; }
  Serial.write(buf, 8);
}


/*
 *      PHASES FOR PROGRAMMABLE MODE HERE
 */

// // Defining keyboard mode
// #define QWERTY   1
// #define DVORAK   2
// #define COLEMARK 3
//
// // For now
// int state = QWERTY;

/*
 *     Function for Programmable mode (Step by Step)
 */

void setHackkey() {
  
  // Turn LED ON
  while (digitalRead(PROGRAM_KEY) == 1);        // loop until program key is pressed
  // read current input here
  delay(1000);                                  // just so things don't move too fast

  // Turn LED to FLASHING
  while (digitalRead(PROGRAM_KEY) == 1) {
   // take in all events into an array
  };
  return;
}

/*
 *     ARDUINO SETUP AND LOOP
 */

// necessary to read from keyboard
USB                                   Usb;
USBHub                                Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);
uint32_t                              next_time;
KbdRptParser                          Prs;

void setup()
{
  Serial.begin( 9600 );
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");
  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");
  delay( 200 );
  next_time = millis() + 5000;
  HidKeyboard.SetReportParser(0, &Prs);

  pinMode(PROGRAM_KEY, INPUT);
  digitalWrite(PROGRAM_KEY, 1);

  delay( 200 );
}

void loop() {
  
  // need this for reading from keyboard
  Usb.Task();

  PROGRAMMABLE_LAYER = digitalRead(LAYER_SWITCH) ? true : false; // TODO check voltage

  if (PROGRAMMABLE_LAYER) {
    if (digitalRead(RESET_KEY) != 1) {
      if (RESET) {
        deleteAllMacros();
      }
      else {
        RESET = true;
      }
    }
    else if (digitalRead(PROGRAM_KEY) != 1) {
      RESET = false;
      setHackkey();
    } 
    else {
      RESET = false;
    }
  }
  // If it's not programmable then it will just send the key presses straight through
  // So the event listener for the keyboard will deal with that
  /* else {
    // not programmable
    manageBuf();
    Serial.write(buf, 8);
    resetBuf();
    }*/
}
