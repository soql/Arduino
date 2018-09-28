#include <dht_nonblocking.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <soql_tools.h>

#define FW_VERSION 3
#define FW_INFO "Kontroller weed na BG"

#define WIFI_COUNT 3

wifi_struct wifi[WIFI_COUNT] = {
  {"SoqlAP", "EQDLPNNM"},
  {"SoqlNet", "820813130882"},
  {"SoqlNet-CN", "820813130882"}
};



// Callback function header
void callback(char* topic, byte* p, unsigned int length);

#define TOKEN "ESP8266_BW_WEED_CONTROLLER"

/*DHT22*/
DHT_nonblocking dht_sensor( D8 , DHT_TYPE_22 );
float temperature;
float humidity;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
IPAddress mqttServerIP(79, 190, 140, 82);

void onConnect(PubSubClient* client);

void onConnect(PubSubClient* client) {
  client->subscribe("/telemetry/weedbg/switch1/cmd");
  client->subscribe("/telemetry/weedbg/switch2/cmd");
  client->subscribe("/telemetry/weedbg/switch3/cmd");
  client->subscribe("/telemetry/weedbg/switch4/cmd");
  client->subscribe("/telemetry/weedbg/switch1/get");
  client->subscribe("/telemetry/weedbg/switch2/get");
  client->subscribe("/telemetry/weedbg/switch3/get");
  client->subscribe("/telemetry/weedbg/switch4/get");
}
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    continue;
  }
  ConnectToAP(wifi, WIFI_COUNT);
  connectToMQTT(&client, mqttServerIP, TOKEN, callback, onConnect);
  sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));
  checkForUpdates(FW_VERSION);

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
}
void loop() {
  ConnectToAP(wifi, WIFI_COUNT);
  connectToMQTT(&client, mqttServerIP, TOKEN, callback, onConnect);
  client.loop();

  if ( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );
    String payload = "{";
    payload += "\"temperature\":"; payload += temperature; payload += ",";
    payload += "\"humidity\":"; payload += humidity; payload += ",";
    payload += "\"rssi\":"; payload += WiFi.RSSI();
    payload += "}";
    sendToMqtt(&client,"/telemetry/weedbg/dht22", payload);

  }
}



void callback(char* topic, byte* p, unsigned int length) {
  Serial.print("Odebrano wiadomosc MQTT. Temat");
  Serial.println(topic);
  Serial.flush();
  Serial.print(" Treść:");
  String payload;
  for (int j = 0; j < length; j++) {
    payload += (char)p[j];
  }
  Serial.println(payload);
  if (strcmp(topic, "/telemetry/weedbg/switch1/cmd") == 0 && payload.equals("on")) {
    digitalWrite(D1, LOW);
    sendToMqtt(&client,"/telemetry/weedbg/switch1/state", payload);
  }
  if (strcmp(topic, "/telemetry/weedbg/switch1/cmd") == 0 && payload.equals("off")) {
    digitalWrite(D1, HIGH);
    sendToMqtt(&client,"/telemetry/weedbg/switch1/state", payload);
  }

  if (strcmp(topic, "/telemetry/weedbg/switch2/cmd") == 0 && payload.equals("on")) {
    digitalWrite(D2, LOW);
    sendToMqtt(&client,"/telemetry/weedbg/switch2/state", payload);
  }
  if (strcmp(topic, "/telemetry/weedbg/switch2/cmd") == 0 && payload.equals("off")) {
    digitalWrite(D2, HIGH);
    sendToMqtt(&client,"/telemetry/weedbg/switch2/state", payload);
  }

  if (strcmp(topic, "/telemetry/weedbg/switch3/cmd") == 0 && payload.equals("on")) {
    digitalWrite(D3, LOW);
    sendToMqtt(&client,"/telemetry/weedbg/switch3/state", payload);
  }
  if (strcmp(topic, "/telemetry/weedbg/switch3/cmd") == 0 && payload.equals("off")) {
    digitalWrite(D3, HIGH);
    sendToMqtt(&client,"/telemetry/weedbg/switch3/state", payload);
  }

  if (strcmp(topic, "/telemetry/weedbg/switch4/cmd") == 0 && payload.equals("on")) {
    digitalWrite(D4, LOW);
    sendToMqtt(&client,"/telemetry/weedbg/switch4/state", payload);
  }
  if (strcmp(topic, "/telemetry/weedbg/switch4/cmd") == 0 && payload.equals("off")) {
    digitalWrite(D4, HIGH);
    sendToMqtt(&client,"/telemetry/weedbg/switch4/state", payload);
  }
  if (strcmp(topic, "/telemetry/weedbg/switch1/get") == 0) {
    sendToMqtt(&client,"/telemetry/weedbg/switch1/state", digitalRead(D1) == 0 ? "off" : "on");
  }
  if (strcmp(topic, "/telemetry/weedbg/switch2/get") == 0) {
    sendToMqtt(&client,"/telemetry/weedbg/switch2/state", digitalRead(D2) == 0 ? "off" : "on");
  }
  if (strcmp(topic, "/telemetry/weedbg/switch3/get") == 0) {
    sendToMqtt(&client,"/telemetry/weedbg/switch3/state", digitalRead(D3) == 0 ? "off" : "on");
  }
  if (strcmp(topic, "/telemetry/weedbg/switch4/get") == 0) {
    sendToMqtt(&client,"/telemetry/weedbg/switch4/state", digitalRead(D4) == 0 ? "off" : "on");
  }

}

long measurement_timestamp = 0;

static bool measure_environment( float *temperature, float *humidity )
{


  /* Measure once every four seconds. */
  if ( millis( ) - measurement_timestamp > 15000ul )
  {
    if ( dht_sensor.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return ( true );
    }
  }

  return ( false );
}
