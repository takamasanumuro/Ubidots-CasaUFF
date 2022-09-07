#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
#define dhtpin 33
#ifndef AUXILIARY_FUNCTIONS_H
#define AUXILIARY_FUNCTIONS_H
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "UbidotsEsp32Mqtt.h"
#include <WiFiMulti.h>
#include <UbiConstants.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "DHT.h"
#include "ESPAsyncWebServer.h"
class IO{

private:
    
    
    unsigned long _timer;
    bool _state;
    uint8_t _mode;
    uint8_t _pin;
public:
    IO(uint8_t pin,uint8_t mode);
    void on();
    void off();
    void toggle();
    void blink(uint16_t interval);
    bool state();
    void safety(uint32_t safetyInterval);
    uint16_t read();
    void setMode(uint8_t mode);
    uint8_t pin();
    float value;
    unsigned long timer();
    
};


extern unsigned long publishTimer;
extern hw_timer_t *timer;
extern IO valve1;
extern IO valve2;
extern IO valve3;
extern IO valve4;
extern IO valve5;
extern WiFiMulti wifiMulti;
extern IO wifiLED;
extern IO mqttLED;
extern IO buttonLED;
extern IO moisture;
extern IO moisturePower;
extern Ubidots ubidots;
extern DHT dht;
extern AsyncWebServer server;

extern float temperature;
extern float humidity;

extern const char *DEVICE_LABEL;  // Put here your Device label to which data  will be published
extern const char *VALVE1_LABEL;
extern const char *VALVE2_LABEL;
extern const char *VALVE3_LABEL;
extern const char *VALVE4_LABEL;
extern const char *VALVE5_LABEL;
extern const char *TEMPERATURE_LABEL;
extern const char *HUMIDITY_LABEL;
extern const char *MOISTURE_LABEL;
extern  int PUBLISH_INTERVAL;
void networkScan();
void connectWiFi();
void addCredentials();
void checkConnections();
void OTASetup();
void serialCommands();
void callback(char* topic, byte* payload, unsigned int length);
void hwTimerSetup();
void hwTimerCallback();
void subscribeToTopics();
void publishToTopics();
long mapIntervals(long x, long in_min, long in_max, long out_min, long out_max);
void setConfigs();
float getAverage(uint8_t pin);
void serverSetup();
void checkStates();
#endif