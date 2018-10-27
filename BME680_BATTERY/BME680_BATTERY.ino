
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

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <soql_tools.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)


#define FW_VERSION 6
#define FW_INFO "Czujnik pogodowy na zewnątrz"

#define WIFI_COUNT 3

wifi_struct wifi[WIFI_COUNT] = {   
  {"TP-LINK_A3D1EC"  , "1438775157"},
  {"DWR-116_5E63AE" , "1438775157"},
  {"ZJC-N"          , "820813130882"},
};

#define TOKEN "ESP8266_BCE680_OUTSIDE"

IPAddress mqttServerIP(192,168,2,3);  

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, mqttServerIP);


/*IPAddress mqttServerIP(79,190,140,82);*/   

WiFiClient wifiClient;
PubSubClient client(wifiClient);

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

void setup() {
 initSerial(115200);
 ConnectToAP(wifi, WIFI_COUNT);  
 checkForUpdates(FW_VERSION);
 timeClient.begin();
 connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
 sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));

 initBme();
 delay(1000);
 read();
 int rawValue=readADC();
 double batteryStatus=convertADC(rawValue);
 timeClient.update();
 delay(2000);
 String payload = "{";
  payload += "\"temperature\":"; payload += bme.temperature; payload += ",";
  payload += "\"humidity\":"; payload += bme.humidity;payload += ",";
  payload += "\"time\":"; payload += timeClient.getEpochTime();payload += ",";
  payload += "\"pressure\":"; payload += (bme.pressure / 100.0);payload += ",";
  payload += "\"gas\":"; payload += (bme.gas_resistance / 1000.0);payload += ",";
  payload += "\"altitude\":"; payload += bme.readAltitude(SEALEVELPRESSURE_HPA);payload += ",";
  payload += "\"battery\":"; payload += batteryStatus;payload += ",";
  payload += "\"rawBattery\":"; payload += rawValue;payload += ",";
  payload += "\"rssi\":"; payload += WiFi.RSSI();payload += ",";
  payload += "\"ssid\":\""; payload += WiFi.SSID(); payload += "\"";
  payload += "}";
  Serial.println(payload);
  sendToMqtt(&client, "telemetry/bme680/outside", payload);
  goDeepSleep(600, true);
}

double convertADC(int value){
  int rawValue=value;
  /*Dzielnik 200/268*/
  int MIN_VALUE=620;
  int MAX_VALUE=758;
  double toReturn=(((double)(rawValue))-MIN_VALUE)/(MAX_VALUE-MIN_VALUE)*100;
  if(toReturn<0){
    toReturn=0;
  }
  if(toReturn>100){
    toReturn=100;
  }
  return toReturn;
}
/*Read ADC*/
double readADC(){
  int rawValue=analogRead(A0);
  return rawValue;
}


void initBme(){
 if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    goDeepSleep(30, true);
  }  
   // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void read() {
  int i=0;
  while(true){
    if (! bme.performReading()) {
      delay(1000);
      Serial.println("Failed to perform reading :(");    
      i++;
    }else{
      return;
    }
    if(i>10){
      goDeepSleep(30, true);
      return;
    }
    }
    
  }
 


void loop(){
}

