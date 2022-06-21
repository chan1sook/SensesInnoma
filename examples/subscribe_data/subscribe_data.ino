/* 
  Example - Subscribe Data from Innoma Platform
  Created by Natthawat Raocharoensinp, June 20, 2022.
*/
#include <SensesInnoma.h>

const char userid[] PROGMEM = "your-userid";
const char key[] PROGMEM = "your-deviceid";
const char wifissid[] PROGMEM = "your-wifi-ssid";
const char wifipw[] PROGMEM = "your-wifi-password";

InnomaClient innomaProtocol(userid, key);

void dataCallback(uint8_t slot, double value);
void controlCallback(uint8_t slot, uint8_t state);

void setup() {
  Serial.begin(115200);
  
  innomaProtocol.begin(wifissid, wifipw);
  innomaProtocol.waitUntilReady();
  Serial.println(F("Connected"));
  
  innomaProtocol.setDataCallback(dataCallback);
  innomaProtocol.setControlCallback(controlCallback);

  innomaProtocol.subscribeData(1);
  innomaProtocol.subscribeControl(1);
}

void loop() {
  if(!innomaProtocol.ready()) {
    Serial.println(F("Disconnected"));

    innomaProtocol.begin(wifissid, wifipw);
    innomaProtocol.waitUntilReady();

    Serial.println(F("Connected"));
    
    innomaProtocol.subscribeData(1);
    innomaProtocol.subscribeControl(1);
  }
  innomaProtocol.loop();

  Serial.print(F("Free Memory: "));
  Serial.println(ESP.getFreeHeap());
  delay(2000);
}


void dataCallback(uint8_t slot, double value) {
  Serial.print(F("Data: "));
  Serial.print(slot);
  Serial.print(' ');
  Serial.println(value);
}

void controlCallback(uint8_t slot, uint8_t state) {
  Serial.print(F("Control: "));
  Serial.print(state);
  Serial.print(' ');
  Serial.println((state == HIGH) ? F("HIGH") : F("LOW"));
}