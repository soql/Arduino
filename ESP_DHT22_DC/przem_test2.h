#include <soql_tools.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define FW_VERSION 1
#define FW_INFO "Czujnik temperatury - testowy"

#define WIFI_COUNT 3

wifi_struct wifi[WIFI_COUNT] = {   
  {"TP-LINK_A3D1EC"  , "1438775157"},
  {"DWR-116_5E63AE" , "1438775157"},
  {"ZJC-N"          , "820813130882"},
};

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.1.168", 0, 60000);

#define TOKEN "ESP8266_DHT22_DC_TEST2"

IPAddress mqttServerIP(192,168,1,168);  

/*DHT22*/
#define DHTPIN D4
#define DHTTYPE DHT22

#define OUT_TOPIC "telemetry/dht22/test2"

boolean real_deepsleep=false;
