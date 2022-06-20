#include "Arduino.h"
#include "InnomaClient.h"

#ifdef LOCALHOST_TEST
const char* InnomaClient::_API_HOST PROGMEM = "192.168.1.11";
#else
const char* InnomaClient::_API_HOST PROGMEM = "innoma.sensesiot.com";
#endif

const char* PROGMEM InnomaClient::_GETDATA_URL_FSTR  = "/getdata/%s/%s/%d";
const char* PROGMEM InnomaClient::_GETCONTROL_URL_FSTR = "/getcontrol/%s/%s/%d";
const char* PROGMEM InnomaClient::_SETDATA_URL_FSTR = "/setdata/%s/%s/%d/%s";
const char* PROGMEM InnomaClient::_SETCONTROL_URL_FSTR = "/setcontrol/%s/%s/%d/%s";

const char* PROGMEM InnomaClient::_CLIENT_MACADDR_FSTR  = "wf-%02X:%02X:%02X:%02X:%02X:%02X";
const char* PROGMEM InnomaClient::_MQTT_USERNAME_FSTR = "%s:%s";
const char* PROGMEM InnomaClient::_DATA_TOPIC_FSTR = "data/%s/%s/%d";
const char* PROGMEM InnomaClient::_CONTROL_TOPIC_FSTR = "control/%s/%s/%d";
const char* PROGMEM InnomaClient::_DATA_TOPIC_START = "data/";
const char* PROGMEM InnomaClient::_CONTROL_TOPIC_START = "control/";

const char* PROGMEM InnomaClient::_ONSTATE_STR = "on";
const char* PROGMEM InnomaClient::_OFFSTATE_STR = "off";

InnomaClient::InnomaClient(const char* userid, const char* devicekey) {
  setUserid(userid);
  setDevicekey(devicekey);
}

const char* InnomaClient::getUserid() {
  return _userid;
}

void InnomaClient::setUserid(const char* userid) {
  strncpy(_userid, userid, USERIDSTR_BUFFER_SIZE - 1);
  _devicekey[USERIDSTR_BUFFER_SIZE] = '\0';
}

const char* InnomaClient::getDevicekey() {
  return _devicekey;
}

void InnomaClient::setDevicekey(const char* devicekey) {
  strncpy(_devicekey, devicekey, DEVKEYSTR_BUFFER_SIZE - 1);
  _devicekey[DEVKEYSTR_BUFFER_SIZE - 1] = '\0';
}

bool InnomaClient::begin(const char* wifissid, const char* wifipw) {
  _mqttClient.setClient(_wifiClient);
  _mqttClient.setServer(_API_HOST, _MQTT_PORT);
  _mqttClient.setCallback([=](char* topic, byte* payload, unsigned int length) { 
    mqttCallback(topic, payload, length);
  });

  WiFi.begin(wifissid, wifipw);
  connectMqtt();

  return ready();
}

wl_status_t InnomaClient::wifiStatus() {
  return WiFi.status();
}

bool InnomaClient::ready() {
  return WiFi.isConnected() && _mqttClient.connected();
}

void InnomaClient::waitUntilReady() {
  while(!WiFi.isConnected());
  while(!_mqttClient.connected()) {
    connectMqtt();
  }
}

double InnomaClient::getData(uint8_t slot) {
  char urlBuffer[URL_BUFFER_SIZE];
  char floatStrBuffer[FLOATSTR_BUFFER_SIZE];

  sprintf(urlBuffer, _GETDATA_URL_FSTR, _userid, _devicekey, slot);

  _httpClient.begin(_wifiClient, _API_HOST, _API_PORT, urlBuffer);
  int statusCode = _httpClient.GET();

  if(statusCode != 200) {
    return NAN;
  }

  WiFiClient stream = _httpClient.getStream();
  int byteRead = stream.readBytes(floatStrBuffer, 31);
  floatStrBuffer[byteRead] = '\0';

  return atof(floatStrBuffer);
}

uint8_t InnomaClient::getControl(uint8_t slot) {
  char urlBuffer[URL_BUFFER_SIZE];
  char floatStrBuffer[FLOATSTR_BUFFER_SIZE];
  
  sprintf(urlBuffer, _GETCONTROL_URL_FSTR, _userid, _devicekey, slot);

  _httpClient.begin(_wifiClient, _API_HOST, _API_PORT, urlBuffer);
  int statusCode = _httpClient.GET();
  if(statusCode != 200) {
    return STATE_UNKNOWN;
  }
  
  WiFiClient stream = _httpClient.getStream();
  int byteRead = stream.readBytes(floatStrBuffer, 31);
  floatStrBuffer[byteRead] = '\0';

  if(strcmp(_ONSTATE_STR, floatStrBuffer) == 0) {
    return HIGH;
  } else {
    return LOW;
  }
}

bool InnomaClient::setData(uint8_t slot, double value) {
  #ifdef USE_MQTT_SET
    char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
    char floatStrBuffer[FLOATSTR_BUFFER_SIZE];
    
    dtostrf(value, 0, 4, floatStrBuffer);
    sprintf(topicBuffer, _DATA_TOPIC_FSTR, _userid, _devicekey, slot);
    return _mqttClient.publish(topicBuffer, floatStrBuffer);
  #else
    char urlBuffer[URL_BUFFER_SIZE];
    char floatStrBuffer[FLOATSTR_BUFFER_SIZE];

    dtostrf(value, 0, 4, floatStrBuffer);
    sprintf(urlBuffer, _SETDATA_URL_FSTR, _userid, _devicekey, slot, floatStrBuffer);
    
    _httpClient.begin(_wifiClient, _API_HOST, _API_PORT, urlBuffer);
    int statusCode = _httpClient.GET();
    return statusCode == 200;
  #endif
}

bool InnomaClient::setControl(uint8_t slot, uint8_t state) {
  #ifdef USE_MQTT_SET
    char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
    const char* stateStr = (state == LOW) ? _OFFSTATE_STR : _ONSTATE_STR;
    
    sprintf(topicBuffer, _CONTROL_TOPIC_FSTR, _userid, _devicekey, slot);
    return _mqttClient.publish(topicBuffer, stateStr);
  #else
    char urlBuffer[URL_BUFFER_SIZE];
    const char* stateStr = (state == LOW) ? _OFFSTATE_STR : _ONSTATE_STR;
    sprintf(urlBuffer, _SETCONTROL_URL_FSTR, _userid, _devicekey, slot, stateStr);

    _httpClient.begin(_wifiClient, _API_HOST, _API_PORT, urlBuffer);
    int statusCode = _httpClient.GET();
    return statusCode == 200;
  #endif
}

bool InnomaClient::connectMqtt() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[CLIENT_WIFI_BUFFER_SIZE];
  sprintf(macStr, _CLIENT_MACADDR_FSTR, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  char usernameStr[CLIENT_USERNAME_BUFFER_SIZE];
  sprintf(usernameStr, _MQTT_USERNAME_FSTR, _userid, _devicekey);
  
  bool status = _mqttClient.connect(macStr, usernameStr, NULL);

  return status;
}

void InnomaClient::loop() {  
  _mqttClient.loop();
}

int InnomaClient::mqttStatus() {
  return _mqttClient.state();
}

bool InnomaClient::subscribeData(uint8_t slot) {
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _DATA_TOPIC_FSTR, _userid, _devicekey, slot);

  return _mqttClient.subscribe(topicBuffer);
}

bool InnomaClient::subscribeControl(uint8_t slot) {
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _CONTROL_TOPIC_FSTR, _userid, _devicekey, slot);
  
  return _mqttClient.subscribe(topicBuffer);
}

bool InnomaClient::unsubscribeData(uint8_t slot) {
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _DATA_TOPIC_FSTR, _userid, _devicekey, slot);

  return _mqttClient.unsubscribe(topicBuffer);
}

bool InnomaClient::unsubscribeControl(uint8_t slot) {
  char topicBuffer[MQTTTOPIC_BUFFER_SIZE];
  sprintf(topicBuffer, _CONTROL_TOPIC_FSTR, _userid, _devicekey, slot);

  return _mqttClient.unsubscribe(topicBuffer);
}

void InnomaClient::setDataCallback(innomaDataCallback callback) {
  _mqttDataCallback = callback;
}

void InnomaClient::setControlCallback(innomaControlCallback callback) {
  _mqttControlCallback = callback;
}

#ifdef USE_ARDUINO_STRING
InnomaClient::InnomaClient(String userid, String devicekey) {
  setUserid(userid);
  setDevicekey(devicekey);
}

void InnomaClient::setUserid(String userid) {
  strncpy(_userid, userid.c_str(), USERIDSTR_BUFFER_SIZE - 1);
  _devicekey[USERIDSTR_BUFFER_SIZE - 1] = '\0';
}

void InnomaClient::setDevicekey(String devicekey) {
  strncpy(_devicekey, devicekey.c_str(), DEVKEYSTR_BUFFER_SIZE - 1);
  _devicekey[DEVKEYSTR_BUFFER_SIZE - 1] = '\0';
}
#endif

void InnomaClient::mqttCallback(char* topic, byte* payload, unsigned int length) {
  uint8_t slot;
  char payloadStr[length + 1];
  strncpy(payloadStr, (char*) payload, length);
  payloadStr[length] = '\0';

  if(isTopicStartWith(topic, _DATA_TOPIC_START) && _mqttDataCallback != NULL) {
    slot = slotFromTopic(topic);

    double value = atof(payloadStr);

     _mqttDataCallback(slot, value);

    return;
  }
  
  if(isTopicStartWith(topic, _CONTROL_TOPIC_START) && _mqttControlCallback != NULL) {
    slot = slotFromTopic(topic);
    
    if(strcmp(_ONSTATE_STR, payloadStr) == 0) {
      _mqttControlCallback(slot, HIGH);
    } else {
      _mqttControlCallback(slot, LOW);
    }
  }
}

bool InnomaClient::isTopicStartWith(const char* topic, const char* str) {
  int len = strlen(str);
  char strBuffer[len + 1];
  strncpy(strBuffer, topic, len);
  strBuffer[len] = '\0';

  return strcmp(strBuffer, str) == 0;
}

uint8_t InnomaClient::slotFromTopic(const char* topic) {
  int len = strlen(topic);
  int start = len;
  for(; start >= 0; start--) {
    if(topic[start] == '/') {
      break;
    }
  }

  return atoi(topic + start + 1);
}