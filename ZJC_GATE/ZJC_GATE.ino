#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <soql_tools.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


#define FW_VERSION 11
#define FW_INFO "Kontroler bramy - zalesie"
#define ZAL
#ifdef ZAL
#define WIFI_COUNT 3
wifi_struct wifi[WIFI_COUNT]={  
  {"ZJCCRYPTO","820813130882"},
  {"ZJC-S","820813130882"},
  {"ZJC-N","820813130882"}  
};
#endif
#ifdef BW
#define WIFI_COUNT 1
wifi_struct wifi[WIFI_COUNT]={  
  {"SoqlNet","820813130882"}
};
#endif

/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define TOKEN "ESP8266_ZJC_GATE"

#ifdef ZAL
IPAddress mqttServerIP(192,168,1,168);  
#endif
#ifdef BW
IPAddress mqttServerIP(192,168,2,2);  
#endif

#define OUT_TOPIC "telemetry/gate"
#define IN_TOPIC  "telemetry/gate/set"
#define OUT_TOPIC_BELL "telemetry/gate/bell"
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Odebrano");
  Serial.println(topic);
  if(strcmp(topic, IN_TOPIC)==0){    
    String p;
    for(int j=0; j<length; j++){
      p+=(char)payload[j];
    }    
    Serial.println(p);
  }
}

void setup() {
 Serial.begin(115200);
 delay(10); 
 ConnectToAP(wifi, WIFI_COUNT);
 checkForUpdates(FW_VERSION);
 connectToMQTT(&client, mqttServerIP, TOKEN, callback, NULL);  
 sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
 pinMode(D0, INPUT_PULLUP);
 WiFi.setOutputPower(20.5);
}
int LINE_BREAK=0;
int LINE_OK=1;

int MINIMAL_BREAK_TIME_MS=50;
int ACCEPTABLE_FAST_OBJECT=300;

int REAL_DISTANCE_CM=10;
/*Statusy linii*/
int d8state;
int d7state;
/*Dzwonek*/
int d2state;

int d8prev_state;
int d7prev_state;
/*Dzwonek*/
int d2prev_state;

/*ms złamania linii */
long d7breakTime;
long d8breakTime;


boolean d7realBreak;
boolean d8realBreak;
boolean objectBreakBoth=false;

/*Czas rozpoczęcia blokady*/
long d7realBreakStartTime;
long d8realBreakStartTime;

/*Czas odblokowania linii po blokadzie*/
long d7realBreakStopTime;
long d8realBreakStopTime;

boolean fast=false;

void sendToMqttBell(){
    String payload = "{\"bell\":\"true\"}"; 
    
  sendToMqtt(&client,OUT_TOPIC_BELL, payload);
}
void sendToMqtt(boolean direction_, boolean full, boolean revert, boolean fast, int time_){
  String payload;    
    double dist_=((double)REAL_DISTANCE_CM*10000/100);  
    double rtime_ = (double)time_*10000/1000;
  
    payload = "{"; 
    payload += "\"direction\":\"";payload+=direction_?"IN":"OUT" ; payload += "\",";
    payload += "\"full\":\"";payload+=full?"true":"false" ; payload += "\",";
    payload += "\"revert\":\"";payload+=revert?"true":"false" ; payload += "\",";
    payload += "\"isFast\":\"";payload+=fast?"true":"false" ; payload += "\",";    
    payload += "\"speed\":\"";payload+=(3,6*dist_/rtime_); payload += "\",";
    payload += "\"time\":\"";payload+=time_ ; payload += "\"";
    payload += "}";
  sendToMqtt(&client,OUT_TOPIC, payload);
}
void loop() {
  Serial.flush();
  ConnectToAP(wifi, WIFI_COUNT);
  connectToMQTT(&client, mqttServerIP, TOKEN, callback, NULL);  
  client.loop();
  d7prev_state=d7state;
  d8prev_state=d8state;
  d2prev_state=d2state;
  
  d8state=digitalRead(D6);
  d7state=digitalRead(D7);
  d2state=digitalRead(D0);  
  if(d2state==0 && d2prev_state==1){
    sendToMqttBell();
  }
/*Zmiana statusu pinu D7*/
  if(d7state!=d7prev_state){

    if(d7state==LINE_BREAK){
      d7breakTime=millis();
    }
    if(d7state==LINE_OK){
      long breakTime;
      Serial.print("Koniec naruszenia d7. Trwało ");
      Serial.println(millis()-d7breakTime);      
      d7realBreakStopTime=millis();
    }
  
  }

  /*Zmiana statusu pinu D8*/
  if(d8state!=d8prev_state){  
    if(d8state==LINE_BREAK){
      d8breakTime=millis();
    }
    if(d8state==LINE_OK){
     Serial.print("Koniec naruszenia d8. Trwało ");
      Serial.println(millis()-d8breakTime);      
      d8realBreakStopTime=millis();
    }
  }

  /*Rozpoznanie zablokowania czujki (minimalny czas blokady jest brany pod uwagę*/
  if(d7state==LINE_BREAK && millis()-d7breakTime>MINIMAL_BREAK_TIME_MS){
    /*Jeżeli linia nie była wcześniej zablokowana*/
    if(!d7realBreak){
      Serial.print("Naruszenie d7 powyżej: ");
      Serial.print(MINIMAL_BREAK_TIME_MS);
      Serial.print(" - ");
      Serial.println(d7state);
      /*Blokujemy linię*/
      d7realBreak=true;
      /*Start czasu blokady*/
      d7realBreakStartTime=millis();
    }
  }else{
    /*Linia nie jest zablokowana*/
    d7realBreak=false;
  }
  
  if(d8state==LINE_BREAK && millis()-d8breakTime>MINIMAL_BREAK_TIME_MS){
    if(!d8realBreak){
      Serial.print("Naruszenie d8 powyżej: ");
      Serial.print(MINIMAL_BREAK_TIME_MS);
      Serial.print(" - ");
      Serial.println(d7state);
      
      d8realBreak=true;
      d8realBreakStartTime=millis();
    }
  }else{
    d8realBreak=false;
  }


  if(
    /*Flaga oznaczająca zablokowanie obu linii*/
    !objectBreakBoth && 
  /*Obie linie zablokowane */
  ((d7realBreak && d8realBreak) || 
  /*Linie niezablokowana ale odblokowana w bardzo niedawnym czasie (ACCEPTABLE_FAST_OBJECT(*/
  (     !d7realBreak && d8realBreak && d7realBreakStartTime+ACCEPTABLE_FAST_OBJECT>millis()) 
    || (!d8realBreak && d7realBreak && d8realBreakStartTime+ACCEPTABLE_FAST_OBJECT>millis())
  )){
    objectBreakBoth=true;
    boolean in=false;
    fast=false;    
    int speed_=0;
    /*d7 zablokowany wcześńiej niż d8*/
    if(d7realBreakStartTime>d8realBreakStartTime){
      Serial.print("WYKRYTO WJAZD W STREFĘ. ");   
      in=true;   
      /*Jeżeli obiekt przemisćił się szybko to któraś z linii jest niezablokowana (mały obiekt)*/
      if(!d7realBreak || !d8realBreak){
        Serial.print("SZYBKI OBIEKT.");
        fast=true;
      }
      Serial.println();      
      speed_=d7realBreakStartTime-d8realBreakStartTime;
      
    }else{
      Serial.print("WYKRYTO WYJAZD ZE STREFY. ");      
      in=false;
      if(!d7realBreak || !d8realBreak){
        Serial.print("SZYBKI OBIEKT.");
        fast=true;
      }
      Serial.println();
      speed_=d7realBreakStartTime-d8realBreakStartTime;
    }
    sendToMqtt(in, false, false, fast, speed_);
    
  }

  /*Obie linie były wcześniej zablokowane ale już są ok*/
  if(objectBreakBoth && d7realBreak==false && d8realBreak==false){
    /*d7 zalokowany wcześniej niż d8*/
    if(d7realBreakStartTime>d8realBreakStartTime){
      /*d7 odblokowany wcześniej niż d8 - czyli wjazd*/
      if(d7realBreakStopTime>d8realBreakStopTime){
        Serial.println("WYKRYTO PEŁNY WJAZD W STREFĘ");
        sendToMqtt(true, true, false, fast,d7realBreakStopTime-d8realBreakStopTime);
      /*d8 odblokowany wcześniej niż d7 - rezygnacja z wjazdu*/
      }else{
        Serial.println("WYKRYTO REZEGNACJĘ");
        sendToMqtt(true, true, true,fast,  d8realBreakStopTime-d7realBreakStopTime);
      }
    /*d8 zalokowany wcześniej niż d7*/
    }else{
      if(d8realBreakStopTime>d7realBreakStopTime){
        Serial.println("WYKRYTO PEŁNY WYJAZD ZE STREFY");
        sendToMqtt(false, true, false,fast, d8realBreakStopTime-d7realBreakStopTime);
      }else{
        Serial.println("WYKRYTO REZEGNACJĘ Z WYJAZDU");
        sendToMqtt(false, true,true, fast, d7realBreakStopTime-d8realBreakStopTime);
      }
    }
    objectBreakBoth=false;
    
    d7realBreakStopTime=0;
    d8realBreakStopTime=0;
    d7realBreakStartTime=0;
    d8realBreakStartTime=0;
  }
    
  
  
}
