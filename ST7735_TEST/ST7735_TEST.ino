#include <TFT_eSPI.h>
#include <SPI.h>
#include <stdlib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WString.h>
#include <soql_tools.h>

#define TOKEN "ESP8266_OVEN_DISPLAY_2"

IPAddress mqttServerIP(192,168,1,168);

#define IN_TOPIC "/telemetry/oven/get"
#define OUT_TOPIC "/telemetry/oven/set"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

wifi_struct wifi[2]={  
  {"ZJC-N","820813130882"},
  {"SoqlNet","820813130882"}
};


void callback(char* topic, byte* payload, unsigned int length) ;

double temperature;
double humidity;
double setOnPiec;
  
long lastChange=0;

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
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);  
}

void showTemperature(){
    String temp=String(temperature, 1);       
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);              
    tft.setTextDatum(4);
    tft.drawString(temp,80,54,7);
}

void showHumidity(){
   String humi=String(humidity, 1);        
   tft.fillScreen(TFT_BLACK);
   tft.setTextColor(TFT_BLUE, TFT_BLACK);      
   tft.drawString(humi,80,54,7);
}

void showTermostat(){
   String tempOnPiec=String(setOnPiec, 1);     
   tft.setTextColor(TFT_RED, TFT_BLACK);      
   tft.drawString(tempOnPiec,80,110,4);  
}

void showAllTermostat(){
   String tempOnPiec=String(setOnPiec, 1);       
   tft.fillScreen(TFT_BLACK);
   tft.setTextColor(TFT_RED, TFT_BLACK);              
   tft.setTextDatum(4);
   tft.drawString(tempOnPiec,80,54,7);
}

int whatToShow=0;
int prevStateD1;
int prevStateD2;
long activeD1Time=0;
long inactiveD1Time=0;

boolean change=true;
void loop() {      
    ConnectToAP(wifi, 2); 
    connectToMQTT();
    mqttClient.loop();        
    if(change){
      if(whatToShow==0){             
        showTemperature();
        showTermostat();
      }
      if(whatToShow==1){                         
       showHumidity();  
       showTermostat();  
      } 
     if(whatToShow==2){                              
       showAllTermostat();  
      } 
      change=false;
    }         

     int actStateD1=digitalRead(D1);   
     int actStateD2=digitalRead(D2);   
    
     if((actStateD1==1 && prevStateD1==0) || (actStateD2==1 && prevStateD2==0)){      
        inactiveD1Time=0;
        activeD1Time=millis();      
     }     
     if(actStateD1==0 && prevStateD1==1 || (actStateD2==0 && prevStateD2==1)){      
        activeD1Time=0;
        inactiveD1Time=millis();
     }  
     prevStateD1=actStateD1;
     prevStateD2=actStateD2;
     
     if((actStateD1==1 || actStateD2==1) && millis()-activeD1Time>150 && whatToShow!=2){
            whatToShow=2;
            inactiveD1Time=0;
            activeD1Time=millis();  
            change=true;              
            return;         
     }
     if((actStateD1==1 || actStateD2) && millis()-activeD1Time>150 && whatToShow==2){
        if(actStateD1==1){
            setOnPiec=setOnPiec+1;
        }
        if(actStateD2==1){
            setOnPiec=setOnPiec-1;
        }
            activeD1Time=millis();
            change=true;   
            return;         
     }
     if(actStateD1==0 && actStateD2==0 && millis()-inactiveD1Time>2000 && whatToShow==2){
        whatToShow=0;
        inactiveD1Time=0;
        activeD1Time=0;
        change=true;
        char TempString[10];
        sendToMQTT(String(setOnPiec));
        return;
     }
    if(activeD1Time!=0 || inactiveD1Time!=0){      
      return;
    }
    long actTime=millis()-lastChange;                           
    
    if(whatToShow==0){
      if(actTime>5000){          
        whatToShow=1;
        lastChange=millis();                   
        change=true;        
        return;                          
      }
    }       
   
    if(whatToShow==1){
      if(actTime>1500){          
        whatToShow=0;
        lastChange=millis();                 
        change=true;                                  
        return;
      }
    }
     
}

void sendToMQTT(String dataToSend){
  int d=0;
  
  char attributes[100];
  dataToSend.toCharArray( attributes, 100 );
  mqttClient.publish(OUT_TOPIC, attributes );
  Serial.print( "Send to MQTT:" );
  Serial.println( attributes );   
  delay(50);
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
    setOnPiec= root["termostatValue"];
    change=true;
    Serial.println("SENSOR");    
    Serial.println(temperature);    
  }  
  
}
 
