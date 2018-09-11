#include "soql_tools.h"
#include <ESP8266WiFi.h>


int ConnectToAP(wifi_struct wifilist[], int wifisize)
{  
  if(WiFi.status() == WL_CONNECTED)
     return 1;
  Serial.flush();
  for(int i=0; i<wifisize; i++){
    int retry=0;
    if(WiFi.status() == WL_CONNECTED)
      return 1;
    Serial.print("Connecting to AP ...");
    Serial.print(wifilist[i].ssid.c_str());
    
    WiFi.begin(wifilist[i].ssid.c_str(), wifilist[i].password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
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
  return 0;
}
