
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define WIFI_AP "ZJC-N"
#define WIFI_PASSWORD "820813130882"

#define TOKEN "ESP8266_HC_SR505_TEST"

IPAddress mqttServerIP(192,168,1,168);  
/*IPAddress mqttServerIP(79,190,140,82);*/   
WiFiClient wifiClient;

void ConnectToAP();
void sendToMQTT(String dataToSend);

void setup() {
 Serial.begin(115200);
 delay(10);  
   pinMode(D5, INPUT);
   pinMode(D7, INPUT);
   
   pinMode(D8, OUTPUT);
   pinMode(D0, OUTPUT);
   
   digitalWrite (D5, LOW);
   digitalWrite (D7, LOW);
   
   digitalWrite (D8, LOW);
   digitalWrite (D0, LOW);
}
int prevState=0;
int prevState2=0;
void loop() {
   pinMode(D5, INPUT);
  ConnectToAP(); 
   int actState=digitalRead(D5);   
   int actState2=digitalRead(D7);   
   if(prevState==0 && actState==1){
    Serial.println("Wykryto ruch");
    String payload = "on"  ;      
    prevState=actState;
    digitalWrite (D8, HIGH);
    sendToMQTT(payload); 
    
   }
   if(prevState==1 && actState==0){
    Serial.println("Koniec sygnału");
    String payload = "off"    ;    
    prevState=actState;
    digitalWrite (D8, LOW);
    sendToMQTT(payload);     
   }
  if(prevState2==0 && actState2==1){
    Serial.println("Wykryto ruch 2");
    String payload = "on"  ;      
    prevState2=actState2;
    digitalWrite (D0, HIGH);
    sendToMQTT(payload); 
    
   }
   if(prevState2==1 && actState2==0){
    Serial.println("Koniec sygnału 2");
    String payload = "off"    ;    
    prevState2=actState2;
    digitalWrite (D0, LOW);
    sendToMQTT(payload);     
   }
   
   
   delay(100);
}


void ConnectToAP()
{
  int c=0;
  if(WiFi.status() == WL_CONNECTED)
    return;
  Serial.print("Connecting to AP ...");
  // attempt to connect to WiFi network
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    c++;
    if(c>=40){
      ESP.reset();
    }
  }
  Serial.println("Connected to AP");
}

/*MQTT**/
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
      delay( 100 );
      if(i>=10){
        Serial.println("[ERROR] Cannot connect to MQTT Server");
        ESP.reset();        
      }
    }
  }
  char attributes[100];
  dataToSend.toCharArray( attributes, 100 );
  client.publish( "/telemetry/sonoff1/switch1/cmd", attributes );
  Serial.print( "Send to MQTT:" );
  Serial.println( attributes );   
}


