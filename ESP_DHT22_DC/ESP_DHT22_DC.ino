#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

struct wifi_struct {
    String ssid;
    String password;
};

wifi_struct wifi[2]={
  {"ZJC-W","820813130882"},
  {"ZJC-N","820813130882"}
};


/*#define WIFI_AP "DWR-116_5E63AE"
#define WIFI_PASSWORD "1438775157"*/

#define TOKEN "ESP8266_DHT22_DC_WEED"

IPAddress mqttServerIP(192,168,1,168);  
/*IPAddress mqttServerIP(79,190,140,82);*/   

struct dhtresults_struct {
    float temperature;
    float humidity;    
};

void setup() {
 Serial.begin(115200);
 delay(10); 
 struct dhtresults_struct dht22=getResultsFromDHT22(); 
 ConnectToAP(); 
 
 String payload = "{";
  payload += "\"temperature\":"; payload += dht22.temperature; payload += ",";
  payload += "\"humidity\":"; payload += dht22.humidity;/* payload += ",";*/
  /*payload += "\"rssi\":"; payload += WiFi.rssi();*/
  payload += "}";
 sendToMQTT(payload);
 goDeepSleep();
}

void goDeepSleep(){
 /*Serial.println("Go into deepsleep mode.");
 ESP.deepSleep(1000000*30);*/
 delay(30000);
 ESP.reset();
}
 
void loop() {
  // put your main code here, to run repeatedly:

}

void ConnectToAP()
{
  for(int i=0; i<sizeof(wifi); i++){
    int retry=0;
    if(WiFi.status() == WL_CONNECTED)
      return;
    Serial.print("Connecting to AP ...");
    // attempt to connect to WiFi network
    
    WiFi.begin(wifi[i].ssid.c_str(), wifi[i].password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      Serial.print(".");
      retry++;
      if(retry>=40){
        break;;
      }
    }
    Serial.println("Connected to AP");
    return;
  }
  goDeepSleep();
}


/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);


void sendToMQTT(String dataToSend){
 ConnectToAP();
  int i=0;
  client.setServer( mqttServerIP, 1883 );  
  Serial.print("Connecting to ThingsBoard node ...");
  while ( !client.connected() ) {
    if ( client.connect(TOKEN) ) {
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
        goDeepSleep();
      }
    }
  }
  char attributes[100];
  dataToSend.toCharArray( attributes, 100 );
  client.publish( "telemetry/flower/inside", attributes );
  Serial.print( "Send to MQTT:" );
  Serial.println( attributes );   
  delay(500);
}

/*DHT22*/
#define DHTPIN 0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

struct dhtresults_struct getResultsFromDHT22(){
  int i=0;
  float t,h;
  struct dhtresults_struct dhtresults;
  while(i<10){
    /*h=2;
    t=2;*/
      h = dht.readHumidity();
      t = dht.readTemperature();
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
  goDeepSleep();
  return dhtresults;
}

