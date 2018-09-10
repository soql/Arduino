#include <ESP8266WiFi.h>

#define WIFI_AP "ZJC-N"
#define WIFI_PASSWORD "820813130882"

void setup() {
   Serial.begin(115200);
 delay(10); 
 
  pinMode(D3, OUTPUT);
}
long prevTime;
void loop() {  
 delay(1000);
 int state=digitalRead(D3);
 if(state==HIGH){
  digitalWrite(D3,LOW);
  Serial.println("LOW");
 }else{ 
  digitalWrite(D3,LOW);
  Serial.println("HIGH");
 } 

}
