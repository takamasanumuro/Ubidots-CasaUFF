#include "auxiliary.h"



enum valveState{
    VALVE1_LOW=0,
    VALVE1_HIGH,
    VALVE2_LOW,
    VALVE2_HIGH,
    VALVE3_LOW,
    VALVE3_HIGH,
    VALVE4_LOW,
    VALVE4_HIGH,
    VALVE5_LOW,
    VALVE5_HIGH,
} ;

unsigned long publishTimer = millis();




void callback(char *topic, byte *payload, unsigned int length)
{
  char buffer[64];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
   
  for (int i = 0; i < length; i++)
  {
    buffer[i]=(char)payload[i];
    Serial.print((char)payload[i]); // converting from ascii to integer digit
  }
  int command=atoi(buffer);
  switch(command){
    case 0:
      valve1.off();
      buttonLED.toggle();
      break;
    case 1:
      valve1.on();
      buttonLED.toggle();
      break;
    case 2:
      valve2.off();
      buttonLED.toggle();
      break;
    case 3:
      valve2.off();      
      buttonLED.toggle();
      break;
    case 4:
      valve3.off();
      buttonLED.toggle();
      break;
    case 5:
      valve3.off();
      buttonLED.toggle();
      break;
    case 6:
      valve4.off();
      buttonLED.toggle();
      break;
    case 7:
      valve4.off();
      buttonLED.toggle();
      break;
    case 8:
      valve5.off();
      buttonLED.toggle();
      break;
    case 9:
      valve5.on();
      buttonLED.toggle();
      break;
    
  }
  Serial.println();
}

void networkScan(){
   // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) Serial.println("no networks found");
  else{
    Serial.printf("%d networks found",n);
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.printf("%d: %s (%d)",i+1,WiFi.SSID(i).c_str(),WiFi.RSSI(i));
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);

      String net = WiFi.SSID(i);
      for (int x = 0; x < net.length(); ++x)
      {
        Serial.print((int)net[x]);
        Serial.print(" ");
      }
      Serial.println("\n----------");

      delay(10);
    }
  }
}
void IO::safety(uint32_t safetyInterval=180e3){
  if(millis()-_timer>safetyInterval){
    _timer=millis();
    if(_state){
      _state=false;
      digitalWrite(_pin,_state);
      ubidots.add(VALVE1_LABEL,VALVE1_LOW);
      ubidots.publish(DEVICE_LABEL);
    }
  }
}

unsigned long IO::timer(){
  return _timer;
}

void checkStates(){

  valve1.safety(300);
  if(millis()-valve5.timer()>60e3*90){
    valve5.off();
  }
}

void connectWiFi(){
 
  while (wifiMulti.run() != WL_CONNECTED) {
    static int counter = 0;
    static unsigned long timer = millis();
    valve1.off(); valve2.on(); valve3.on(); valve4.on(); valve5.off();
    if (millis() - timer > 500)
    {
      //wifiLED.blink(500);
      mqttLED.blink(500);
      buttonLED.blink(500);
      Serial.printf(".");
      ++counter;
      if (counter == 80)
        ESP.restart();
      timer = millis();
    }
  }
  //wifiLED.on();
  
}

void checkConnections(){
  connectWiFi();
  if (!ubidots.connected())
  {
    ubidots.reconnect(); // blocking function
    mqttLED.on();
    subscribeToTopics();
  }
}
void addCredentials(){
  
  wifiMulti.addAP("espnet","12345678");
  wifiMulti.addAP("TP_LINK_88EE","86122238");
}

void OTASetup(){
    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void serialCommands(){
    if(Serial.available()){
    char incomingByte=Serial.read();
    switch(incomingByte){
      case 'p':
        Serial.println();
        Serial.println(WiFi.RSSI());
        Serial.println(WiFi.localIP());
        Serial.println(WiFi.SSID());
        break;
     
      case 'u':
        Serial.println();
        Serial.printf("Ubidots: %d",ubidots.connected());
        break;
      case 'o':
        Serial.println();
        ubidots.disconnect();
        delay(100);
        Serial.printf("Ubidots: %d",ubidots.connected());
        delay(2000);
        break;
      case 'n':
        Serial.println();
        networkScan();
        break;
    

      case 'm':
        moisturePower.on();
        Serial.printf("\nAnalog: %d\n",analogRead(moisture.pin()));
        moisture.value=getAverage(moisture.pin());
        Serial.printf("Average: %f\n",moisture.value);
        moisture.value=mapIntervals(moisture.value,800,2047,100,0);
        temperature=dht.readTemperature();
        humidity=dht.readHumidity();
        Serial.printf("Temperature: %f\n Humidity: %f\nMoisture: %f\n",temperature,humidity,moisture.value);
        moisturePower.off();
        break;
      case 's':
        Serial.printf("\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");
        break;
    }
  }
}

void hwTimerSetup(){
  
  pinMode(LED_BUILTIN, OUTPUT);
  timer=timerBegin(0,80,true);
  timerAttachInterrupt(timer,&hwTimerCallback,true);
  timerAlarmWrite(timer,1e6,true);
  timerAlarmEnable(timer);
}
void IRAM_ATTR hwTimerCallback(){
  static bool state=LOW;
  state=!state;
  digitalWrite(LED_BUILTIN,state);
}

IO::IO(uint8_t pin,uint8_t mode)
{
    
    _pin = pin;
    _mode=mode;
    if(mode == OUTPUT) {
        pinMode(_pin,mode);
        digitalWrite(pin,LOW);
    }
    else if(mode==INPUT) pinMode(pin,INPUT);
    else if(mode==INPUT_PULLUP) pinMode(pin,INPUT_PULLUP);
    else if(mode==INPUT_PULLDOWN) pinMode(pin,INPUT_PULLDOWN);

    _timer=millis();
    off();
}

void IO::on(){
    _state=HIGH;
    digitalWrite(_pin, _state);
    _timer=millis();
}

uint16_t IO::read(){
  if(_mode==INPUT) return analogRead(_pin);
  else if(_mode==INPUT_PULLUP) return digitalRead(_pin);
  else if(_mode==INPUT_PULLDOWN) return digitalRead(_pin);
  else return 0;
}

void IO::setMode(uint8_t mode){
  _mode=mode;
  pinMode(_pin,_mode);
}


void IO::off(){
    _state=LOW;
    digitalWrite(_pin, _state);
    _timer=millis();
}

void IO::toggle(){
    digitalWrite(_pin, !digitalRead(_pin));
    _timer=millis();
}	

void IO::blink(uint16_t interval){
    if(millis()-_timer>interval){
        toggle();
    }
}

bool IO::state(){
  return _state;
}

uint8_t IO::pin(){
  return _pin;
}
void subscribeToTopics(){

  ubidots.subscribeLastValue(DEVICE_LABEL,VALVE1_LABEL);
  ubidots.subscribeLastValue(DEVICE_LABEL,VALVE2_LABEL);
  ubidots.subscribeLastValue(DEVICE_LABEL,VALVE3_LABEL);
  ubidots.subscribeLastValue(DEVICE_LABEL,VALVE4_LABEL);
  ubidots.subscribeLastValue(DEVICE_LABEL,VALVE5_LABEL);

}
void readMoisture(){
  moisturePower.on();
  moisture.value=getAverage(moisture.pin());
  moisture.value=mapIntervals(moisture.value,800,2047,100,0);
  moisturePower.off();
}



void publishToTopics(){
 
  if(millis()-publishTimer>PUBLISH_INTERVAL){
   
    temperature=dht.readTemperature();
    humidity=dht.readHumidity();
    readMoisture();
  
    Serial.printf("Temperature: %f\nHumidity: %f\nMoisture: %f\n",temperature,humidity,moisture.value);
    if(temperature!=NAN && humidity!=NAN){
      ubidots.add(TEMPERATURE_LABEL,temperature);
      ubidots.add(HUMIDITY_LABEL,humidity);
      ubidots.add(MOISTURE_LABEL,moisture.value);
      ubidots.publish(DEVICE_LABEL);
    }
    else{ Serial.println("Error reading DHT sensor");}
     publishTimer=millis();
  }
}

long ifloor(long n,long d){return ((n%d)<0L)?(n/d-1):n/d;}
long iceil(long n,long d){return ((n%d)>0L)?(n/d+1):n/d;}
long mapIntervals(long x, long in_min, long in_max, long out_min, long out_max)
{
  if(in_min==in_max) return 0x7FFFFFFF; // slope is infinite; return max (long)
  long x1,x2,y1,y2;
  if(((in_max-in_min)<0)!=((out_max-out_min)<0)) {
    // Slope is negative
    x1 = min(in_min,in_max) - 1;
    x2 = max(in_min,in_max);
    y1 = max(out_min,out_max) + 1;
    y2 = min(out_min,out_max);
  }
  else {
    // Slope is positive
    x1 = min(in_min,in_max);
    x2 = max(in_min,in_max) + 1;
    y1 = min(out_min,out_max);
    y2 = max(out_min,out_max) + 1;
  }
  long dx = x2-x1;
  long dy = y2-y1;
  return ifloor((x-x1)*dy+y1*dx,dx);
} // End mapIntervals

void setConfigs(){
  
  analogReadResolution(11);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector, which turns off the ESP32 when the voltage is too low
  esp_err_t esp_wifi_set_ps(WIFI_PS_NONE); // disable power saving for wifi
}

 float getAverage(uint8_t pin){
  uint16_t counter=0, maxValue=0,minValue=2047,value;
  for(int i=0;i<6;i++){
    value=analogRead(pin);
     if(value<minValue) minValue=value;
    if(value>maxValue) maxValue=value;
    counter+=value;
  }
  //Serial.printf("Min: %d\nMax: %d\n",minValue,maxValue);
  //Serial.printf("Numerator: %d\n",counter-minValue-maxValue);
  return (counter-minValue-maxValue)/4.0;

}

void serverSetup(){
  
  server.onNotFound([](AsyncWebServerRequest* request){
    request->send(404,"text/plain","Not Found Anything");
  });
  
 
  server.on("/",HTTP_GET,[](AsyncWebServerRequest* request){
    if(request->hasParam("interval")) {
      PUBLISH_INTERVAL=request->getParam("interval")->value().toInt(); 
      request->send(200,"text/plain","Interval set to "+String(PUBLISH_INTERVAL));
    }else
    request->send(400,"text/plain","Bad Interval Request");
  });

  server.on("/temperature",HTTP_GET,[](AsyncWebServerRequest* request){
    
      temperature=dht.readTemperature();
      ubidots.add(TEMPERATURE_LABEL,temperature);
      ubidots.publish(DEVICE_LABEL);
      request->send(200,"text/plain","Temperature: "+String(temperature));
    
    ;
  });
  server.on("/humidity",HTTP_GET,[](AsyncWebServerRequest* request){
   
      humidity=dht.readHumidity();
      ubidots.add(HUMIDITY_LABEL,humidity);
      ubidots.publish(DEVICE_LABEL);
      request->send(200,"text/plain","Humidity: "+String(humidity));
    
  });
  server.on("/moisture",HTTP_GET,[](AsyncWebServerRequest* request){
    
      readMoisture();
      ubidots.add(MOISTURE_LABEL,moisture.value);
      ubidots.publish(DEVICE_LABEL);
      request->send(200,"text/plain","Moisture: "+String(moisture.value));
    
  });

  server.on("/valve1",HTTP_GET,[](AsyncWebServerRequest* request){
    if(request->hasParam("state")) {
      AsyncWebParameter* state=request->getParam("state");
      if(state->value().toInt()==VALVE1_HIGH){
        valve1.on();
        request->send(200,"text/plain","Valve 1 is on");
        ubidots.add(VALVE1_LABEL,VALVE1_HIGH);
        ubidots.publish(DEVICE_LABEL);
          
      }
      else if(state->value().toInt()==VALVE1_LOW){
        valve1.off();
        request->send(200,"text/plain","Valve 1 is off");
        ubidots.add(VALVE1_LABEL,VALVE1_LOW);
        ubidots.publish(DEVICE_LABEL);
          
      } 
    }else request->send(400,"text/plain","Bad Valve 1 Request");
  });

  server.on("/valve2",HTTP_GET,[](AsyncWebServerRequest* request){
    if(request->hasParam("state")) {
      AsyncWebParameter* state=request->getParam("state");
      if(state->value().toInt()==HIGH){
        valve2.off();
        request->send(200,"text/plain","Valve 2 is on");
        ubidots.add(VALVE2_LABEL,VALVE2_HIGH);
        ubidots.publish(DEVICE_LABEL);
          
      }
      else if(state->value().toInt()==LOW){
        valve2.on();
        request->send(200,"text/plain","Valve 2 is off");
        ubidots.add(VALVE2_LABEL,VALVE2_LOW);
        ubidots.publish(DEVICE_LABEL);
          
      } 
    }else request->send(400,"text/plain","Bad Valve 2 Request");
  });

  server.on("/valve3",HTTP_GET,[](AsyncWebServerRequest* request){
    if(request->hasParam("state")) {
      AsyncWebParameter* state=request->getParam("state");
      if(state->value().toInt()==HIGH){
        valve3.off();
        request->send(200,"text/plain","Valve 3 is on");
        ubidots.add(VALVE3_LABEL,VALVE3_HIGH);
        ubidots.publish(DEVICE_LABEL);
          
      }
      else if(state->value().toInt()==LOW){
        valve3.on();
        request->send(200,"text/plain","Valve 3 is off");
        ubidots.add(VALVE3_LABEL,VALVE3_LOW);
        ubidots.publish(DEVICE_LABEL);
          
      } 
    }else request->send(400,"text/plain","Bad Valve 3 Request");
  });
  
  server.on("/valve4",HTTP_GET,[](AsyncWebServerRequest* request){
    if(request->hasParam("state")) {
      AsyncWebParameter* state=request->getParam("state");
      if(state->value().toInt()==HIGH){
        valve4.off();
        request->send(200,"text/plain","Valve 4 is on");
        ubidots.add(VALVE4_LABEL,VALVE4_HIGH);
        ubidots.publish(DEVICE_LABEL);
          
      }
      else if(state->value().toInt()==LOW){
        valve4.on();       
        request->send(200,"text/plain","Valve 4 is off");
        ubidots.add(VALVE4_LABEL,VALVE4_LOW);
        ubidots.publish(DEVICE_LABEL);
          
      } 
    }else request->send(400,"text/plain","Bad Valve 4 Request");
  });

  server.on("/valve5",HTTP_GET,[](AsyncWebServerRequest* request){
    if(request->hasParam("state")) {
      AsyncWebParameter* state=request->getParam("state");
      if(state->value().toInt()==HIGH){
        valve5.on();
        request->send(200,"text/plain","Valve 5 is on");
        ubidots.add(VALVE5_LABEL,VALVE5_HIGH);
        ubidots.publish(DEVICE_LABEL);
          
      }
      else if(state->value().toInt()==LOW){
        valve5.off();
        request->send(200,"text/plain","Valve 5 is off");
        ubidots.add(VALVE5_LABEL,VALVE5_LOW);
        ubidots.publish(DEVICE_LABEL);
          
      } 
    }else request->send(400,"text/plain","Bad Valve 5 Request");
  });

  server.begin();
}