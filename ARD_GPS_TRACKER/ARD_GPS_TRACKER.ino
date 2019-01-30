#define TINY_GSM_MODEM_A6

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <TinyGPS.h>

#define SerialAT Serial

#include <SoftwareSerial.h>
#ifdef DUMP_SERIAL
SoftwareSerial SerialMon(4, 3); // RX, TX
#endif
//#define DUMP_AT_COMMANDS
 TinyGPS gps;
 
const char apn[]  = "Internet";
const char user[] = "Internet";
const char pass[] = "Internet";

// MQTT details
const char* broker = "oth.net.pl";

const char* topic = "telemetry/bus/localization";

#define DUMP_AT_COMMANDS
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

void(* resetFunc) (void) = 0;//declare reset function at address 0

static void smartdelay(unsigned long ms, SoftwareSerial* gpsserial, TinyGPS* gps)
{
  unsigned long start = millis();
  do 
  {
    while (gpsserial->available())
      gps->encode(gpsserial->read());
  } while (millis() - start < ms);
}



String getGPSData(){   
//  SerialMon.println("Start GPS data...");
  SoftwareSerial gpsserial(5, 6);
  gpsserial.begin(9600);  
  delay(1000);  
  boolean ok=true;
  float flat, flon, speed;
  unsigned long age, date, time, chars = 0;  

  int year;
  byte month, day, hour, minute, second;
  
  int i=0;
  do {
    i++;
    ok=true;
    gps.f_get_position(&flat, &flon, &age);  
    
    if(flat!=TinyGPS::GPS_INVALID_F_ANGLE){            
    }else{
  //    SerialMon.println("NOT OK");
      ok=false;
    }
    smartdelay(0, &gpsserial, &gps);
    if(flon!=TinyGPS::GPS_INVALID_F_ANGLE){     
    }else{
    //  SerialMon.println("NOT OK");
      ok=false;
    }
    smartdelay(0, &gpsserial, &gps);
    speed=gps.f_speed_kmph();
    if(speed!=TinyGPS::GPS_INVALID_F_SPEED){    
    }else{
      #ifdef DUMP_SERIAL
      SerialMon.println("NOT OK");
      #endif
      ok=false;
    }
    smartdelay(0, &gpsserial, &gps);
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, NULL, &age);
    if(age!=TinyGPS::GPS_INVALID_AGE){     
    }else{
      #ifdef DUMP_SERIAL
      SerialMon.println("NOT OK");
      #endif
      ok=false;
    }
    smartdelay(0, &gpsserial, &gps);
    smartdelay(1000, &gpsserial, &gps);
  }while(ok!=true && i<10);    

  String payload = "{";
  payload += "\"latitude\":"; payload += String(flat,6); payload += ",";
  payload += "\"longitude\":"; payload += String(flon,6);payload += ",";
  payload += "\"time\":\""; payload += String(year)+"-"+String(month)+"-"+String(day)+" "+String(hour)+":"+String(minute)+":"+String(second);
  payload += "\",";
  payload += "\"speed\":"; payload += speed;
  payload += "}";
  
  //SerialMon.println(payload);
  return payload;
}
String payload;
void setup() {
  lastReconnectAttempt=0;
  //
  #ifdef DUMP_SERIAL
  SerialMon.begin(57600);
  #endif       
  delay(5000);
  SerialAT.begin(57600);
  //TinyGsmAutoBaud(SerialAT);
  payload=getGPSData();      
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
  if (!modem.gprsConnect(apn, user, pass)) {
   // SerialMon.println(" fail");    
    delay(5000);
    resetFunc();
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
    delay(1000);        
    return false;
  }
  #ifdef DUMP_SERIAL
  SerialMon.println("MQTT OK");  
 #endif
  return mqtt.connected();
}

void loop() {
  boolean con=false;
  int m=0;
   do{
    #ifdef DUMP_SERIAL
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    #endif
    // Reconnect every 10 seconds   
      unsigned long t = millis();
      if (t - lastReconnectAttempt > 10000L) {
        lastReconnectAttempt = t;
        con=mqttConnect();
        m++;
        if(m>=5){
          client.stop();
          #ifdef DUMP_SERIAL
          SerialAT.println("AT+CIPCLOSE");
          #endif
          delay(1000);
          resetFunc();
          break;
        }
        if (con) {
          lastReconnectAttempt = 0;
        }
      }
    }while(!con);
    //payload=getGPSData();      
    char attributes[256];
    payload.toCharArray( attributes, 256 );

    mqtt.publish(topic,attributes);    
    delay(1000);
    
    mqtt.disconnect();    
    client.stop();          
//    client.stopAll();          
    client.flush();
    
    delay(1000);
    resetFunc();
  }
  

