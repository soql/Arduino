/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor
  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660
  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!
  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#define BME680_VER

#include <Wire.h>
#ifdef BME680_VER
#include <SPI.h>
#include "Adafruit_BME280.h"
#endif
#include <Adafruit_Sensor.h>

#include <WiFi.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <soql_tools32.h>


//#define DHT22_VER

#ifdef BME680_VER
#define SEALEVELPRESSURE_HPA (1013.25)
#endif

#define FW_VERSION 4
#define FW_INFO "Czujnik temperatury w szafce"

#define WIFI_COUNT 4

wifi_struct wifi[WIFI_COUNT] = {     
  {"ZJC-N"  , "820813130882"},
  {"ZJC-W","820813130882"},
  {"ZJCCRYPTO","820813130882"},
};

#define TOKEN "ESP32_BCE680_WEED_INS3"

IPAddress mqttServerIP(192,168,1,168);  

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.1.168");

struct dhtresults_struct {
    float temperature;
    float humidity;    
};


WiFiClient wifiClient;
PubSubClient client(wifiClient);

#ifdef DHT22_VER
#define DHTPIN 21
#define DHTTYPE DHT22
#endif

#ifdef BME680_VER
Adafruit_BME280 bme; // I2C
#endif

void setup() { 
 initSerial(115200);
 pinMode(33,INPUT_PULLUP);
 pinMode(32,INPUT_PULLUP);
 int analogValue1=0;
 int analogValue2=0;
 int analogValue1sum=0;
 int analogValue2sum=0;
 for(int i=0; i<5; i++){
  analogValue1=readADC(33);
  analogValue2=readADC(32);
  analogValue1sum+=analogValue1;
  analogValue2sum+=analogValue2;
  
  Serial.println(analogValue1);
  Serial.println(analogValue2);
  delay(50);
 }
 
 ConnectToAP(wifi, WIFI_COUNT);  
 checkForUpdates(FW_VERSION);
 timeClient.begin();
 connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
 sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
 int i=0;

 
 while(!timeClient.update()){
  i++;
  if(i>20)
    break;
  Serial.println("TIME NOT UPDATE");
  delay(100);
 }
#ifdef BME680_VER
 boolean bmeOk=read();
 if(bmeOk){
  Serial.println("BME OK");
  delay(1000);
  read();
 }else{
  Serial.println("BME NOT OK");
 }
 #endif
 
  
 /*while(!timeClient.update()){
  delay(100);
 } */

#ifdef DHT22_VER
pinMode(DHTPIN, INPUT);
 struct dhtresults_struct dht22=getResultsFromDHT22();
#endif
String payload;
 #ifdef BME680_VER
 if(bmeOk){
    payload = "{"; 
    payload += "\"temperature\":"; payload += bme.readTemperature(); payload += ",";
    payload += "\"humidity\":"; payload += bme.readHumidity();payload += ",";
    payload += "\"time\":"; payload += timeClient.getEpochTime();payload += ",";
    payload += "\"pressure\":"; /*payload += (bme.pressure / 100.0);*/payload += "0,";
    payload += "\"gas\":"; /*payload += (bme.gas_resistance / 1000.0);*/payload += "0,";
    payload += "\"altitude\":"; /*payload += bme.readAltitude(SEALEVELPRESSURE_HPA);*/payload += "0,"; 
    
/*    payload += "\"pressure\":"; payload += (bme.pressure / 100.0);payload += ",";
    payload += "\"gas\":"; payload += (bme.gas_resistance / 1000.0);payload += ",";
    payload += "\"altitude\":"; payload += bme.readAltitude(SEALEVELPRESSURE_HPA);payload += "0,"; */
    payload += "\"rssi\":"; payload += WiFi.RSSI();payload += ",";
    payload += "\"ssid\":\""; payload += WiFi.SSID(); payload += "\"";
    payload += "}";
    Serial.println(payload);
    sendToMqtt(&client, "telemetry/bme680/inside", payload);
 }
#endif
#ifdef DHT22_VER
if(dht22.temperature!=-999){
  payload = "{"; 
   /* payload += "\"temperature\":"; payload += bme.temperature; payload += ",";
    payload += "\"humidity\":"; payload += bme.humidity;payload += ",";*/
    payload += "\"temperature\":"; payload += dht22.temperature; payload += ",";
    payload += "\"humidity\":"; payload += dht22.humidity; payload += ",";
    payload += "\"time\":"; payload += timeClient.getEpochTime();payload += ",";
    payload += "\"pressure\":"; /*payload += (bme.pressure / 100.0);*/payload += "0,";
    payload += "\"gas\":"; /*payload += (bme.gas_resistance / 1000.0);*/payload += "0,";
    payload += "\"altitude\":"; /*payload += bme.readAltitude(SEALEVELPRESSURE_HPA);*/payload += "0,"; 
    payload += "\"rssi\":"; payload += WiFi.RSSI();payload += ",";
    payload += "\"ssid\":\""; payload += WiFi.SSID(); payload += "\"";
    payload += "}";
    Serial.println(payload);
    sendToMqtt(&client, "telemetry/bme680/inside", payload);
}
#endif

/* int analogValue1;
 int analogValue2;
 for(int i=0; i<5; i++){
  analogValue1=readADC(12);
  analogValue2=readADC(13);
  Serial.println(analogValue1);
  Serial.println(analogValue2);
  delay(20);
 }*/
  payload = "{"; 
    payload += "\"soilValue1\":"; payload += (analogValue1sum/5); payload += ",";
    payload += "\"soilValue2\":"; payload += (analogValue2sum/5); payload += ",";
    payload += "\"time\":"; payload += timeClient.getEpochTime(); payload += ",";
    payload += "\"rssi\":"; payload += WiFi.RSSI();payload += ",";
    payload += "\"ssid\":\""; payload += WiFi.SSID(); payload += "\"";    
    payload += "}";
    Serial.println(payload);
    sendToMqtt(&client, "telemetry/soil/inside", payload);
    delay(3000);
 goDeepSleep(30, true);
}

/*Read ADC*/
double readADC(int pin){
  int rawValue=analogRead(pin);
  return rawValue;
}

#ifdef BME680_VER
boolean initBme(){
 if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    return false;
  }  
  
 /* bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); */
  return true;
}

boolean read() {
  int i=0;
  while(true){
  if (!bme.begin(0x76)) {      
      delay(1000);
      Serial.println("Failed to perform reading :(");    
      i++;
    }else{
      return true;
    }
    if(i>10){      
      return false;
    }
    }
    return false;
  }
#endif
#ifdef DHT22_VER
DHT dht(DHTPIN, DHTTYPE);

struct dhtresults_struct getResultsFromDHT22(){
  dht.begin();
  int i=0;
  float t,h;
  struct dhtresults_struct dhtresults;
  while(i<30){
    /*h=2;
    t=2;*/
      h = dht.readHumidity();
      t = dht.readTemperature();
      if (isnan(h) || isnan(t)) {
        i++;
        Serial.println("Failed to read from DHT sensor!");      
        delay(1000);
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
  dhtresults.temperature=-999;
  dhtresults.humidity=-999;
  return dhtresults;
}
#endif
void loop(){
}
