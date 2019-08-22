#define TINY_GSM_MODEM_SIM800
#include <TinyGPS++.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <Arduino.h>

TinyGPSPlus gps;

#define SerialAT Serial2
#define DUMP_SERIAL
#ifdef DUMP_SERIAL
#define SerialMon Serial
#endif
 
const char apn[]  = "internet";
const char user[] = "";
const char pass[] = "";

// MQTT details
const char* broker = "oth.net.pl";

const char* topic = "telemetry/bus/localization";

#define DUMP_AT_COMMANDS
#define ss Serial3
/*#define TINY_GSM_DEBUG SerialMon


#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif*/

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

long lastReconnectAttempt = 0;

void resetFunc(){
   #ifdef DUMP_SERIAL
  SerialMon.println("RESET");
   #endif     
  pinMode(PB6, OUTPUT);
  digitalWrite(PB6, LOW);
  delay(2000);
  digitalWrite(PB6, HIGH);
  setup();
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

String getGPSData(){   
 #ifdef DUMP_SERIAL 
  SerialMon.println("Start GPS data..."); 
   #endif
  ss.begin(9600);    
  boolean ok=true;
  float flat, flon, speed;
  unsigned long age, date, time, chars = 0;  

  int year;
  byte month, day, hour, minute, second;
  
  int i=0;
  ok=true;
  do {
    i++; 
       
    ok=true;
    smartDelay(0);
   flat=gps.location.lat();
    
    if(gps.location.isValid()){            
    }else{
      #ifdef DUMP_SERIAL
      SerialMon.println("NOT OK");
      #endif
      ok=false;
    }
    smartDelay(0);
    flon=gps.location.lng();
    if(gps.location.isValid()){     
    }else{
      #ifdef DUMP_SERIAL
      SerialMon.println("NOT OK");
      #endif
      ok=false;
    }
    smartDelay(0);
    
    speed=gps.speed.kmph();
    
    if( gps.speed.isValid()){    
    }else{
      #ifdef DUMP_SERIAL
      SerialMon.println("NOT OK");
      #endif
      ok=false;
    }
    smartDelay(0);
    TinyGPSDate d=gps.date;
     if(d.isValid()){  
       year=d.year();
       month=d.month();
       day=d.day();
    }else{
      #ifdef DUMP_SERIAL
      SerialMon.println("NOT OK");
      #endif;
      ok=false;
    }
    smartDelay(0);
 TinyGPSTime t=gps.time;
     if(t.isValid()){  
       hour=t.hour();
       minute=t.minute();
       second=t.second();
    }else{
      #ifdef DUMP_SERIAL
      SerialMon.println("NOT OK");
      #endif
      ok=false;
    }
  #ifdef DUMP_SERIAL
      SerialMon.println("KONIEC");
      SerialMon.flush();
      #endif
    smartDelay(0);
    smartDelay(1000);    
    if(ok)
      break;
      
  }while(ok!=true && i<120);     
 char sz[32];
    sprintf(sz, "%04d-%02d-%02d", year, month,day);
    char sz2[32];
    sprintf(sz2, "%02d:%02d:%02d", hour, minute,second);
  String payload;
  if(ok){
      payload = "{";
      payload += "\"latitude\":"; payload += String(flat,6); payload += ",";
      payload += "\"longitude\":"; payload += String(flon,6);payload += ",";
      payload += "\"time\":\""; payload += String(sz)+" "+String(sz2);
      payload += "\",";
      payload += "\"speed\":"; payload += speed;
      payload += "}";
  }else{
      payload += "{\"keepalive\":"; payload += "true\"}";
  }
#ifdef DUMP_SERIAL  
  SerialMon.println(payload);
  #endif
  ss.end();
  return payload;
}
String payload;
int attemptMqtt=0;
void setup() {  
  attemptMqtt=0;
  lastReconnectAttempt=0;
  //
  #ifdef DUMP_SERIAL
  SerialMon.begin(57600);
  SerialMon.println("Hello...");
  #endif       
 // delay(5000);
  //SerialAT.begin(57600);
  payload=getGPSData();   
  TinyGsmAutoBaud(SerialAT);      
#ifdef DUMP_SERIAL  
  SerialMon.println("Initializing modem...");
  #endif
  int status=0;
  do{  
    status=modem.init();
    if(!status){
      #ifdef DUMP_SERIAL
      SerialMon.println("Modem niezainicjalizowany");
      #endif
      modem.poweroff();
      delay(2000);   
      status=modem.restart();   
      delay(2000);   
    }    
  }while(!status);

  String modemInfo = modem.getModemInfo();
  #ifdef DUMP_SERIAL
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);


  SerialMon.print("Waiting for network...");
  #endif
  if (!modem.waitForNetwork()) {
    #ifdef DUMP_SERIAL
    SerialMon.println(" fail");        
    #endif
    delay(5000);
    resetFunc();
  }
  if (modem.isNetworkConnected()) {
    #ifdef DUMP_SERIAL
    SerialMon.println(" OK");  
    #endif
  }else{    
    delay(5000);
    #ifdef DUMP_SERIAL
    SerialMon.println(" Fail");  
    #endif
    resetFunc();
  }
  #ifdef DUMP_SERIAL
  SerialMon.print("Connecting to ");
  SerialMon.print(apn);
  #endif
  int att=0;
  while(!modem.gprsConnect(apn, user, pass)) {
    #ifdef DUMP_SERIAL
    SerialMon.println(" fail");  
      #endif
          att++;
          if(att>10){
            resetFunc();
          }
  }
  #ifdef DUMP_SERIAL
  SerialMon.println(" OK");  
  #endif
 
  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
}

boolean mqttConnect() {
  #ifdef DUMP_SERIAL
  SerialMon.print("Connecting to MQTT ");
  SerialMon.print(broker);
  #endif

  // Connect to MQTT Broker
  boolean status = mqtt.connect("GsmClientTest1");

  if (status == false) {    
    #ifdef DUMP_SERIAL
    SerialMon.println("MQTT NIE OK");      
    #endif    
    return false;
  }
  #ifdef DUMP_SERIAL
  SerialMon.println("MQTT OK");  
 #endif
  return mqtt.connected();
}

void loop() {
  if (!mqtt.connected()) {
    #ifdef DUMP_SERIAL
    SerialMon.println("=== MQTT NOT CONNECTED ===");
     #endif
    // Reconnect every 10 seconds
    unsigned long t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
         attemptMqtt=0;        
      }else{
        attemptMqtt++;
        if(attemptMqtt>3)
          resetFunc();
      }
    }
    delay(100);
    return;
  }

  mqtt.loop();  
  char attributes[256];
  payload.toCharArray( attributes, 256 );
  mqtt.publish(topic,attributes);    
  delay(3000);   
  payload=getGPSData();     
}
