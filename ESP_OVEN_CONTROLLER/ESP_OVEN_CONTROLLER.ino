#include "DHT.h"
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
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

IPAddress mqttServerIP(192,168,1,168);
/*IPAddress mqttServerIP(91,239,168,107);*/

void callback(char* topic, byte* payload, unsigned int length);

/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);
Servo termostat;

int i=0;

struct dhtresults_struct {
    float temperature;
    float humidity;    
};

void setup() {
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
  termostat.attach(SERVO_PORT);
  ConnectToAP(); 
  client.loop();
  i++;
  if(i>300000){
    i=0;
    struct dhtresults_struct dht22=getResultsFromDHT22(); 
    String payload = "{";
    payload += "\"temperature\":"; payload += dht22.temperature; payload += ",";
    payload += "\"humidity\":"; payload += dht22.humidity; payload += ",";
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
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
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



struct dhtresults_struct getResultsFromDHT22(){
  int i=0;
  float t,h;
  struct dhtresults_struct dhtresults;
  while(i<10){
    h=2;
    t=2;
      /*h = dht.readHumidity();
      t = dht.readTemperature();*/
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
  ESP.reset();  
  return dhtresults;
}


