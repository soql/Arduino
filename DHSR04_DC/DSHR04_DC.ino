#include <PubSubClient.h>
#include <ESP8266WiFi.h>


#define WIFI_AP "ZJC-N"
#define WIFI_PASSWORD "820813130882"

#define TOKEN "ESP8266_TOKEN_2"

#define echoPin 2 // Echo Pin
#define trigPin 0 // Trigger Pin


long duration, distance; // Duration used to calculate distance

int status = WL_IDLE_STATUS;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

char mqttServer[] = "oth.net.pl";
void setup() {
   Serial.begin(115200); 
   pinMode(trigPin, OUTPUT);
   pinMode(echoPin, INPUT);
   client.setServer( mqttServer, 1883 );      
}

void loop() {
   if ( !client.connected() ) {
    reconnect();
  }  
  Serial.println("Połączone!!");
  sendMqttData(getDistance());
  delay(5000);
}

void sendMqttData(int distanceToSend){
  String payload = "{";
  payload += "\"distance\":"; payload+=distanceToSend;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "telemetry/dhsr04/koltownia", attributes );
  Serial.println( attributes );
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

long getDistance(){
    /* The following trigPin/echoPin cycle is used to determine the
  distance of the nearest object by bouncing soundwaves off of it. */
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration/58.2;
  Serial.println(distance);
  return distance;
  
}


