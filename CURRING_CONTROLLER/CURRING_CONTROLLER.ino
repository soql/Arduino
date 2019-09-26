#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WString.h>
#include <soql_tools.h>

#define TOKEN "ESP8266_CURRING"

#define FW_VERSION 9
#define FW_INFO "Kontroler curringu"

IPAddress mqttServerIP(192,168,1,168);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define WIFI_COUNT 2

wifi_struct wifi[2]={  
  {"ZJC-N","820813130882"},
  {"SoqlNet","820813130882"}
};

int PHASE=0;


int TOTAL_OFF=0;
/*Zamknięte (docisk)*/
int PHASE_1=1;
/*Zamknięte*/
int PHASE_1_OFF=2;
/*Otwarte (docisk)*/
int PHASE_2=3;
/*Otwarte*/
int PHASE_2_OFF=4;

#define IN_TOPIC "/telemetry/curring/setr"
#define OUT_TOPIC "/telemetry/curring/get"

int sendMqttTime;

void callback(char* topic, byte* payload, unsigned int length) ;
void changePhase( );
int changePhaseAuto=0;
int prevStateD2;
boolean pushedD2=false;
long activeD1Time;
void setup() {
  initSerial(115200);
  ConnectToAP(wifi, WIFI_COUNT);
  checkForUpdates(FW_VERSION);
  connectToMQTT(&client, mqttServerIP, TOKEN, callback, NULL);  
  sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));  
  
  client.setServer( mqttServerIP, 1883);  
  client.setCallback(callback);
  
  pinMode(D3, OUTPUT);  

  pinMode(D5, OUTPUT);  
  pinMode(D6, OUTPUT);  

  pinMode(D1, OUTPUT);  
  pinMode(D7, OUTPUT);  

  pinMode(D2, INPUT);  

changePhaseAuto=millis();
   
}


void loop() {  
  ConnectToAP(wifi, 2); 
  connectToMQTT(&client, mqttServerIP, TOKEN, callback, NULL);  
  client.subscribe(IN_TOPIC);  
  client.loop();      
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
  //  changePhase(D7, D1);
  }
  prevStateD2=actStateD2;
  if(changePhaseAuto>0 && millis()-changePhaseAuto>2000 && (PHASE==TOTAL_OFF || PHASE==PHASE_2_OFF || PHASE_1_OFF)){
    changePhase();      
    changePhaseAuto=0; 
  }
  if(millis()-sendMqttTime>10000){
    sendToMqtt();
    sendMqttTime=millis();
  }
}
void sendToMqtt(){
  String payload;    
    
    payload = "{"; 
    payload += "\"state\":\"";payload+=PHASE; payload += "\",";
    payload += "\"D1\":\"";payload+=digitalRead(D5); payload += "\",";
    payload += "\"D2\":\"";payload+=digitalRead(D6); payload += "\",";
    payload += "\"D3\":\"";payload+=digitalRead(D1); payload += "\",";    
    payload += "\"D4\":\"";payload+=digitalRead(D7); payload += "\"";    
    payload += "}";
 sendToMqtt(&client,OUT_TOPIC, payload);
}
void changePhase( ){
    Serial.println("Wcisnete");
    
    if(PHASE==TOTAL_OFF || PHASE==PHASE_2_OFF){
      digitalWrite(D5,0);
      digitalWrite(D6,1);
      digitalWrite(D1,0);
      digitalWrite(D7,1);
      changePhaseAuto=millis();
      PHASE=PHASE_1;
      sendToMqtt();
      return;
    }
    if(PHASE==PHASE_1){
      digitalWrite(D5,1);
      digitalWrite(D6,1);
      digitalWrite(D1,1);
      digitalWrite(D7,1);   
      changePhaseAuto=0;
      PHASE=PHASE_1_OFF;
      sendToMqtt();
      return;
    }
    if(PHASE==PHASE_1_OFF){
      digitalWrite(D5,1);
      digitalWrite(D6,0);
      digitalWrite(D1,1);
      digitalWrite(D7,0);
      changePhaseAuto=millis();
      PHASE=PHASE_2;
      sendToMqtt();
      return;
    }
    if(PHASE==PHASE_2){
      digitalWrite(D5,1);
      digitalWrite(D6,1);
      digitalWrite(D1,1);
      digitalWrite(D7,1);      
      changePhaseAuto=0;
      PHASE=PHASE_2_OFF;      
      sendToMqtt();
      return;
    }  
    
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("EEE ");
  Serial.println(topic);
  if(strcmp(topic, IN_TOPIC)==0){    
    changePhase();
  }
  if(strcmp(topic, OUT_TOPIC)==0){    
    sendToMqtt();
  }
}
 
