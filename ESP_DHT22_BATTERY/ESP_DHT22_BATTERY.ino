#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>

/*#define WIFI_AP "ZJC-N"
#define WIFI_PASSWORD "820813130882"*/

#define WIFI_AP "DWR-116_5E63AE"
#define WIFI_PASSWORD "1438775157"

/*#define WIFI_AP "TP-LINK_A3D1EC"
#define WIFI_PASSWORD "1438775157"*/

#define TOKEN "ESP8266_DHT_OFFICE"

IPAddress timeServerIP(192,168,2,3);   
/*IPAddress timeServerIP(79,190,140,82); */  

IPAddress mqttServerIP(192,168,2,3);  
/*IPAddress mqttServerIP(79,190,140,82);*/   

struct dhtresults_struct {
    float temperature;
    float humidity;    
};

void setup() {
 Serial.begin(115200);
 delay(10);
 pinMode(D0, WAKEUP_PULLUP);
 Serial.print("Odczyt ADC: ");
 int rawValue=readADC();
 Serial.println(rawValue);
 double batteryStatus=convertADC(rawValue);
 struct dhtresults_struct dht22=getResultsFromDHT22(); 
 ConnectToAP(); 
 uint32_t actualTime=getTimeFromNTP(timeServerIP); 
 
 String payload = "{";
  payload += "\"temperature\":"; payload += dht22.temperature; payload += ",";
  payload += "\"humidity\":"; payload += dht22.humidity;payload += ",";
  payload += "\"time\":"; payload += actualTime;payload += ",";
  payload += "\"battery\":"; payload += batteryStatus;payload += ",";
  payload += "\"rawBattery\":"; payload += rawValue;payload += ",";
   payload += "\"rssi\":"; payload += WiFi.RSSI();
  payload += "}";

 sendToMQTT(payload);
 goDeepSleep();
}

void goDeepSleep(){
 Serial.println("Go into deepsleep mode.");
 ESP.deepSleep(1000000*600);
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

/*Getting time from NTP Server*/
uint32_t getTimeFromNTP(IPAddress timeServerIP){
  ConnectToAP();
  startUDP();  
  
  uint32_t time = 0;
  int i=0;
  while(i<20 && time==0){
    sendNTPpacket(timeServerIP); 
    delay(500);
    Serial.print("Geting time from NTP. Retry nr: ");
    Serial.println(i+1);
    time = getTime();      
    
    i++;
  }
  if(time==0){
    Serial.println("[ERROR] Cannot get time from NTP Server.");
    goDeepSleep();
  }
  Serial.print("Time: ");
  Serial.println(time); 
  return time;
  
  
}

const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message

byte NTPBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

WiFiUDP UDP; 

void startUDP() {
  Serial.println("Starting UDP");
  UDP.begin(123);
  Serial.print("Local port:\t");
  Serial.println(UDP.localPort());
  Serial.println();
  delay(500);
}

uint32_t getTime() {
  if (UDP.parsePacket() == 0) { // If there's no response (yet)
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}

void sendNTPpacket(IPAddress& address) {
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
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
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
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
  client.publish( "telemetry/dht22/livingroom", attributes );
  Serial.print( "Send to MQTT:" );
  Serial.println( attributes );   
  delay(500);
}

/*Read ADC*/
double readADC(){
  int rawValue=analogRead(A0);
  return rawValue;
}

double convertADC(int value){
  int rawValue=value;
  /*Stary dzielnik (biuro)*/
  int MIN_VALUE=840;
  int MAX_VALUE=1024;
  
  /*Dzielnik 200/268*/
 /* int MIN_VALUE=620;
  int MAX_VALUE=758;*/
  double toReturn=(((double)(rawValue))-MIN_VALUE)/(MAX_VALUE-MIN_VALUE)*100;
  if(toReturn<0){
    toReturn=0;
  }
  if(toReturn>100){
    toReturn=100;
  }
  return toReturn;
}


/*DHT22*/
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

struct dhtresults_struct getResultsFromDHT22(){
  int i=0;
  float t,h;
  struct dhtresults_struct dhtresults;
  while(i<10){
      h = dht.readHumidity();
      t = dht.readTemperature();
      if (isnan(h) || isnan(t)) {
        i++;
        Serial.println("Failed to read from DHT sensor!");      
        delay(500);
      }else{
        dhtresults.temperature=t;
        dhtresults.humidity=h;
        Serial.print("DHT22 Results. Humidity: ");      
        Serial.print(h);      
        Serial.print(". Temperature: ");              
        Serial.print(t);      
        Serial.println(" *C ");      
        return dhtresults;
      }
  }
  goDeepSleep();
  return dhtresults;
}

