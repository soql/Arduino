#include <dht_nonblocking.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <soql_tools.h>

wifi_struct wifi[2]={
  {"SoqlNet","820813130882"},
  {"SoqlNet-CN","820813130882"}
};

// Callback function header
void callback(char* topic, byte* p, unsigned int length);

#define TOKEN "ESP8266_BW_WEED_CONTROLLER"

/*DHT22*/
DHT_nonblocking dht_sensor( D7, DHT_TYPE_22 );

WiFiClient wifiClient;
PubSubClient client(wifiClient);
IPAddress mqttServerIP(192,168,2,2);

void setup() { 
 Serial.begin(115200);
 while(!Serial){
  continue;
 }
   pinMode(D1, OUTPUT);
   pinMode(D2, OUTPUT);
   pinMode(D3, OUTPUT);
   pinMode(D4, OUTPUT);
   digitalWrite(D1,HIGH);
   digitalWrite(D2,HIGH);
   digitalWrite(D3,HIGH);
   digitalWrite(D4,HIGH);
}
void loop() {  
  ConnectToAP(wifi, 2); 
  connectToMQTT(&client, mqttServerIP, TOKEN); 
  client.loop();
}



void connectToMQTT(PubSubClient* client, IPAddress mqttServerIP, char* token){
   int retry=0;   
   while ( !client->connected() ) {    
    Serial.print("Connecting to MQTT Server ...");   
    client->setServer( mqttServerIP, 1883);  
    client->setCallback(callback);
     if ( client->connect(token) ) {
        client->subscribe("/telemetry/weedbg/switch1/cmd");        
        client->subscribe("/telemetry/weedbg/switch2/cmd");        
        client->subscribe("/telemetry/weedbg/switch3/cmd");        
        client->subscribe("/telemetry/weedbg/switch4/cmd");    
        client->subscribe("/telemetry/weedbg/switch1/get");        
        client->subscribe("/telemetry/weedbg/switch2/get");        
        client->subscribe("/telemetry/weedbg/switch3/get");        
        client->subscribe("/telemetry/weedbg/switch4/get");          
       Serial.println( "[DONE]" );
     }else{
       delay( 100 );
       retry++;
       Serial.print(".");
         if(retry>=10){
        Serial.println("[ERROR] Cannot connect to MQTT Server");
        ESP.reset();
      }
     }
   }   
}

void sendToMqtt(char* topic, String dataToSend){
  char attributes[100];
  dataToSend.toCharArray( attributes, 100 );
  client.publish( topic, attributes );
  Serial.print( "Send to MQTT:" );
  Serial.println( attributes );  
  delay(100);
}

void callback(char* topic, byte* p, unsigned int length){
  Serial.print("Odebrano wiadomosc MQTT. Temat");
  Serial.println(topic);
  Serial.flush();
  Serial.print(" Treść:");
   String payload;
    for(int j=0; j<length; j++){
      payload+=(char)p[j];
    }
  Serial.println(payload);   
   if(strcmp(topic, "/telemetry/weedbg/switch1/cmd")==0 && payload.equals("on")){  
      digitalWrite(D1,LOW);
      sendToMqtt("/telemetry/weedbg/switch1/state",payload);
   }
   if(strcmp(topic, "/telemetry/weedbg/switch1/cmd")==0 && payload.equals("off")){  
      digitalWrite(D1,HIGH);
      sendToMqtt("/telemetry/weedbg/switch1/state",payload);
   }

    if(strcmp(topic, "/telemetry/weedbg/switch2/cmd")==0 && payload.equals("on")){  
      digitalWrite(D2,LOW);
      sendToMqtt("/telemetry/weedbg/switch2/state",payload);
   }
   if(strcmp(topic, "/telemetry/weedbg/switch2/cmd")==0 && payload.equals("off")){  
      digitalWrite(D2,HIGH);
      sendToMqtt("/telemetry/weedbg/switch2/state",payload);
   }

    if(strcmp(topic, "/telemetry/weedbg/switch3/cmd")==0 && payload.equals("on")){  
      digitalWrite(D3,LOW);
      sendToMqtt("/telemetry/weedbg/switch3/state",payload);
   }
   if(strcmp(topic, "/telemetry/weedbg/switch3/cmd")==0 && payload.equals("off")){  
      digitalWrite(D3,HIGH);
      sendToMqtt("/telemetry/weedbg/switch3/state",payload);
   }

    if(strcmp(topic, "/telemetry/weedbg/switch4/cmd")==0 && payload.equals("on")){  
      digitalWrite(D4,LOW);
      sendToMqtt("/telemetry/weedbg/switch4/state",payload);
   }
   if(strcmp(topic, "/telemetry/weedbg/switch4/cmd")==0 && payload.equals("off")){  
      digitalWrite(D4,HIGH);
      sendToMqtt("/telemetry/weedbg/switch4/state",payload);
   }
   if(strcmp(topic, "/telemetry/weedbg/switch1/get")==0){ 
    sendToMqtt("/telemetry/weedbg/switch1/state", digitalRead(D1)==0?"off":"on");
   }
   if(strcmp(topic, "/telemetry/weedbg/switch2/get")==0){ 
    sendToMqtt("/telemetry/weedbg/switch2/state", digitalRead(D2)==0?"off":"on");
   }
   if(strcmp(topic, "/telemetry/weedbg/switch3/get")==0){ 
    sendToMqtt("/telemetry/weedbg/switch3/state", digitalRead(D3)==0?"off":"on");
   }
   if(strcmp(topic, "/telemetry/weedbg/switch4/get")==0){ 
    sendToMqtt("/telemetry/weedbg/switch4/state", digitalRead(D4)==0?"off":"on");
   }
   
}

