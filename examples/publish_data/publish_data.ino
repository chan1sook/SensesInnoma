/* 
  Example - Publish Data to Innoma Platform
  Created by Natthawat Raocharoensinp, June 20, 2022.
  Released into the public domain.
*/
#include <SensesInnoma.h>

const char userid[] PROGMEM = "1";
const char key[] PROGMEM = "t2dv7e0pwl7k";
const char wifissid[] PROGMEM = "ICE_2.4G";
const char wifipw[] PROGMEM = "29592959";

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

  Serial.print(F("Free Memory: "));
  Serial.println(ESP.getFreeHeap());
  delay(2000);
}
