#include "auxiliary.h"

IO valve1=IO(25,OUTPUT);
IO valve2=IO(23,OUTPUT);
IO valve3=IO(21,OUTPUT);
IO valve4=IO(19,OUTPUT);
IO valve5=IO(18,OUTPUT);
IO wifiLED=IO(LED_BUILTIN,OUTPUT);
IO mqttLED=IO(26,OUTPUT);
IO buttonLED=IO(27,OUTPUT);
IO moisture=IO(39,INPUT);
IO moisturePower=IO(14,OUTPUT);
WiFiMulti wifiMulti;
hw_timer_t *timer=NULL;
DHT dht(33, DHT22);
uint8_t device=0;
float temperature;
float humidity;
AsyncWebServer server(8080);
const char *DEVICE_LABEL = "horta2";  // Put here your Device label to which data  will be published
const char *VALVE1_LABEL="valve1";
const char *VALVE2_LABEL="valve2";
const char *VALVE3_LABEL="valve3";
const char *VALVE4_LABEL="valve4";
const char *VALVE5_LABEL="valve5";
const char *TEMPERATURE_LABEL="temperature";
const char *HUMIDITY_LABEL="humidity";
const char *MOISTURE_LABEL="moisture";
const char *UBIDOTS_TOKEN = "BBFF-vl76grmoEYF55xCwdGGfCrCjiwYxU9";  // Put here your Ubidots TOKEN
const uint32_t connectTimeoutMs=10e3;
int PUBLISH_INTERVAL = 80e3; // Update rate in milliseconds
Ubidots ubidots(UBIDOTS_TOKEN);


void setup()
{
  //pinMode(dhtpin,INPUT_PULLUP);
  //dht.begin();
  Serial.begin(115200);
  setConfigs();
  addCredentials();
  //networkScan();
  connectWiFi();
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  subscribeToTopics();
  serverSetup();
  OTASetup();
  //hwTimerSetup();
 
 
}

void loop()
{
  checkConnections();
  //serialCommands();
  //publishToTopics();
  checkStates();
  ubidots.loop();
  ArduinoOTA.handle();
}