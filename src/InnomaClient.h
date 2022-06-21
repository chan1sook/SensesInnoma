/* 
  InnomaClient.h - A library for send and receive data from "Innoma Platform"
  Created by Natthawat Raocharoensinp, June 18, 2022.
  Released into the public domain.
*/
#ifndef InnomaClient_h
#define InnomaClient_h

// #define LOCALHOST_TEST

#define MQTT_SETDATA

#define USERIDSTR_BUFFER_SIZE (11)
#define DEVKEYSTR_BUFFER_SIZE (21)
#define CLIENT_WIFI_BUFFER_SIZE (21)
#define CLIENT_USERNAME_BUFFER_SIZE (31)
#define URL_BUFFER_SIZE (128)
#define MQTTTOPIC_BUFFER_SIZE (64)
#define FLOATSTR_BUFFER_SIZE (33)

#define STATE_UNKNOWN 0xFF

typedef void (*innomaDataCallback)(uint8_t, double);
typedef void (*innomaControlCallback)(uint8_t, uint8_t);

#include "Arduino.h"

#ifdef ESP32
#include "WiFi.h"
#include "HTTPClient.h"
#else
#error "Unsupported Board Architecture"
#endif

#include "PubSubClient.h"

class InnomaClient {
  private:
    static const char* PROGMEM _API_HOST;

    static const uint16_t PROGMEM _API_PORT = 4000;
    static const char* PROGMEM _GETDATA_URL_FSTR;
    static const char* PROGMEM _GETCONTROL_URL_FSTR;
    static const char* PROGMEM _SETDATA_URL_FSTR;
    static const char* PROGMEM _SETCONTROL_URL_FSTR;
    
    static const uint16_t PROGMEM _MQTT_PORT = 4001;
    static const char* PROGMEM _CLIENT_MACADDR_FSTR;
    static const char* PROGMEM _MQTT_USERNAME_FSTR;
    static const char* PROGMEM _DATA_TOPIC_FSTR;
    static const char* PROGMEM _CONTROL_TOPIC_FSTR;
    static const char* PROGMEM _DATA_TOPIC_START;
    static const char* PROGMEM _CONTROL_TOPIC_START;
    
    static const char* PROGMEM _OFFSTATE_STR;
    static const char* PROGMEM _ONSTATE_STR;

    char _userid[USERIDSTR_BUFFER_SIZE];
    char _devicekey[DEVKEYSTR_BUFFER_SIZE];
    WiFiClient _wifiClient;
    HTTPClient _httpClient;
    PubSubClient _mqttClient;

    innomaDataCallback _mqttDataCallback;
    innomaControlCallback _mqttControlCallback;
  public:
    InnomaClient(const char* userid, const char* devicekey);
    
    const char* getUserid();
    void setUserid(const char* userid);
    const char* getDevicekey();
    void setDevicekey(const char* userid);

    bool begin(const char* wifissid, const char* wifipw);
    wl_status_t wifiStatus();
    bool ready();
    void waitUntilReady();

    double getData(uint8_t slot);
    uint8_t getControl(uint8_t slot);
    bool setData(uint8_t slot, double value);
    bool setControl(uint8_t slot, uint8_t state);

    bool connectMqtt();
    void loop();
    int mqttStatus();

    bool subscribeData(uint8_t slot);
    bool subscribeControl(uint8_t slot);
    bool unsubscribeData(uint8_t slot);
    bool unsubscribeControl(uint8_t slot);

    void setDataCallback(innomaDataCallback callback);
    void setControlCallback(innomaControlCallback callback);
    
    InnomaClient(String userid, String devicekey);
    void setUserid(String userid);
    void setDevicekey(String userid);
  protected:
    void mqttCallback(char* topic, uint8_t* payload, unsigned int length);
    bool isTopicStartWith(const char* topic, const char* str);
    uint8_t slotFromTopic(const char* topic);
};

#endif
