#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <soql_tools.h>
#define WIFI_AP "ZJC-N"
#define WIFI_PASSWORD "820813130882"

#define FW_VERSION 5
#define FW_INFO "Czujniki ruchu - wjazd zalesie"

#define WIFI_COUNT 2

wifi_struct wifi[WIFI_COUNT] = {    
  {"ZJC-N"          , "820813130882"},
  {"ZJC-S"          , "820813130882"}
};


#define TOKEN "WEMOS_D1_PRO_MOVE_TRIGGER"


IPAddress mqttServerIP(192,168,1,168);  
/*IPAddress mqttServerIP(79,190,140,82);*/   
WiFiClient wifiClient;


/*MQTT**/
PubSubClient client(wifiClient);

void setup() {
   delay(10);  
   pinMode(D5, INPUT);
   pinMode(D7, INPUT);
   
   pinMode(D8, OUTPUT);
   pinMode(D0, OUTPUT);
   
   digitalWrite (D5, LOW);
   digitalWrite (D7, LOW);
   
   digitalWrite (D8, LOW);
   digitalWrite (D0, LOW);
   initSerial(115200);
   ConnectToAP(wifi, WIFI_COUNT);  
   checkForUpdates(FW_VERSION);
   connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
   sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));

}
int prevState=0;
int prevState2=0;
void loop() {
  ConnectToAP(wifi, WIFI_COUNT);
  connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
   pinMode(D5, INPUT);
  
   int actState=digitalRead(D5);   
   int actState2=digitalRead(D7);   
   if(prevState==0 && actState==1){
    Serial.println("Wykryto ruch");
    String payload = "{sensor: 1, state: on}"  ;      
    prevState=actState;    
    sendToMqtt(&client,"/telemetry/movment" ,payload);     
    if(actState2==1){
      Serial.println("Wyjazd");
    String payload = "{wyjazd}"  ;          
    digitalWrite (D8, HIGH);
    sendToMqtt(&client,"/telemetry/movment" ,payload);     
    }
    
   }
   if(prevState==1 && actState==0){
    Serial.println("Koniec sygnału");
    String payload = "{sensor: 1, state: off}"    ;    
    prevState=actState;
    digitalWrite (D8, LOW);
    sendToMqtt(&client,"/telemetry/movment" ,payload);     
   }
  if(prevState2==0 && actState2==1){
    Serial.println("Wykryto ruch 2");
    String payload = "{sensor: 2, state: on}"  ;      
    prevState2=actState2;    
    sendToMqtt(&client,"/telemetry/movment" ,payload);     
    if(actState==1){
      Serial.println("Wjazd");
    String payload = "{wjazd}"  ;          
    digitalWrite (D0, HIGH);
    sendToMqtt(&client,"/telemetry/movment" ,payload);     
    }
    
   }
   
   if(prevState2==1 && actState2==0){
    Serial.println("Koniec sygnału 2");
    String payload = "{sensor: 2, state: off}"    ;    
    prevState2=actState2;
    digitalWrite (D0, LOW);
    sendToMqtt(&client,"/telemetry/movment" ,payload);     
   }
   
  }
   if(prevState==0 && actState==1 && actState2==1){    
    String payload = "{wyjazd}"  ;          
    digitalWrite (D8, HIGH);
    sendToMqtt(&client,"/telemetry/movment" ,payload);     
    
   }
     
}




