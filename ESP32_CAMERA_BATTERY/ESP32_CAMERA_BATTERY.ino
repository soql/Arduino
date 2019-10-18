/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card
  
  IMPORTANT!!! 
   - Select Board "ESP32 Wrover Module"
   - Select the Partion Scheme "Huge APP (3MB No OTA)
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include "esp_camera.h"
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32_FTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <soql_tools32.h>
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "soc/sens_reg.h"
#include "driver/rtc_io.h"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22



#define TOKEN "ESP8266_CAMERA_BATTERY"

#define FW_VERSION 8
#define FW_INFO "Kamera bateria"
#define OUT_TOPIC "/telemetry/camera1/data"

// FTP Server credentials
char ftp_server[] = "zjc.oth.net.pl";
char ftp_user[]   = "esp32cam";
char ftp_pass[]   = "%494Wh#f$PXk";

String pic_name = "";
camera_fb_t * fb = NULL;

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.1.168");

IPAddress mqttServerIP(192,168,1,168);

#define WIFI_COUNT 4

wifi_struct wifi[WIFI_COUNT] = {           
  {"ZJC-W","820813130882"},\
  {"ZJC-N"  , "820813130882"},
  {"ZJCCRYPTO","820813130882"},
  {"SoqlNet"  , "820813130882"}
  
};

uint64_t reg_a;
uint64_t reg_b;
uint64_t reg_c;

int analogPin1=14;

/*Co ile sekund fota*/
int deepSleepTime=30*60;

ESP32_FTPClient ftp (ftp_server, ftp_user, ftp_pass);

String timeRead;
int batteryState1=0;

void FTP_upload();
void goDeepSleepInt(int sec){
    WRITE_PERI_REG(SENS_SAR_START_FORCE_REG, reg_a);  // fix ADC registers
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  WRITE_PERI_REG(SENS_SAR_MEAS_START2_REG, reg_c);    
 goDeepSleep(sec);  
}
void sendToMqtt(PubSubClient client){
  String payload;        
    payload = "{"; 
    payload += "\"time\":\"";payload+=timeRead; payload += "\",";
    payload += "\"batteryRaw\":\"";payload+=String(batteryState1); payload += "\"";
    payload += "}";
 sendToMqtt(&client,OUT_TOPIC, payload);
}
int ConnectToAPInt(wifi_struct wifilist[], int wifisize)
{
  if(WiFi.status() == WL_CONNECTED){
  return 1;
  }else{
  Serial.println("");
  }
  Serial.flush();
  for(int i=0; i<wifisize; i++){
    int retry=0;
    if(WiFi.status() == WL_CONNECTED)
      return 1;
    Serial.print("Connecting to AP ");
    Serial.print(wifilist[i].ssid.c_str());

    WiFi.begin(wifilist[i].ssid.c_str(), wifilist[i].password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(300);
      Serial.print(".");
      Serial.flush();
      retry++;
      if(retry>=40){
        Serial.println(".");
        break;
      }
    }
    if(WiFi.status() != WL_CONNECTED)
      continue;
    Serial.println("Connected to AP");
    Serial.flush();
    return 1;
  }
  Serial.println("Cannot start wifi connection :-(");
  goDeepSleepInt(5);
  return 0;
}


void connectToMQTTInt(PubSubClient* client, IPAddress mqttServerIP, char* token, void (*callback)(char*, uint8_t*, uint32_t), void (*onConnect)(PubSubClient*)){
   if(client->connected()){
     return;
   }
   int retry=0;
   Serial.print("Connecting to MQTT Server ...");
   while ( !client->connected() ) {
    Serial.print(".");
    client->setServer( mqttServerIP, 1883);
    if(callback!=NULL){
      client->setCallback(callback);
    }
     if ( client->connect(token) ) {
       if(onConnect!=NULL){
           onConnect(client);
       }
       Serial.println( "[DONE]" );
     }else{
       delay( 100 );
       retry++;
       Serial.print(".");
         if(retry>=10){
        Serial.println("[ERROR] Cannot connect to MQTT Server");
        goDeepSleepInt(5);
      }
     }
   }
}

void setup() {  
reg_a = READ_PERI_REG(SENS_SAR_START_FORCE_REG);
reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
reg_c = READ_PERI_REG(SENS_SAR_MEAS_START2_REG);
  WiFi.mode(WIFI_OFF);
  btStop();
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  batteryState1=0; 
  delay(2000);



 /* analogSetCycles(8);                        
  analogSetSamples(1);                       
  analogSetClockDiv(1);                     
  analogSetAttenuation(ADC_0db); */
  pinMode(analogPin1, INPUT);  
/*  adcAttachPin(analogPin1);*/
 /* analogSetClockDiv(255);*/
 delay(100);
/* for(int j=0; j<15; j++){
 adcStart(analogPin1);
  while ( adcBusy(analogPin1));  
  batteryState1 = batteryState1 + adcEnd(analogPin1);
  delay(100);
 }  */
 for(int j=0; j<15; j++){   
  batteryState1 = batteryState1 + analogRead(analogPin1);
  delay(100);
 } 
  Serial.println(batteryState1/15);
  delay(100);
 
 /*delay(100);
  Serial.println("ADC MESS START");
  Serial.println(SENS_SAR_START_FORCE_REG);
  Serial.println(SENS_SAR_READ_CTRL2_REG);
  for(int j=0; j<15; j++){
    batteryState1 = batteryState1+analogRead(analogPin1);    
    delay(200);
  }*/
  batteryState1=batteryState1/15;   
  Serial.println(batteryState1);     
 
  /*WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); *///disable brownout detector
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
   
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.sharpness=-2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  WiFiClient wifiClient;
  PubSubClient client(wifiClient);

  ConnectToAPInt(wifi, WIFI_COUNT);    
  connectToMQTTInt(&client, mqttServerIP, TOKEN, NULL, NULL);  
 
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
     goDeepSleepInt(30);
  }
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  rtc_gpio_hold_dis(GPIO_NUM_4);
  delay(3000);
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
     goDeepSleepInt(30);
  }
  Serial.println("Cam ok");   

    
  timeClient.begin();
 int i=0;
 while(!timeClient.update()){
  i++;
  if(i>20)
   goDeepSleepInt(30);
  Serial.println("TIME NOT UPDATE");
  delay(100);
 } 
 /*Gaszenie diody*/
 delay(500);
// pinMode(4, OUTPUT);
 digitalWrite(4, LOW);  
 rtc_gpio_hold_en(GPIO_NUM_4);
 timeRead=String(timeClient.getEpochTime());
 timeClient.end();
 pic_name= timeRead+".jpg";
  FTP_upload();
  delay(1000);
  esp_camera_fb_return(fb); 
    
  delay(2000);  
  sendToMqtt(&client,"/telemetry/technical/info", generateTechInfo(FW_VERSION, FW_INFO));  
  sendToMqtt(client);  
  
  Serial.println("Going to sleep now");
  
  client.disconnect();
  wifiClient.stop();
  WiFi.mode(WIFI_OFF);
  delay(1000);
goDeepSleepInt(deepSleepTime);
}

void loop() {
  
}


void FTP_upload()
{
  Serial.println("Uploading via FTP");
  ftp.OpenConnection();
  
  //Create a file and write the image data to it;
  ftp.InitFile("Type I");
  ftp.ChangeWorkDir("~/"); // change it to reflect your directory
  const char *f_name = pic_name.c_str();
  ftp.NewFile( f_name );
  ftp.WriteData(fb->buf, fb->len);
  ftp.CloseFile();  
    
  Serial.println("Finish Uploading via FTP");

}
