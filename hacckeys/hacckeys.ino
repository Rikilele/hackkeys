// All includes here
#include <hidboot.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

// Special Keys


/*
 *      CHAR CATCHING BY ARDUINO STARTS HERE
 */

// Classes
class KbdRptParser : public KeyboardReportParser {
  protected:
    void OnKeyDown (uint8_t mod, uint8_t key);
};

// Variables

boolean lCtrlOn;
boolean lShftOn;
boolean lAltOn;
boolean lGUIOn;
boolean rCtrlOn;
boolean rShftOn;
boolean rAltOn;
boolean rGUIOn;
uint8_t charAscii;

// Catch ASCII from keyboard
void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  // introduce a mod, so things work out
  MODIFIERKEYS modif;
  *((uint8_t*)&modif) = m;
  
  lCtrlOn = (modif.bmLeftCtrl  == 1) ? true : false;
  lShftOn = (modif.bmLeftShift == 1) ? true : false;
  lAltOn  = (modif.bmLeftAlt   == 1) ? true : false;
  lGUIOn  = (modif.bmLeftGUI   == 1) ? true : false;

  rCtrlOn = (modif.bmRightCtrl  == 1) ? true : false;
  rShftOn = (modif.bmRightShift == 1) ? true : false;
  rAltOn  = (modif.bmRightAlt   == 1) ? true : false;
  rGUIOn  = (modif.bmRightGUI   == 1) ? true : false;
  
  charAscii = OemToAscii(mod, key);

  // function to go to next step
  
}


/*
 *      CHAR MANIPULATION FROM ARDUINO STARTS HERE
 */

// What gets sent to USB
uint8_t buf[8] = { 0 };

// Defining keyboard mode
#define QWERTY   1
#define DVORAK   2
#define COLEMARK 3

// For now
int state = QWERTY;

// Function to clear input modifiers and key
void releaseKey() {
  buf[0] = 0;
  buf[2] = 0;
  Serial.write(buf, 8);
}


/*
 *     ARDUINO SETUP AND LOOP
 */

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
}

void loop() {

  if (state == QWERTY) {

    // alphabet first
    buf[2] = key;
    

   
   // これ一番最後 
   Serial.write(buf, 8);
   releaseKey();
  }

}
