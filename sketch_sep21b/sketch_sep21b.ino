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



#define TIME_TO_SLEEP  5        //Time ESP32 will go to sleep (in seconds)

int pictureNumber = 0;
// FTP Server credentials
char ftp_server[] = "zjc.oth.net.pl";
char ftp_user[]   = "esp32cam";
char ftp_pass[]   = "%494Wh#f$PXk";

String pic_name = "";
camera_fb_t * fb = NULL;
const char* ssid = "SoqlNet";
const char* password = "820813130882";

/*NtpClient*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "192.168.2.2");


#define WIFI_COUNT 4

wifi_struct wifi[WIFI_COUNT] = {     
  {"SoqlNet"  , "820813130882"},
  {"ZJC-N"  , "820813130882"},
  {"ZJC-W","820813130882"},
  {"ZJCCRYPTO","820813130882"},
};


WiFiClient wifiClient;

ESP32_FTPClient ftp (ftp_server, ftp_user, ftp_pass);

void setup() {  
  Serial.begin(115200);
   Serial.setDebugOutput(true);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

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
  
  pinMode(4, INPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  WiFi.begin(ssid, password);
  ConnectToAP(wifi, WIFI_COUNT);    
 // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
     goDeepSleep(30);
  }
    
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
     goDeepSleep(30);
  }
  Serial.println("Cam ok");
  // initialize EEPROM with predefined size
  

  
  delay(1000);
  Serial.println("Cam ok 2");
  timeClient.begin();
 int i=0;
 while(!timeClient.update()){
  i++;
  if(i>20)
    break;
  Serial.println("TIME NOT UPDATE");
  delay(100);
 }
 Serial.println( timeClient.getEpochTime());
 timeClient.end();
 pic_name= String(timeClient.getEpochTime())+".jpg";
  FTP_upload();
  delay(1000);
  esp_camera_fb_return(fb); 
  
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);  
  rtc_gpio_hold_en(GPIO_NUM_4);
  delay(2000);
  Serial.println("Going to sleep now");
  delay(15000);
 goDeepSleep(30);
  Serial.println("This will never be printed");
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
