#include <soql_tools.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial ss(D5, D4);

#define FW_VERSION 1
#define FW_INFO "GPS bus"

#define TOKEN "ARDUINO_GPS_BUS"

#define WIFI_COUNT 1

wifi_struct wifi[2]={
  {"AndroidAP5261","8693666a"},
  {"SoqlNetBW","820813130882"}
  
};

WiFiClient wifiClient;
PubSubClient client(wifiClient);
IPAddress mqttServerIP(91,239,168,107);


void setup() {
  initSerial(115200);
    ss.begin(4800);
   ConnectToAP(wifi, WIFI_COUNT);
   connectToMQTT(&client, mqttServerIP, TOKEN, NULL, NULL);
   sendToMqtt(&client, "/telemetry/gps","test");
   

}

void loop() {
 bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
    sendToMqtt(&client, "/telemetry/gps/lat",String(flat));
  }
  
  gps.stats(&chars, &sentences, &failed);
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");

}
