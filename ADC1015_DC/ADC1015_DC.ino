#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <soql_tools.h>

#define FW_VERSION 2
#define FW_INFO "Kontroller wilgotności gleby"


#define WIFI_COUNT 3

wifi_struct wifi[WIFI_COUNT] = { 
  {"ZJC-W", "820813130882"},
  {"ZJC-N", "820813130882"},
  {"ZJCCRYPTO", "820813130882"},
};

/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define TOKEN "ESP8266_DHT_ADC_WEED"

IPAddress mqttServerIP(192,168,1,168);  


struct ads_result {
    int adc0;
    int adc1;
    int adc2;
    int adc3;
};

void setup() {
initSerial(115200);
  ConnectToAP(wifi, WIFI_COUNT);  
  checkForUpdates(FW_VERSION);
  connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
  sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
  
  Wire.pins(0, 2);
  Wire.begin(0, 2);
  byte error, address;
  int nDevices;
  nDevices = 0;
  delay(10);  
  struct ads_result ads=readADS(); 
 
 String payload = "{";
  payload += "\"adc0\":"; payload +=ads.adc0; payload += ",";
  payload += "\"adc1\":"; payload +=ads.adc1; payload += ",";
  payload += "\"adc2\":"; payload +=ads.adc2; payload += ",";
  payload += "\"adc3\":"; payload +=ads.adc3; payload += ",";
  payload += "\"rssi\":"; payload += WiFi.RSSI(); payload += ",";
  payload += "\"ssid\":\""; payload += WiFi.SSID(); payload += "\"";
  payload += "}";
 sendToMqtt(&client,"telemetry/flower/soil",payload);
 goDeepSleep(false, 30);
}



Adafruit_ADS1015 ads; 



struct ads_result readADS(){
  ads.setGain(GAIN_ONE);
  ads.begin();  
  struct ads_result adsresults;  

  adsresults.adc0 = ads.readADC_SingleEnded(0);
  adsresults.adc1 = ads.readADC_SingleEnded(1);
  adsresults.adc2 = ads.readADC_SingleEnded(2);
  adsresults.adc3 = ads.readADC_SingleEnded(3);
    

   Serial.print("AIN0: "); Serial.println(adsresults.adc0);
   Serial.print("AIN1: "); Serial.println(adsresults.adc1);
   Serial.print("AIN2: "); Serial.println(adsresults.adc2);
   Serial.print("AIN3: "); Serial.println(adsresults.adc3);
   Serial.println(" ");
   return adsresults;
}

void loop() {
  // put your main code here, to run repeatedly:

}




