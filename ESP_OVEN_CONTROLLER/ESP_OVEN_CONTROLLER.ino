#include "dht_nonblocking.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <soql_tools.h>

wifi_struct wifi[2]={
  {"ZJC-N","820813130882"},
  {"ZJC-W","820813130882"}
};


#define TOKEN "ESP8266_OVEN_CONTROLLER"

#define IN_TOPIC "/telemetry/oven/set"
#define OUT_TOPIC "/telemetry/oven/get"

#define SERVO_PORT 0

/*DHT22*/
static const int DHT_SENSOR_PIN = 2;

#define DHT_SENSOR_TYPE DHT_TYPE_22

DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

IPAddress mqttServerIP(192,168,1,168);
/*IPAddress mqttServerIP(91,239,168,107);*/

void callback(char* topic, byte* payload, unsigned int length);

float temperature;
float humidity;  
int termostatValue=6;

/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);
Servo termostat;

void setup() {
 termostat.attach(SERVO_PORT);
 Serial.begin(115200); 
 delay(10);  
 termostat.write(map(termostatValue, 6, 24, 0, 165));    
 
 
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Cos odebrano");
  Serial.println(topic);
  if(strcmp(topic, IN_TOPIC)==0){    
    String p;
    for(int j=0; j<length; j++){
      p+=(char)payload[j];
    }
    termostatValue=p.toInt();  
    termostat.write(map(termostatValue, 6, 24, 0, 165));    
    generateAndSend();
  }
}
 
void loop() {  
  ConnectToAP(wifi, 2);
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
    payload += "\"rssi\":"; payload += WiFi.RSSI();
    payload += "}";
    sendToMQTT(payload); 
}
void sendToMQTT(String dataToSend){
ConnectToAP(wifi, 2);
  int d=0;
  
  while ( !client.connected() ) {
    client.setServer( mqttServerIP, 1883);  
    client.setCallback(callback);
    Serial.print("Connecting to MQTT Server ...");
    if ( client.connect(TOKEN) ) {
      Serial.println( "[DONE]" );
      client.subscribe(IN_TOPIC);
    } else {
      d++;
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 1000 );
      if(d>=10){
        Serial.println("[ERROR] Cannot connect to MQTT Server");
        ESP.reset();
      }
    }
  }
  char attributes[100];
  dataToSend.toCharArray( attributes, 100 );
  client.publish(OUT_TOPIC, attributes );
  Serial.print( "Send to MQTT:" );
  Serial.println( attributes );   
  delay(500);
}
long measurement_timestamp = 0;

static bool measure_environment( float *temperature, float *humidity )
{
  

  /* Measure once every four seconds. */
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

