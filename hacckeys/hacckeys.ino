// All includes here
#include <hidboot.h>
#include <usbhub.h>
#include <EEPROM.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

// Special Keys - pins
#define LAYER_SWITCH   14
#define PROGRAM_KEY    6
#define RESET_KEY      5

// GLOBAL Variables
boolean PROGRAMMABLE_LAYER;                   // true is PROGRAMMABLE, false is NOT PROGRAMMABLE
boolean PROGRAM_MODE_COMM     = false;
boolean PROGRAM_MODE_MACRO    = false;
boolean RESET                 = false;        // reset flag is set when reset is pressed once
// LED
#define LED_PROG       6

/*
 *     DB FUNCTIONS
 */

uint8_t prog_comm[7];
uint8_t prog_macro[12][7];
uint8_t prog_macro_pos = 0;

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
 *     DB FUNCTIONS
 */

boolean addMacro(){
  Serial.println("Adding macro...");
  // Load flags
  uint16_t macro_slots;
  EEPROM.get(0, macro_slots);
  // Go through each flag
  for(int i = 0; i < 10; i++){
    // If not taken (flag = 0)
    if( !( (macro_slots >> i) & 0x1 ) ){
      // Get address of first 7-byte block (for command)
      int addr = 2 + (i*7*12);
      // Write command
      EEPROM.put(addr, prog_comm);
      // Loop through macro array and write each one
      for(int j = 0; j < 12; j++){
	addr += 7;
	EEPROM.put(addr, prog_macro[j]);
      }
      // Write success
      Serial.println("Success!");
      return true;
    }
    // If taken, loop again
  }
  // Here, everything is taken, so return false
  Serial.println("Failure!");
  return false;  
}

boolean execMacro(){
  Serial.println("Executing macro...");
  bool equal = false;
  int addr = 0;
  uint8_t buffer[7];
  // Loop for each macro
  for(int i = 0; i < 10; i++){
    addr = 2 + (i*7*12);
    EEPROM.get(addr, buffer);
    // Loop to check command equality
    for(int j = 0; j < 7; j++){
      if(buffer[j] == prog_comm[j]){ equal = true; }
      else{ equal = false; j = 7;}
    }
    // If it is equal, read and execute all the next 7-byte blocks until null terminator
    if(equal){
      addr += 7;
      for(int k = 0; k < 12; k++){
	EEPROM.get(addr, buffer);
	addr += 7;
	buf[0] = buffer[0];
	buf[1] = 0x00;
	buf[2] = buffer[1];
	buf[3] = buffer[2];
	buf[4] = buffer[3];
	buf[5] = buffer[4];
	buf[6] = buffer[5];
	buf[7] = buffer[6];
	Serial.write(buf, 8);
	if(buf[0] == buf[1] == buf[2] == buf[3] == buf[4] == buf[5] == buf[6] == buf[7] == 0){ k = 12; }
      }
      Serial.println("Success!");
      return true;
    }
    // If not equal, loop again
  }
  // No matching command, return false
  Serial.println("Failure!");
  return false;
}

boolean deleteMacro(){
  Serial.println("Deleting single macro...");
  bool equal = false;
  int addr = 0;
  uint8_t buffer[7];
  // Loop for each macro
  for(int i = 0; i < 10; i++){
    addr = 2 + (i*7*12);
    EEPROM.get(addr, buffer);
    // Loop to check command equality
    for(int j = 0; j < 7; j++){
      if(buffer[j] == prog_comm[j]){ equal = true; }
      else{ equal = false; j = 7;}
    }
    // If it is equal, set the corresponding flag to zero (open)
    if(equal){
      // Load flags
      uint16_t macro_slots;
      EEPROM.get(0, macro_slots);
      macro_slots &= ~(0x1 << i);
      EEPROM.put(0, macro_slots);
      Serial.println("Success!");
      return true;
    }
    // If not equal, loop again
  }
  // No matching command, return false
  Serial.println("Failure!");
  return false;  
}
void deleteAllMacros(){
  Serial.println("Deleting all macros");
  uint16_t macro_slots;
  EEPROM.get(0, macro_slots);
  macro_slots &= 0x0;
  EEPROM.put(0, macro_slots);
}


/*
 *      KEYBOARD EVENTS HANDLED HERE IN DETAIL
 */

// Classes
class KbdRptParser : public KeyboardReportParser {
  protected:
    void OnKeyDown (uint8_t mod, uint8_t key);
    void OnKeyUp  (uint8_t mod, uint8_t key);
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  RESET = false;
  buf[0] = mod;
  if(buf[2] == 0){ buf[2] = key; }
  else if(buf[3] == 0){ buf[3] = key; }
  else if(buf[4] == 0){ buf[4] = key; }
  else if(buf[5] == 0){ buf[5] = key; }
  else if(buf[6] == 0){ buf[6] = key; }
  else if(buf[7] == 0){ buf[7] = key; }
  
  if(!PROGRAMMABLE_LAYER){
    Serial.write(buf, 8);
  }
  else if (PROGRAM_MODE_COMM) {
    if (digitalRead(PROGRAM_KEY) != 1) {
      prog_comm[0] = buf[0];
      prog_comm[1] = buf[2];
      prog_comm[2] = buf[3];
      prog_comm[3] = buf[4];
      prog_comm[4] = buf[5];
      prog_comm[5] = buf[6];
      prog_comm[6] = buf[7];
      PROGRAM_MODE_COMM  = false;
      PROGRAM_MODE_MACRO = true;
    }
  }
  else if (PROGRAM_MODE_MACRO) {
    prog_macro[prog_macro_pos][0] = buf[0];
    prog_macro[prog_macro_pos][1] = buf[2];
    prog_macro[prog_macro_pos][2] = buf[3];
    prog_macro[prog_macro_pos][3] = buf[4];
    prog_macro[prog_macro_pos][4] = buf[5];
    prog_macro[prog_macro_pos][5] = buf[6];
    prog_macro[prog_macro_pos][6] = buf[7];
    prog_macro_pos += 1;
    //PROGRAM_MODE_MACRO = false;
  }
  else {
    prog_comm[0] = buf[0];
    prog_comm[1] = buf[2];
    prog_comm[2] = buf[3];
    prog_comm[3] = buf[4];
    prog_comm[4] = buf[5];
    prog_comm[5] = buf[6];
    prog_comm[6] = buf[7];
    if(!execMacro())
      Serial.write(buf, 8);
  }
  
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {
  buf[0] = 0;
  if(buf[2] == key){ buf[2] = 0; }
  else if(buf[3] == key){ buf[3] = 0; }
  else if(buf[4] == key){ buf[4] = 0; }
  else if(buf[5] == key){ buf[5] = 0; }
  else if(buf[6] == key){ buf[6] = 0; }
  else if(buf[7] == key){ buf[7] = 0; }

  if(!PROGRAMMABLE_LAYER){
    Serial.write(buf, 8);
  }
  else if (PROGRAM_MODE_MACRO) {
    prog_macro[prog_macro_pos][0] = buf[0];
    prog_macro[prog_macro_pos][1] = buf[2];
    prog_macro[prog_macro_pos][2] = buf[3];
    prog_macro[prog_macro_pos][3] = buf[4];
    prog_macro[prog_macro_pos][4] = buf[5];
    prog_macro[prog_macro_pos][5] = buf[6];
    prog_macro[prog_macro_pos][6] = buf[7];
    prog_macro_pos += 1;
    //PROGRAM_MODE_MACRO = false;
  }
  else {
    Serial.write(buf, 8);
  }

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
 *     LED SETUP : Thanks to built in example sketch
 */

int             ledState        = HIGH;             // ledState used to set the LED
unsigned long   previousMillis  = 0;               // will store last time LED was updated
const long      interval        = 1000;            // interval at which to blink (milliseconds)


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
  pinMode(LED_PROG, OUTPUT);

  Serial.println("Done starting");
  delay( 200 );
}

void loop() {
  
  // need this for reading from keyboard
  Usb.Task();

  PROGRAMMABLE_LAYER = digitalRead(LAYER_SWITCH) ? true : false; // TODO check voltage
  
  if (PROGRAMMABLE_LAYER) {
    
    if (digitalRead(RESET_KEY) != 1) {     // read reset key first
      Serial.println("reset key pressed!");
      if (RESET) {
        deleteAllMacros();                 // if second time pressing reset, reset all programs
	RESET = false;
      }
      else {
        RESET = true;                      // set up reset flag
      }
    }
    else if (digitalRead(PROGRAM_KEY) != 1) {
      RESET = false;
      PROGRAM_MODE_COMM = true;
    }
  }

  if(digitalRead(RESET_KEY) && PROGRAM_MODE_MACRO){
    PROGRAM_MODE_MACRO = false;
    deleteMacro();
  }
  
  if(digitalRead(PROGRAM_KEY) && PROGRAM_MODE_MACRO) {
    PROGRAM_MODE_MACRO = false;
    addMacro();
  }
  
  if(prog_macro_pos == 11){
    PROGRAM_MODE_MACRO = false;
    addMacro();
  }
  
  /*
  *     LED task
  */

  if (PROGRAM_MODE_COMM) {
    ledState = HIGH;
  }
  else if (PROGRAM_MODE_MACRO) {
    // check to see if it's time to blink the LED; that is, if the
    // difference between the current time and last time you blinked
    // the LED is bigger than the interval at which you want to
    // blink the LED.
    unsigned long currentMillis = millis();
    
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      } 
    }
  } else {
    ledState = HIGH;
  }
  // set the LED with the ledState of the variable:
  digitalWrite(LED_PROG, ledState);




  Serial.println(PROGRAMMABLE_LAYER);
  Serial.println(PROGRAM_MODE_COMM);
  Serial.println(PROGRAM_MODE_MACRO);
  Serial.println(RESET);
}
