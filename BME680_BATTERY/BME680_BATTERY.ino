
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
#include <WiFiUdp.h>
#include <PubSubClient.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

/*#define WIFI_AP "ZJC-N"
#define WIFI_PASSWORD "820813130882"*/
/*#define WIFI_AP "DWR-116_5E63AE"*/
#define WIFI_AP "TP-LINK_A3D1EC"
#define WIFI_PASSWORD "1438775157"

#define TOKEN "ESP8266_BCE680_2"

IPAddress timeServerIP(192,168,2,3);   
IPAddress mqttServerIP(192,168,2,3);
/*IPAddress timeServerIP(79,190,140,82); 
IPAddress mqttServerIP(79,190,140,82);   */

int deepSleepSeconds=600*1000000;
;

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

String topicOk="telemetry/bme680/outside";
String topicError="telemetry/errors";

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(D0, WAKEUP_PULLUP);
  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }  

 initBme();
  
  read();
   int rawValue=readADC();
   double batteryStatus=convertADC(rawValue);
  ConnectToAP();
  uint32_t actualTime=getTimeFromNTP(timeServerIP);  

 String payload = "{";
  payload += "\"temperature\":"; payload += bme.temperature; payload += ",";
  payload += "\"humidity\":"; payload += bme.humidity;payload += ",";
  payload += "\"time\":"; payload += actualTime;payload += ",";
  payload += "\"pressure\":"; payload += (bme.pressure / 100.0);payload += ",";
  payload += "\"gas\":"; payload += (bme.gas_resistance / 1000.0);payload += ",";
  payload += "\"altitude\":"; payload += bme.readAltitude(SEALEVELPRESSURE_HPA);payload += ",";
  payload += "\"battery\":"; payload += batteryStatus;payload += ",";
  payload += "\"rawBattery\":"; payload += rawValue;;payload += ",";
  payload += "\"rssi\":"; payload += WiFi.RSSI();
  payload += "}";
  Serial.println(payload);
  sendToMQTT(payload, topicOk);
  goDeepSleep(NULL, deepSleepSeconds);
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

void goDeepSleep(char* error, int deepSleepSeconds){
  if(error!=NULL){
    sendToMQTT(error, topicError);
    delay(1000);
  }
 Serial.println("Go into deepsleep mode.");
 ESP.deepSleep(deepSleepSeconds);
}

/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void sendToMQTT(String dataToSend, String topic){
 ConnectToAP();
  int i=0;
  client.setServer( mqttServerIP, 1883 );  
  Serial.print("Connecting to ThingsBoard node ...");
  while ( !client.connected() ) {
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      i++;
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 1000 );
      if(i>=10){
        Serial.println("[ERROR] Cannot connect to MQTT Server");
        goDeepSleep(NULL, 1000000);
      }
    }
  }
  char attributes[300];
  dataToSend.toCharArray( attributes, 300 );
  int res=client.publish( "telemetry/bme680/outside", attributes );
  Serial.print( "Send to MQTT:" );
  Serial.print(topic);
  Serial.println( attributes );   
  Serial.print("RESULT=");
  Serial.println(res);
  delay(500);
}

/*Getting time from NTP Server*/
uint32_t getTimeFromNTP(IPAddress timeServerIP){
  ConnectToAP();
  startUDP();  
  
  uint32_t time = 0;
  int i=0;
  while(i<20 && time==0){
    sendNTPpacket(timeServerIP); 
    delay(500);
    Serial.print("Geting time from NTP. Retry nr: ");
    Serial.println(i+1);
    time = getTime();      
    
    i++;
  }
  if(time==0){
    Serial.println("[ERROR] Cannot get time from NTP Server.");
    goDeepSleep("Cannot read time", 1000000);
  }
  Serial.print("Time: ");
  Serial.println(time); 
  return time;
  
  
}

const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message

byte NTPBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

WiFiUDP UDP; 

void startUDP() {
  Serial.println("Starting UDP");
  UDP.begin(123);
  Serial.print("Local port:\t");
  Serial.println(UDP.localPort());
  Serial.println();
  delay(500);
}

uint32_t getTime() {
  if (UDP.parsePacket() == 0) { // If there's no response (yet)
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}

void sendNTPpacket(IPAddress& address) {
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

void initBme(){
   // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void ConnectToAP()
{
  int i=0;
  if(WiFi.status() == WL_CONNECTED)
    return;
  Serial.print("Connecting to AP ...");
  // attempt to connect to WiFi network
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    i++;
    if(i>=100){
      goDeepSleep(NULL, 1000000);
    }
  }
  Serial.println("Connected to AP");
}


void read() {
  int i=0;
  while(true){
    if (! bme.performReading()) {
      Serial.println("Failed to perform reading :(");    
      i++;
    }else{
      return;
    }
    if(i>10){
      goDeepSleep("no read from bme680", 1000000);
      return;
    }
    }
    
  }
 


void loop(){
}

