
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
#include "ClosedCube_BME680.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <soql_tools32.h>

#define FW_VERSION 1
#define FW_INFO "Czujnik temperatury w szafce"

#define WIFI_COUNT 3

wifi_struct wifi[WIFI_COUNT] = {   
  {"SoqlNet"  , "820813130882"}  
};

struct dhtresults_struct {
    float temperature;
    float humidity;    
};

#define TOKEN "ESP32_BCE680_WEED_INSIDE"

IPAddress mqttServerIP(192,168,2,2);  

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.2.2");

double temp;
double pres;
double hum;
uint32_t gas;
/*IPAddress mqttServerIP(79,190,140,82);*/   

WiFiClient wifiClient;
PubSubClient client(wifiClient);

ClosedCube_BME680 bme680;

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
 /*int rawValue=readADC();
 double batteryStatus=convertADC(rawValue);*/
 while(!timeClient.update()){
  delay(100);
 }
 delay(2000);
 String payload = "{";
  payload += "\"temperature\":"; payload += temp; payload += ",";
  payload += "\"humidity\":"; payload += hum;payload += ",";
  payload += "\"time\":"; payload += timeClient.getEpochTime();payload += ",";
  payload += "\"pressure\":"; payload += pres;payload += ",";
  payload += "\"gas\":"; payload +=gas;payload += ",";  
  /*payload += "\"battery\":"; payload += batteryStatus;payload += ",";
  payload += "\"rawBattery\":"; payload += rawValue;payload += ",";*/
  payload += "\"rssi\":"; payload += WiFi.RSSI();payload += ",";
  payload += "\"ssid\":\""; payload += WiFi.SSID(); payload += "\"";
  payload += "}";
  Serial.println(payload);
  sendToMqtt(&client, "telemetry/bme680/inside", payload);
  goDeepSleep(30, true);
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
    Wire.begin();
  bme680.init(0x77); // I2C address: 0x76 or 0x77
  bme680.reset();
  
  Serial.print("Chip ID=0x");
  Serial.println(bme680.getChipID(), HEX);

  // oversampling: humidity = x1, temperature = x2, pressure = x16
  bme680.setOversampling(BME680_OVERSAMPLING_X1, BME680_OVERSAMPLING_X2, BME680_OVERSAMPLING_X16);
  bme680.setIIRFilter(BME680_FILTER_3);
  bme680.setGasOn(300, 100); // 300 degree Celsius and 100 milliseconds 

  bme680.setForcedMode();
}

void read() {
  while(1){
  ClosedCube_BME680_Status status = readAndPrintStatus();
  if (status.newDataFlag) {
    Serial.print("result: ");
    double temp = bme680.readTemperature();
    double pres = bme680.readPressure();
    double hum = bme680.readHumidity();

    Serial.print("T=");
    Serial.print(temp);
    Serial.print("C, RH=");
    Serial.print(hum);
    Serial.print("%, P=");
    Serial.print(pres);
    Serial.print("hPa");
      
    uint32_t gas = bme680.readGasResistance();

    Serial.print(", G=");
    Serial.print(gas);
    Serial.print(" Ohms");

    Serial.println();
    return;
  }else {
   delay(200); // sensor data not yet ready
  }
  }
}
 
ClosedCube_BME680_Status readAndPrintStatus() {
  ClosedCube_BME680_Status status = bme680.readStatus();
  return status;
}

void loop(){
}
