#include "soql_tools.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>

const char* fwUrlBase = "http://esp.oth.net.pl:8099/";

void goDeepSleep(int timeInSeconds, boolean deep){
	if(deep){
		 Serial.println("Go into deepsleep mode.");
		 ESP.deepSleep(1000000*timeInSeconds);
	}else{
	 delay(1000*timeInSeconds);
	 ESP.restart();
	}
}

int ConnectToAP(wifi_struct wifilist[], int wifisize)
{
  if(WiFi.status() == WL_CONNECTED){
	return 1;
  }else{
	Serial.println("");
  }
  Serial.flush();
  for(int i=0; i<wifisize; i++){
    int retry=0;
    if(WiFi.status() == WL_CONNECTED)
      return 1;
    Serial.print("Connecting to AP ");
    Serial.print(wifilist[i].ssid.c_str());

    WiFi.begin(wifilist[i].ssid.c_str(), wifilist[i].password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(300);
      Serial.print(".");
      Serial.flush();
      retry++;
      if(retry>=40){
        Serial.println(".");
        break;
      }
    }
    if(WiFi.status() != WL_CONNECTED)
      continue;
    Serial.println("Connected to AP");
    Serial.flush();
    return 1;
  }
  Serial.println("Cannot start wifi connection :-(");
  ESP.restart();
  return 0;
}


void connectToMQTT(PubSubClient* client, IPAddress mqttServerIP, char* token, void (*callback)(char*, uint8_t*, uint32_t), void (*onConnect)(PubSubClient*)){
   if(client->connected()){
     return;
   }
   int retry=0;
   Serial.print("Connecting to MQTT Server ...");
   while ( !client->connected() ) {
    Serial.print(".");
    client->setServer( mqttServerIP, 1883);
    if(callback!=NULL){
	    client->setCallback(callback);
    }
     if ( client->connect(token) ) {
	     if(onConnect!=NULL){
		       onConnect(client);
	     }
       Serial.println( "[DONE]" );
     }else{
       delay( 100 );
       retry++;
       Serial.print(".");
         if(retry>=10){
        Serial.println("[ERROR] Cannot connect to MQTT Server");
        ESP.restart();
      }
     }
   }
}

String getMAC()
{
  uint8_t mac[6];
  char result[14];
  WiFi.macAddress( mac );
  snprintf( result, sizeof( result ), "%02x%02x%02x%02x%02x%02x", mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );
  return String( result );
}

String generateTechInfo(int version, char* info){
  String payload = "{";
  payload += "\"macAddress\":"; payload += getMAC(); payload += ",";
  payload += "\"version\":"; payload += String(version);payload += ",";
  payload += "\"info\":"; payload += String(info);
  payload += "}";
  return payload;
}


void checkForUpdates(int version) {
  String mac = getMAC();
  String fwURL = String( fwUrlBase );
  fwURL.concat(mac);
  fwURL.concat("/");
  String fwVersionURL = fwURL;
  fwVersionURL.concat( "version" );

  Serial.println( "Checking for firmware updates." );
  Serial.print( "MAC address: " );
  Serial.println( mac );
  Serial.print( "Firmware version URL: " );
  Serial.println( fwVersionURL );

  HTTPClient httpClient;
  httpClient.begin( fwVersionURL );
  int httpCode = httpClient.GET();
  if ( httpCode == 200 ) {
    String newFWVersion = httpClient.getString();

    Serial.print( "Current firmware version: " );
    Serial.println( version );
    Serial.print( "Available firmware version: " );
    Serial.println( newFWVersion );

    int newVersion = newFWVersion.toInt();
    if ( newVersion > version ) {
      Serial.println( "Preparing to update" );

      String fwImageURL = fwURL;
      fwImageURL.concat( "firmware.bin" );
      t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
      }
    }
    else {
      Serial.println( "Already on latest version" );
    }
  }
  else {
    Serial.print( "Firmware version check failed, got HTTP response code " );
    Serial.println( httpCode );
  }
  httpClient.end();
}

void sendToMqtt(PubSubClient* client, char* topic, String dataToSend){
  char attributes[256];
  dataToSend.toCharArray( attributes, 256 );
  client->publish( topic, attributes );
  Serial.print( "Send to MQTT:" );
  Serial.println( attributes );
  delay(100);
}
void initSerial(int baudrate){
	Serial.begin(baudrate);
	while(!Serial){
		continue;
	}
}
