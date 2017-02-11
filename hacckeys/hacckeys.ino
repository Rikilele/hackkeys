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

// GLOBAL Variables
boolean PROGRAMMABLE_LAYER;                   // true is PROGRAMMABLE, false is NOT PROGRAMMABLE
boolean PROGRAM_MODE_COMM     = false;
boolean PROGRAM_MODE_MACRO    = false;
boolean RESET                 = false;        // reset flag is set when reset is pressed once

uint8_t prog_comm[7];
uint8_t prog_macro[20][7];
int addr = 0;

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
  // First get length of macro
  uint8_t length = 1; // number of 7-byte blocks
  boolean cont = true;
  while(cont){
    if((prog_macro[length-1][0] | prog_macro[length-1][1] | prog_macro[length-1][2] | prog_macro[length-1][3] | prog_macro[length-1][4] | prog_macro[length-1][5] | prog_macro[length-1][6]) == 0){
      cont = false;
    }
    else { length += 1; }
  }
  // Check if there's enough ROM to store the macro
  if((EEPROM.length() - addr) < (length*7)){ return false; }
  // If there is, store the macro in ROM
  else{
    EEPROM.put(addr, length);
    addr += 1;
    EEPROM.put(addr, prog_comm);
    addr += 7;
    EEPROM.put(addr, prog_macro);
    addr += (1 + (length * 7));
    // Also remember to increment number of macro counter
    uint8_t num_macros = EEPROM.read(0);
    EEPROM.write(0, num_macros+1);
    return true;
  }
}

boolean execMacro(){
  uint8_t num_macros = EEPROM.read(0);
  uint8_t len_macro;
  uint8_t buffer[7];
  int m_addr = 1;
  boolean equal = false;
  // Loop to check each macro in ROM
  for( int i = 0; i < num_macros; i++ ){
    len_macro = EEPROM.read(m_addr);
    m_addr += 1;
    EEPROM.get(m_addr, buffer);
    // Check if the command equals this one in ROM
    for(int j = 0; j < 7; j++){
      if(buffer[j] == prog_comm[j]){ equal = true; }
      else{ equal = false; j = 7;}
    }
    // If it is equal, read the next len_macro-1 7-byte blocks and send the keycodes
    // And exit this function with a success return status
    if(equal){
      m_addr += 7;
      for(int k = 0; k < (len_macro - 1); k++){
	EEPROM.get(m_addr, buffer);
	m_addr += 7;
	buf[0] = buffer[0];
	buf[1] = 0x00;
	buf[2] = buffer[1];
	buf[3] = buffer[2];
	buf[4] = buffer[3];
	buf[5] = buffer[4];
	buf[6] = buffer[5];
	buf[7] = buffer[6];
	Serial.Write(buf, 8)
      }
      return true;
    }
    // If not equal, skip ahead to next entry in ROM
    m_addr += 7 * (len_macro - 1);
  }
  // All macros in ROM checked, none match the command, so return false
  return false;
}

boolean deleteMacro(){
  uint8_t num_macros = EEPROM.read(0);
  uint8_t len_macro;
  uint8_t buffer[7];
  int m_addr = 1;
  boolean equal = false;
  // Loop to check each macro in ROM
  for( int i = 0; i < num_macros; i++ ){
    len_macro = EEPROM.read(m_addr);
    m_addr += 1;
    EEPROM.get(m_addr, buffer);
    // Check if the command equals this one in ROM
    for(int j = 0; j < 7; j++){
      if(buffer[j] == prog_comm[j]){ equal = true; }
      else{ equal = false; j = 7;}
    }
    // If it is equal, read the next len_macro-1 7-byte blocks and send the keycodes
    // And exit this function with a success return status
    if(equal){
    }
      return true;
    }
    // If not equal, skip ahead to next entry in ROM
    m_addr += 7 * (len_macro - 1);
  }
  // All macros in ROM checked, none match the command, so return false
  return false;  
}
void deleteAllMacros();
void loadMacros();


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
    // turn on LED
    // write command to temp global
    PROGRAM_MODE_COMM  = false;
    PROGRAM_MODE_MACRO = true;
  }
  else if (PROGRAM_MODE_MACRO) {
    // flash LED
    // send key events to temp global
    // this function should reset the flag as well
  }
  else {
    // check for comm/macro combination
    // then,
    Serial.write(buf, 8);
  }
  
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {
  // TODO: Macro mode should be including key up
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

  uint64_t num;
  boolean cont = true;
  while(cont && addr < EEPROM.length()){
    EEPROM.get(addr, num);
    if(num == 0xffffffffffffffff){ cont = false; }
    else{ addr += 1; }
  }

  delay( 200 );
}

void loop() {
  
  // need this for reading from keyboard
  Usb.Task();

  PROGRAMMABLE_LAYER = digitalRead(LAYER_SWITCH) ? true : false; // TODO check voltage

  if (PROGRAMMABLE_LAYER) {
    
    if (digitalRead(RESET_KEY) != 1) {     // read reset key first
      if (RESET) {
        deleteAllMacros();                 // if second time pressing reset, reset all programs
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
}
