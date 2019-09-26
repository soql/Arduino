#include "dht_nonblocking.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

#include <soql_tools.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

#define FW_VERSION 20
#define FW_INFO "Kontroller pieca zjc"

wifi_struct wifi[2]={
  {"ZJC-N","820813130882"},
  {"ZJC-W","820813130882"}
};

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.1.168", 0, 60000);

#define TOKEN "ESP8266_OVEN_CONTROLLER"

#define IN_TOPIC "/telemetry/oven/set"
#define OUT_TOPIC "/telemetry/oven/get"

#define SERVO_PORT D2

#define WIFI_COUNT 2
/*DHT22*/
static const int DHT_SENSOR_PIN = D1;

#define DHT_SENSOR_TYPE DHT_TYPE_22

DHT_nonblocking* dht_sensor;

IPAddress mqttServerIP(192,168,1,168);
/*IPAddress mqttServerIP(91,239,168,107);*/

void callback(char* topic, byte* payload, unsigned int length);

float temperature;
float humidity;  

  uint addr = 0;
/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);
Servo termostat;
int termostatValue=9;
void eeWriteInt(int pos, int val) {
    byte* p = (byte*) &val;
    EEPROM.write(pos, *p);
    EEPROM.write(pos + 1, *(p + 1));
    EEPROM.write(pos + 2, *(p + 2));
    EEPROM.write(pos + 3, *(p + 3));
    EEPROM.commit();
}
int eeGetInt(int pos) {
  int val;
  byte* p = (byte*) &val;
  *p        = EEPROM.read(pos);
  *(p + 1)  = EEPROM.read(pos + 1);
  *(p + 2)  = EEPROM.read(pos + 2);
  *(p + 3)  = EEPROM.read(pos + 3);
  return val;
}

void setup() {
 EEPROM.begin(512);
 Serial.begin(115200); 
 delay(1000);   
 ConnectToAP(wifi, WIFI_COUNT);
 connectToMQTT(&client, mqttServerIP, TOKEN, callback, onConnect);
 sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
 checkForUpdates(FW_VERSION);
 dht_sensor= new DHT_nonblocking( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );
 pinMode(SERVO_PORT, OUTPUT);
 delay(1000); 
 termostat.attach(SERVO_PORT);
 delay(1000); 
 termostatValue=eeGetInt(0);
 int toWrite=map(termostatValue, 6, 24, 0, 105);
 
 termostat.write(toWrite);    
 Serial.println("Debug 1");
 
}

void onConnect(PubSubClient* client) {
    client->subscribe(IN_TOPIC);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Cos odebrano");
  Serial.println(topic);
  if(strcmp(topic, IN_TOPIC)==0){    
    String p;
    for(int j=0; j<length; j++){
      p+=(char)payload[j];
    }        
    if(p.equals("RESET")){
      goDeepSleep(1, false);
    }
    if(termostatValue!=p.toInt()){
      termostatValue=p.toInt(); 
       pinMode(SERVO_PORT, OUTPUT);     
      termostat.attach(SERVO_PORT);
      int toWrite=map(termostatValue, 6, 24, 0, 105);
      termostat.write(toWrite);   
      eeWriteInt(0,termostatValue);             
      generateAndSend();
    }
  }
}
 
void loop() {  
  ConnectToAP(wifi, 2);
  connectToMQTT(&client, mqttServerIP, TOKEN, callback, onConnect);
  timeClient.begin();
  timeClient.update();
  client.loop();
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );        
    generateAndSend();
  } 
  
}

void generateAndSend(){  
  String payload = "{";
    payload += "\"temperature\":"; payload += temperature; payload += ",";
    payload += "\"humidity\":"; payload += humidity; payload += ",";
    payload += "\"termostatValue\":"; payload += termostatValue; payload += ",";
    payload += "\"ap\":\""; payload += WiFi.SSID();payload += "\",";
    payload += "\"time\":"; payload += timeClient.getEpochTime();payload += ",";
    payload += "\"rssi\":"; payload += WiFi.RSSI();
    payload += "}";
    sendToMqtt(&client,OUT_TOPIC,payload); 
}

long measurement_timestamp = 0;

static bool measure_environment( float *temperature, float *humidity )
{
  
  
  if( millis( ) - measurement_timestamp > 15000ul )
  {
    if( dht_sensor->measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }

  return( false );
}
