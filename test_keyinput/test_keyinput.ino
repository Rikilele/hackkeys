#include <hidboot.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

uint8_t buf[8] = { 0 };

/*
 * class necessary from the beginning 
 */
class KbdRptParser : public KeyboardReportParser
{
  protected:
    void OnKeyDown	(uint8_t mod, uint8_t key);
    void OnKeyUp  (uint8_t mod, uint8_t key);
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key){
  /*buf[0] = mod;
  buf[2] = key;
  Serial.write(buf, 8);*/
  buf[0] = mod;
  if(buf[2] == 0){ buf[2] = key; }
  else if(buf[3] == 0){ buf[3] = key; }
  else if(buf[4] == 0){ buf[4] = key; }
  else if(buf[5] == 0){ buf[5] = key; }
  else if(buf[6] == 0){ buf[6] = key; }
  else if(buf[7] == 0){ buf[7] = key; }
  Serial.write(buf, 8);
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key){
  /*buf[0] = 0;
  buf[2] = 0;
  Serial.write(buf, 8);*/
  buf[0] = 0;
  if(buf[2] == key){ buf[2] = 0; }
  else if(buf[3] == key){ buf[3] = 0; }
  else if(buf[4] == key){ buf[4] = 0; }
  else if(buf[5] == key){ buf[5] = 0; }
  else if(buf[6] == key){ buf[6] = 0; }
  else if(buf[7] == key){ buf[7] = 0; }
  Serial.write(buf, 8);
}


USB                                   Usb;
USBHub                                Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);
uint32_t                              next_time;
KbdRptParser                          Prs;

void setup() {
  Serial.begin(9600);

#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif

  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay( 200 );

  next_time = millis() + 5000;

  HidKeyboard.SetReportParser(0, &Prs);

  //pinMode(7, INPUT);
  //digitalWrite(7, 1);

  delay(200);

}

/*void releaseKey(){
  buf[0] = 0;
  buf[2] = 0;
  Serial.write(buf, 8);
  }*/

void loop() {
  Usb.Task();
  /*int state = digitalRead(7);
  if(state != 1){
    buf[2] = 4;
    Serial.write(buf, 8);
    releaseKey();
    }*/

}
