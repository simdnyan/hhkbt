#include <avr/pgmspace.h>

#include <avrpins.h>
#include <max3421e.h>
#include <usbhost.h>
#include <usb_ch9.h>
#include <Usb.h>
#include <usbhub.h>
#include <avr/pgmspace.h>
#include <address.h>
#include <hidboot.h>

#include <printhex.h>
#include <message.h>
#include <hexdump.h>
#include <parsetools.h>

uint8_t modifier = 0;
uint8_t pressed_keys[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

class KbdRptParser : public KeyboardReportParser{
  void PrintKey(uint8_t mod, uint8_t key);
  void AddKey(uint8_t mod, uint8_t key);
  void DeleteKey(uint8_t mod, uint8_t key);

protected:
  virtual void OnControlKeysChanged(uint8_t before, uint8_t after);
  virtual void OnKeyDown(uint8_t mod, uint8_t key);
  virtual void OnKeyUp(uint8_t mod, uint8_t key);
};

void KbdRptParser::AddKey(uint8_t m, uint8_t key){
  for (int i = 0; i < 6; i++){
    if (pressed_keys[i] == 0x00 || pressed_keys[i] == key){
      pressed_keys[i] = key;
      return;
    }
  }
}

void KbdRptParser::DeleteKey(uint8_t m, uint8_t key){
  for (int i = 0; i < 6; i++){
    if (pressed_keys[i] == key){
      pressed_keys[i] = 0x00;
      for (int j = i; j < 5; j++){
        pressed_keys[j] = pressed_keys[j + 1];
      }
      pressed_keys[5] = 0x00;
      return;
    }
  }
}
void KbdRptParser::PrintKey(uint8_t m, uint8_t key){
    uint8_t buff[11];
    buff[0] = 0xfd;
    buff[1] = 0x09;
    buff[2] = 0x01;
    buff[3] = modifier;
    buff[4] = 0x00;
    for (int i = 5; i < 11; i++){
      buff[i] = pressed_keys[i - 5];
    }

    Serial.write(buff, sizeof(buff));
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key){
    AddKey(mod, key);
    PrintKey(mod, key);
}
void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key){
    DeleteKey(mod, key);
    PrintKey(mod, key);
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {
  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  int modifier_keys[8] = {
    (afterMod.bmLeftCtrl   == 1),
    (afterMod.bmLeftShift  == 1),
    (afterMod.bmLeftAlt    == 1),
    (afterMod.bmLeftGUI    == 1),
    (afterMod.bmRightCtrl  == 1),
    (afterMod.bmRightShift == 1),
    (afterMod.bmRightAlt   == 1),
    (afterMod.bmRightGUI   == 1)
  };

  modifier = 0;
  for (int i = 0; i < 8; i++){
    if (modifier_keys[i]){
      modifier += 1 << i;
    }
  }

  PrintKey(after, 0x00);
}

USB    Usb;
USBHub Hub(&Usb);
HIDBoot<HID_PROTOCOL_KEYBOARD> HidKeyboard(&Usb);

uint32_t next_time;

KbdRptParser Prs;

void setup(){
  Serial.begin( 115200 );

  delay( 200 );

  next_time = millis() + 5000;

  HidKeyboard.SetReportParser(0, (HIDReportParser*)&Prs);
}

void loop(){
  Usb.Task();
}


