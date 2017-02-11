// All includes here
#include <hidboot.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

// Special Keys
#define LAYER_KEY   0
#define PROGRAM_KEY 1
#define RESET_KEY   2

/*
 *     DB FUNCTIONS
 */

boolean addMacro();
uint16_t[] readMacro(uint8_t mod, uint8_t key);
void deleteMacro(uint8_t mod, uint8_t key);
void deleteAllMacros();

/*
 *      CHAR CATCHING BY ARDUINO STARTS HERE
 */

// Classes
class KbdRptParser : public KeyboardReportParser {
  protected:
    void OnKeyDown (uint8_t mod, uint8_t key);
};

// Variables
boolean PROGRAMMABLE;     // true is PROGRAMMABLE false is NOT PROGRAMMABLE
boolean RESET;            // reset flag is set when reset is pressed once
boolean lCtrlOn;
boolean lShftOn;
boolean lAltOn;
boolean lGUIOn;
boolean rCtrlOn;
boolean rShftOn;
boolean rAltOn;
boolean rGUIOn;
uint8_t CHARASCII;
uint8_t MODIFIERS;
uint8_t PRESSEDKEY;

// Catch ASCII from keyboard
// This is probably an interrupt ---- 注意！
void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  // introduce a mod, so things work out
  MODIFIERKEYS modif;
  *((uint8_t*)&modif) = mod;
  
  lCtrlOn = (modif.bmLeftCtrl  == 1) ? true : false;
  lShftOn = (modif.bmLeftShift == 1) ? true : false;
  lAltOn  = (modif.bmLeftAlt   == 1) ? true : false;
  lGUIOn  = (modif.bmLeftGUI   == 1) ? true : false;

  rCtrlOn = (modif.bmRightCtrl  == 1) ? true : false;
  rShftOn = (modif.bmRightShift == 1) ? true : false;
  rAltOn  = (modif.bmRightAlt   == 1) ? true : false;
  rGUIOn  = (modif.bmRightGUI   == 1) ? true : false;
  
  CHARASCII = OemToAscii(mod, key);
  MODIFIERS = mod;
  PRESSEDKEY = key;
}


/*
 *      CHAR MANIPULATION FROM ARDUINO STARTS HERE
 */

// What gets sent to USB
uint8_t buf[8] = { 0 };

//// Defining keyboard mode
//#define QWERTY   1
//#define DVORAK   2
//#define COLEMARK 3
//
//// For now
//int state = QWERTY;

// Function to clear input modifiers and key
void resetBuf() {
  buf[0] = 0;
  buf[2] = 0;
  Serial.write(buf, 8);
}

void checkThenManageBuf () {
  uint16_t[] moves = readMacro(MODIFIERS, PRESSEDKEY); // doesn't compile
  manageBuf(); // if needed
  return;
}

void manageBuf () {
  // manipulate buf
  return;
}

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

  // set pins
  pinMode(PROGRAM_KEY, INPUT);
  digitalWrite(PROGRAM_KEY, 1);

  delay( 200 ); //いるかどうかわからん
}

void loop() {
  
  // need this for reading from keyboard
  Usb.Task();

  PROGRAMMABLE = digitalRead(LAYER_KEY)? true : false;

  if (PROGRAMMABLE) {
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
      // manipulate buf
      checkThenManageBuf();
      Serial.write(buf, 8);
      resetBuf();
    }
  
  } else {
    // not programmable
    manageBuf();
    Serial.write(buf, 8);
    resetBuf();
  }
}
