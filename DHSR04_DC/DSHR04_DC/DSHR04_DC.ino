#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <soql_tools.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "DHT.h"

struct dhtresults_struct {
    float temperature;
    float humidity;    
};

/*DHT22*/
#define DHTPIN D6
#define DHTTYPE DHT22

#define FW_VERSION 8
#define FW_INFO "Kontroller poziomu peletu"

#define WIFI_COUNT 3

wifi_struct wifi[WIFI_COUNT] = {   
  {"TP-LINK_A3D1EC"  , "1438775157"},
  {"DWR-116_5E63AE" , "1438775157"},
  {"ZJC-N"          , "820813130882"},
};


#define TOKEN "WEMOS_D1_LITE_OVEN_MEAS"

#define echoPin D1 // Echo Pin
#define trigPin D2 // Trigger Pin


long duration, distance; // Duration used to calculate distance

WiFiClient wifiClient;
PubSubClient client(wifiClient);

IPAddress mqttServerIP(192,168,2,3);

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.2.3", 0, 60000);

void goDeepSleep(int timeInSeconds){
  ESP.deepSleep(timeInSeconds*1000000);
}

void isort(int *a, int n)
{
 for (int i = 1; i < n; ++i)
 {
   int j = a[i];
   int k;
   for (k = i - 1; (k >= 0) && (j < a[k]); k--)
   {
     a[k + 1] = a[k];
   }
   a[k + 1] = j;
 }
}

void setup() {
    initSerial(115200);
    ConnectToAP(wifi, WIFI_COUNT);  
    checkForUpdates(FW_VERSION);
    connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
    sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
  
   pinMode(trigPin, OUTPUT);
   pinMode(echoPin, INPUT);
   timeClient.begin();
   while(!timeClient.update()){
      delay(100);
   }
   struct dhtresults_struct dht22=getResultsFromDHT22();  
   int sum=0;
   /*int results[20];

    
    
    for(int i=0; i<20; i++){
      results[i]=getDistance();
      delay(1000);
    }
    isort(results, 20);
    ;
    for(int i=5; i<15; i++){
      sum+=results[i];
    }*/

      
    String payload = "{";
    payload += "\"temperature\":"; payload += dht22.temperature; payload += ",";
    payload += "\"humidity\":"; payload += dht22.humidity; payload += ",";  
    payload += "\"distance\":"; payload += sum/10; payload += ",";    
    payload += "\"time\":"; payload += timeClient.getEpochTime(); payload += ",";
    payload += "\"rssi\":"; payload += WiFi.RSSI();
    payload += "}";
    sendToMqtt(&client,"/telemetry/boiler/oven", payload);   
    delay(1000);
    goDeepSleep(30);
}

void loop() {
    ConnectToAP(wifi, WIFI_COUNT);
    connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);  
    timeClient.update();

    int results[20];

    
    
    for(int i=0; i<20; i++){
      results[i]=getDistance();
      delay(1000);
    }
    isort(results, 20);

    int sum=0;
    for(int i=5; i<15; i++){
      sum+=results[i];
    }

    
  
    String payload = "{";
    payload += "\"distance\":"; payload += sum/10; payload += ",";    
    payload += "\"time\":"; payload += timeClient.getFormattedTime(); payload += ",";
    payload += "\"rssi\":"; payload += WiFi.RSSI();
    payload += "}";
    sendToMqtt(&client,"/telemetry/boiler/oven", payload);   
    delay(1000);
    goDeepSleep(60); 
}

long getDistance(){
    /* The following trigPin/echoPin cycle is used to determine the
  distance of the nearest object by bouncing soundwaves off of it. */
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration/58.2;
  Serial.println(distance);
  return distance;
  
}

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
 goDeepSleep(30,true);
  return dhtresults;
}



