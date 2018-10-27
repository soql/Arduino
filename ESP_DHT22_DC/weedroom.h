#include <soql_tools.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define FW_VERSION 1
#define FW_INFO "DHT22 w weedroom"

wifi_struct wifi[3]={
  {"ZJC-W","820813130882"},
  {"ZJC-N","820813130882"},
  {"ZJCCRYPTO","820813130882"}
};

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.1.168", 0, 60000);

#define TOKEN "ESP8266_DHT22_DC_WEED"

IPAddress mqttServerIP(192,168,1,168);  

/*DHT22*/
#define DHTPIN 0
#define DHTTYPE DHT22

#define OUT_TOPIC "telemetry/flower/inside"
