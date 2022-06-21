/* 
  Example - Control Device from Innoma Platform
  Created by Natthawat Raocharoensinp, June 20, 2022.
*/
#include <SensesInnoma.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const char userid[] PROGMEM = "your-userid";
const char key[] PROGMEM = "your-deviceid";
const char wifissid[] PROGMEM = "your-wifi-ssid";
const char wifipw[] PROGMEM = "your-wifi-password";

InnomaClient innomaProtocol(userid, key);

void controlCallback(uint8_t slot, uint8_t state);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  innomaProtocol.begin(wifissid, wifipw);
  innomaProtocol.waitUntilReady();
  Serial.println(F("Connected"));
  
  innomaProtocol.setControlCallback(controlCallback);
  innomaProtocol.subscribeControl(1);

  uint8_t state = innomaProtocol.getControl(1);
  digitalWrite(LED_BUILTIN, state);
}

void loop() {
  if(!innomaProtocol.ready()) {
    Serial.println(F("Disconnected"));

    innomaProtocol.begin(wifissid, wifipw);
    innomaProtocol.waitUntilReady();

    Serial.println(F("Connected"));
    
    innomaProtocol.subscribeControl(1);
  }
  innomaProtocol.loop();
}

void controlCallback(uint8_t slot, uint8_t state) {
  Serial.print(F("Control: "));
  Serial.print(LED_BUILTIN);
  Serial.print(' ');
  Serial.print(state);
  Serial.print(' ');
  Serial.println((state == HIGH) ? F("HIGH") : F("LOW"));

  digitalWrite(LED_BUILTIN, state);
}