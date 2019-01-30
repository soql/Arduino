#include <soql_tools.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define FW_VERSION 1
#define FW_INFO "Czujnik temperatury - łazienka piętro"

#define WIFI_COUNT 3

wifi_struct wifi[WIFI_COUNT] = {   
  {"TP-LINK_A3D1EC"  , "1438775157"},
  {"DWR-116_5E63AE" , "1438775157"},
  {"ZJC-N"          , "820813130882"},
};

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.2.3", 0, 60000);

#define TOKEN "ESP8266_DHT22_DC_BATHROOM_FLOOR"

IPAddress mqttServerIP(192,168,2,3);  

/*DHT22*/
#define DHTPIN D2
#define DHTTYPE DHT22

#define OUT_TOPIC "telemetry/dht22/floor/bathroom"

boolean real_deepsleep=true;
