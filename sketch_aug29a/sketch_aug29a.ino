#include <ESP8266WiFi.h>
void setup() {
 Serial.begin(57600);
 delay(1000);
 Serial.println("AAA");
}
int REAL_DISTANCE_CM=10;


void loop() {
  int tim=142;
  double speed_=((double)REAL_DISTANCE_CM*1000000/100);  
  double time_ = (double)tim*1000000/1000;
  String payload;   
  payload += "\"time\":\"";payload+=speed_ ; payload += "\",";
  payload+=time_ ; payload += "\" "; payload+=speed_/time_ ; payload += "\"";
 Serial.println(payload);
 Serial.flush();
 delay(1000);

}
