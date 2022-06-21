/* 
  Example - Publish Data to Innoma Platform
  Created by Natthawat Raocharoensinp, June 20, 2022.
*/
#include <SensesInnoma.h>

const char userid[] PROGMEM = "your-userid";
const char key[] PROGMEM = "your-deviceid";
const char wifissid[] PROGMEM = "your-wifi-ssid";
const char wifipw[] PROGMEM = "your-wifi-password";

InnomaClient innomaProtocol(userid, key);

void setup() {
  Serial.begin(115200);
  
  innomaProtocol.begin(wifissid, wifipw);
  innomaProtocol.waitUntilReady();
  Serial.println(F("Connected"));
}

void loop() {
  if(!innomaProtocol.ready()) {
    Serial.println(F("Disconnected"));

    innomaProtocol.begin(wifissid, wifipw);
    innomaProtocol.waitUntilReady();

    Serial.println(F("Connected"));
  }
  innomaProtocol.loop();
  
  innomaProtocol.setData(1, random(0, 100));
  innomaProtocol.setControl(1, (random(0, 1) == 1) ? HIGH : LOW);

  Serial.println(F("Send Data"));
  delay(2000);
}
