/*#include "weedroom.h"*/
/*#include "bathroom_floor.h"*/
#include "przem_kotlownia.h"
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <soql_tools.h>
#include <NTPClient.h>
#include <WiFiUdp.h>



struct dhtresults_struct {
    float temperature;
    float humidity;    
};

/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);



void setup() {
 Serial.begin(115200);
 delay(10); 
 ConnectToAP(wifi, WIFI_COUNT);
 checkForUpdates(FW_VERSION);
 connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
 sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
 timeClient.begin();
 
 while(!timeClient.update()){
  delay(100);
 }
 
 struct dhtresults_struct dht22=getResultsFromDHT22();  
 
 String payload = "{";
  payload += "\"temperature\":"; payload += dht22.temperature; payload += ",";
  payload += "\"humidity\":"; payload += dht22.humidity; payload += ",";
  payload += "\"time\":"; payload += timeClient.getEpochTime(); payload += ",";
  payload += "\"rssi\":"; payload += WiFi.RSSI(); payload += ",";
  payload += "\"ssid\":\""; payload += WiFi.SSID(); payload += "\"";
  
  payload += "}";
 sendToMqtt(&client,OUT_TOPIC, payload);
 goDeepSleep(30,real_deepsleep);
}

 
void loop() {
  setup();

}


DHT dht(DHTPIN, DHTTYPE);

struct dhtresults_struct getResultsFromDHT22(){
  int i=0;
  float t,h;
  struct dhtresults_struct dhtresults;
  while(i<10){
    /*h=2;
    t=2;*/
      h = dht.readHumidity();
      t = dht.readTemperature();
      if (isnan(h) || isnan(t)) {
        i++;
        Serial.println("Failed to read from DHT sensor!");      
        delay(500);
      }else{
        dhtresults.temperature=t;
        dhtresults.humidity=h;
        Serial.print("DHT22 Results. Humidity: ");      
        Serial.print(h);      
        Serial.print(". Temperature: ");              
        Serial.print(t);      
        Serial.println(" *C ");      
        return dhtresults;
      }
  }
  goDeepSleep(30,real_deepsleep);
  return dhtresults;
}
