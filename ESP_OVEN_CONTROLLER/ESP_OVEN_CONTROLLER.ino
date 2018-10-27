#include "dht_nonblocking.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <soql_tools.h>

#define FW_VERSION 7
#define FW_INFO "Kontroller pieca zjc"

wifi_struct wifi[2]={
  {"ZJC-N","820813130882"},
  {"ZJC-W","820813130882"}
};


#define TOKEN "ESP8266_OVEN_CONTROLLER"

#define IN_TOPIC "/telemetry/oven/set"
#define OUT_TOPIC "/telemetry/oven/get"

#define SERVO_PORT D3

#define WIFI_COUNT 2
/*DHT22*/
static const int DHT_SENSOR_PIN = D4;

#define DHT_SENSOR_TYPE DHT_TYPE_22

DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

IPAddress mqttServerIP(192,168,1,168);
/*IPAddress mqttServerIP(91,239,168,107);*/

void callback(char* topic, byte* payload, unsigned int length);

float temperature;
float humidity;  
int termostatValue=8;

/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);
Servo termostat;

void setup() {
 Serial.begin(115200); 
 delay(1000);  
 
 ConnectToAP(wifi, WIFI_COUNT);
 connectToMQTT(&client, mqttServerIP, TOKEN, callback, onConnect);
 sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
 checkForUpdates(FW_VERSION);
 pinMode(SERVO_PORT, OUTPUT);
 delay(1000); 
 termostat.attach(SERVO_PORT);
 delay(1000); 
 int toWrite=map(termostatValue, 6, 24, 0, 165);
 termostat.write(toWrite);    
 
 
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
    if(termostatValue!=p.toInt()){
      termostatValue=p.toInt();      
      termostat.attach(SERVO_PORT);
      int toWrite=map(termostatValue, 6, 24, 0, 165);
      termostat.write(toWrite);    
      generateAndSend();
    }
  }
}
 
void loop() {  
  ConnectToAP(wifi, 2);
  connectToMQTT(&client, mqttServerIP, TOKEN, callback, onConnect);
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
    payload += "\"rssi\":"; payload += WiFi.RSSI();
    payload += "}";
    sendToMqtt(&client,OUT_TOPIC,payload); 
}

long measurement_timestamp = 0;

static bool measure_environment( float *temperature, float *humidity )
{
  
  
  if( millis( ) - measurement_timestamp > 15000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }

  return( false );
}
