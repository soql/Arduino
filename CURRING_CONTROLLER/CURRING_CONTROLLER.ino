#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WString.h>
#include <soql_tools.h>

#define TOKEN "ESP8266_CURRING"

#define FW_VERSION 1
#define FW_INFO "Kontroler curringu"

IPAddress mqttServerIP(192,168,1,168);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#define WIFI_COUNT 2

wifi_struct wifi[2]={  
  {"ZJC-N","820813130882"},
  {"SoqlNet","820813130882"}
};

int PHASE=0;

int TOTAL_OFF=0;
int PHASE_1=1;
int PHASE_1_OFF=2;
int PHASE_2=3;
int PHASE_2_OFF=4;

#define IN_TOPIC "/telemetry/curring/set"
#define OUT_TOPIC "/telemetry/curring/get"

void callback(char* topic, byte* payload, unsigned int length) ;

int prevStateD2;
boolean pushedD2=false;
long activeD1Time;
void setup() {
  initSerial(115200);
  ConnectToAP(wifi, WIFI_COUNT);
  checkForUpdates(FW_VERSION);
  connectToMQTT(&mqttClient, mqttServerIP, TOKEN, callback, NULL);  
  sendToMqtt(&mqttClient,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));  
  
  mqttClient.setServer( mqttServerIP, 1883);  
  mqttClient.setCallback(callback);
  
  pinMode(D3, OUTPUT);  

  pinMode(D5, OUTPUT);  
  pinMode(D6, OUTPUT);  

  pinMode(D2, INPUT);  

  digitalWrite(D5,1);
  digitalWrite(D6,1);

  PHASE=TOTAL_OFF;
}


void loop() {  
  ConnectToAP(wifi, 2); 
  connectToMQTT(&mqttClient, mqttServerIP, TOKEN, callback, NULL);  
  mqttClient.subscribe(IN_TOPIC);
  mqttClient.loop();      
  /*Dioda*/  
  /*digitalWrite(D3,1);
  delay(500);
  digitalWrite(D3,0);
  delay(500);*/
  int actStateD2=digitalRead(D2);   
  //Serial.println(actStateD2);
  if(actStateD2==1 && prevStateD2==0){     
    activeD1Time=millis(); 
  }
  if(actStateD2==0 && prevStateD2==1){     
    digitalWrite(D3,0);
    pushedD2=false;
  }
  if(!pushedD2 && actStateD2==1 && millis()-activeD1Time>50){
    digitalWrite(D3,1);
    activeD1Time=millis();  
    pushedD2=true;        
    changePhase();
  }
  prevStateD2=actStateD2;
}

void changePhase(){
    Serial.println("Wcisnete");
    
    if(PHASE==TOTAL_OFF || PHASE==PHASE_2_OFF){
      digitalWrite(D5,0);
      digitalWrite(D6,1);
      PHASE=PHASE_1;
      return;
    }
    if(PHASE==PHASE_1){
      digitalWrite(D5,1);
      digitalWrite(D6,1);
      PHASE=PHASE_1_OFF;
      return;
    }
    if(PHASE==PHASE_1_OFF){
      digitalWrite(D5,1);
      digitalWrite(D6,0);
      PHASE=PHASE_2;
      return;
    }
    if(PHASE==PHASE_2){
      digitalWrite(D5,1);
      digitalWrite(D6,1);
      PHASE=PHASE_2_OFF;
      return;
    }  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("EEE");
  if(strcmp(topic, IN_TOPIC)==0){    
    changePhase();
  }
}
 
