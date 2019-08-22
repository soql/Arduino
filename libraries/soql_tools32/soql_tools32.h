#ifndef _SOQL_TOOLS_H
#define _SOQL_TOOLS_H
#include <WString.h>
#include <PubSubClient.h>

struct wifi_struct {
    String ssid;
    String password;
};

int ConnectToAP(wifi_struct wifilist[], int wifisize);
void connectToMQTT(PubSubClient* client, IPAddress mqttServerIP, char* token, void (*callback)(char*, uint8_t*, uint32_t), void (*onConnect)(PubSubClient*));
String getMAC();
String generateTechInfo(int version, char* info);
void checkForUpdates(int version);
void sendToMqtt(PubSubClient* client, char* topic, String dataToSend);
void initSerial(int baudrate);
uint32_t getTimeFromNTP(IPAddress timeServerIP);
void goDeepSleep(int timeInSeconds, boolean deep);

#endif /*_SOQL_TOOLS_H */

