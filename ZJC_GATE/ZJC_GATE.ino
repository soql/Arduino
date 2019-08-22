#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <soql_tools.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


#define FW_VERSION 1
#define FW_INFO "Kontroler bramy - zalesie"
#define BW
#ifdef ZAL
wifi_struct wifi[3]={  
  {"ZJC-N","820813130882"},
  {"ZJCCRYPTO","820813130882"}
};
#endif
#ifdef BW
wifi_struct wifi[3]={  
  {"SoqlNet","820813130882"}
};
#endif



#define TOKEN "ESP8266_ZJC_GATE"

#ifdef ZAL
IPAddress mqttServerIP(192,168,1,168);  
#endif
#ifdef BW
IPAddress mqttServerIP(192,168,2,2);  
#endif

#define OUT_TOPIC "telemetry/gate"

void setup() {
 Serial.begin(115200);
 delay(10); 
 ConnectToAP(wifi, WIFI_COUNT);
 checkForUpdates(FW_VERSION);
 connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
 sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
  
}
int d8state;
int d7state;
int d8prev_state;
int d7prev_state;
void loop() {
  d7prev_state=d7state;
  d8prev_state=d8state;
  
  d8state=digitalRead(D8);
  d7state=digitalRead(D7);
  
  //  Serial.println(d7state);

  if(d7state!=d7prev_state){
    Serial.print("Zmiana D7 z ");
    Serial.print(d7prev_state);
    Serial.print(" na ");
    Serial.println(d7state);
  }
  if(d8state!=d8prev_state){
    Serial.print("Zmiana D8 z ");
    Serial.print(d8prev_state);
    Serial.print(" na ");
    Serial.println(d8state);
  }
 
  
}
