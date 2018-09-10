#include <TFT_eSPI.h>
#include <SPI.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define WIFI_AP "SoqlNet"
#define WIFI_PASSWORD "820813130882"

#define TOKEN "ESP8266_OVEN_DISPLAY_2"

IPAddress mqttServerIP(91,239,168,107);

#define IN_TOPIC "telemetry/flower/inside"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);



void callback(char* topic, byte* payload, unsigned int length) ;

double temperature;
double humidity;

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

  


void connectToMQTT(){
   int d=0;
  while ( !mqttClient.connected() ) {
    Serial.print("Connecting to MQTT Server ...");
    if ( mqttClient.connect(TOKEN) ) {
      Serial.println( "[DONE]" );
      boolean res=mqttClient.subscribe(IN_TOPIC, 1);
      Serial.print("Subscribe: ");
      Serial.println(res);
    }  else {
      d++;
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( mqttClient.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 1000 );
      if(d>=10){
        Serial.println("[ERROR] Cannot connect to MQTT Server");
        ESP.reset();
      }
    }
  }
}

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

long lastChange=0;
long actTime=0;


void setup(void) {
  Serial.begin(115200);
  while (!Serial) continue;
  Serial.println(MQTT_MAX_PACKET_SIZE);
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  mqttClient.setServer( mqttServerIP, 1883);  
  mqttClient.setCallback(callback);
  lastChange=millis();
    
}
double prevTemp;
double prevHumidity;
int whatToShow=0;
double setOnPiec=5.5;
boolean change=true;
void loop() {      
    ConnectToAP(); 
    connectToMQTT();
    mqttClient.loop();    
    char* temp="4125";
         if(change){
      if(whatToShow==0){             
        String temp=String(temperature, 1);       
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);              
        tft.setTextDatum(4);
        tft.drawString(temp,80,64,7);
        String tempOnPiec=String(setOnPiec, 1);     
        tft.setTextColor(TFT_RED, TFT_BLACK);      
        tft.drawString(tempOnPiec,80,110,4);  
      }
      if(whatToShow==1){                         
        String humi=String(humidity, 1);
        humi=humi+"%";
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_BLUE, TFT_BLACK);      
        tft.drawString(humi,80,64,7);    
      } 
         }     
      actTime=millis()-lastChange;  
      Serial.println(actTime);      
      change=false;
          if(whatToShow==0){
            if(actTime>5000){          
              whatToShow=1;
              lastChange=millis();                   
          change=true;                                  
            }
          }
          actTime=millis()-lastChange;  
          if(whatToShow==1){
            if(actTime>1500){          
              whatToShow=0;
              lastChange=millis();                 
              change=true;                                  
            }
          }
          
      prevHumidity=humidity;
      prevTemp=temperature;
     
     
}

void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<300> jsonBuffer;
  Serial.print("Cos odebrano");
  Serial.println(topic);
  Serial.flush();
  if(strcmp(topic, IN_TOPIC)==0){    
    String json;
    for(int j=0; j<length; j++){
      json+=(char)payload[j];
    }
    Serial.print("Odebrano ");
    Serial.print(length);    
    Serial.println(json);    
     DeserializationError error = deserializeJson(jsonBuffer, json);
      if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
    JsonObject root = jsonBuffer.as<JsonObject>();
    temperature = root["temperature"];    
    humidity= root["humidity"];    
    Serial.println("SENSOR");    
    Serial.println(temperature);    
  }  
  
}
 
