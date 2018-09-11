#include "dht_nonblocking.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>


#define WIFI_AP "ZJC-N"
#define WIFI_PASSWORD "820813130882"

/*#define WIFI_AP "SoqlAP"
#define WIFI_PASSWORD "EQDLPNNM"*/

/*#define WIFI_AP "DWR-116_5E63AE"
#define WIFI_PASSWORD "1438775157"*/

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

/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);
Servo termostat;

void setup() {
 termostat.attach(SERVO_PORT);
 Serial.begin(115200);
 delay(10);  
 
 
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Cos odebrano");
  Serial.println(topic);
  if(strcmp(topic, IN_TOPIC)==0){    
    String p;
    for(int j=0; j<length; j++){
      p+=(char)payload[j];
    }
    Serial.print("Odebrano ");
    Serial.print(length);
    Serial.println("b");
    Serial.println(p);    
    Serial.println(p.toInt());  
    termostat.write(p.toInt()); 
  }
}
 
void loop() {  
  ConnectToAP(); 
  client.loop();
  float temperature;
  float humidity;  
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );
     String payload = "{";
    payload += "\"temperature\":"; payload += temperature; payload += ",";
    payload += "\"humidity\":"; payload += humidity; payload += ",";
    payload += "\"rssi\":"; payload += WiFi.RSSI();
    payload += "}";
    sendToMQTT(payload); 
    
  } 
}

void ConnectToAP()
{
  int c=0;
  if(WiFi.status() == WL_CONNECTED)
    return;
  Serial.print("Connecting to AP ...");
  // attempt to connect to WiFi network
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    c++;
    if(c>=40){
      ESP.reset();
    }
  }
  Serial.println("Connected to AP");
}




void sendToMQTT(String dataToSend){
 ConnectToAP();
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
  if( millis( ) - measurement_timestamp > 4000ul )
  {
    if( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }

  return( false );
}

