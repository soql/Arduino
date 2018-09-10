#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>

#define WIFI_AP "ZJC-W"
#define WIFI_PASSWORD "820813130882"

/*#define WIFI_AP "DWR-116_5E63AE"
#define WIFI_PASSWORD "1438775157"*/

#define TOKEN "ESP8266_DHT_ADC_WEED"

IPAddress mqttServerIP(192,168,1,168);  
/*IPAddress mqttServerIP(79,190,140,82);*/

struct ads_result {
    int adc0;
    int adc1;
    int adc2;
    int adc3;
};

void setup() {
 Serial.begin(115200);
  Wire.pins(0, 2);
  Wire.begin(0, 2);
  byte error, address;
  int nDevices;
  nDevices = 0;
 delay(10);  
 struct ads_result ads=readADS();
 ConnectToAP(); 
 
 String payload = "{";
  payload += "\"adc0\":"; payload +=ads.adc0; payload += ",";
  payload += "\"adc1\":"; payload +=ads.adc1; payload += ",";
  payload += "\"adc2\":"; payload +=ads.adc2; payload += ",";
  payload += "\"adc3\":"; payload +=ads.adc3;
  payload += "}";
 sendToMQTT(payload);
 goDeepSleep();
}



Adafruit_ADS1015 ads; 



struct ads_result readADS(){
  ads.setGain(GAIN_ONE);
  ads.begin();  
  struct ads_result adsresults;  

  adsresults.adc0 = ads.readADC_SingleEnded(0);
  adsresults.adc1 = ads.readADC_SingleEnded(1);
  adsresults.adc2 = ads.readADC_SingleEnded(2);
  adsresults.adc3 = ads.readADC_SingleEnded(3);
    

   Serial.print("AIN0: "); Serial.println(adsresults.adc0);
   Serial.print("AIN1: "); Serial.println(adsresults.adc1);
   Serial.print("AIN2: "); Serial.println(adsresults.adc2);
   Serial.print("AIN3: "); Serial.println(adsresults.adc3);
   Serial.println(" ");
   return adsresults;
}
void goDeepSleep(){
  delay(30000);
  ESP.reset();
 /*Serial.println("Go into deepsleep mode.");
 ESP.deepSleep(1000000*3);*/
}
 
void loop() {
  // put your main code here, to run repeatedly:

}

void ConnectToAP()
{
  int i=0;
  if(WiFi.status() == WL_CONNECTED)
    return;
  Serial.print("Connecting to AP ...");
  // attempt to connect to WiFi network
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    i++;
    if(i>=40){
      goDeepSleep();
    }
  }
  Serial.println("Connected to AP");
}


/*MQTT**/
WiFiClient wifiClient;
PubSubClient client(wifiClient);


void sendToMQTT(String dataToSend){
 ConnectToAP();
  int i=0;
  client.setServer( mqttServerIP, 1883 );  
  Serial.print("Connecting to ThingsBoard node ...");
  while ( !client.connected() ) {
    if ( client.connect(TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      i++;
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 1000 );
      if(i>=10){
        Serial.println("[ERROR] Cannot connect to MQTT Server");
        goDeepSleep();
      }
    }
  }
  char attributes[100];
  dataToSend.toCharArray( attributes, 100 );
  client.publish( "telemetry/flower/soil", attributes );
  Serial.print( "Send to MQTT:" );
  Serial.println( attributes );   
  delay(500);
}


