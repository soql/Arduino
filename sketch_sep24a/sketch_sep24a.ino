#include "esp_camera.h"
#include <soql_tools32.h>
#include "soc/sens_reg.h"
#include "driver/rtc_io.h"
#define CAMERA_MODEL_AI_THINKER

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
#define WIFI_COUNT 4

wifi_struct wifi[WIFI_COUNT] = {         
  {"ZJC-N"  , "820813130882"},
  {"ZJC-W","820813130882"},
  {"ZJCCRYPTO","820813130882"},
  {"SoqlNet"  , "820813130882"}};
  
 uint64_t reg_a;
uint64_t reg_b;
uint64_t reg_c;
 
void setup() {
  Serial.begin(115200);

}

void loop() {
  reg_a = READ_PERI_REG(SENS_SAR_START_FORCE_REG);
reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
reg_c = READ_PERI_REG(SENS_SAR_MEAS_START2_REG);
pinMode(2, INPUT);
pinMode(4, INPUT);
pinMode(14, INPUT);
pinMode(15, INPUT);
pinMode(13, INPUT);
pinMode(12, INPUT);

  for(int i=0; i<10; i++){
    int sensorValue = analogRead(2);
    Serial.print(2);
   Serial.print("    ");
    Serial.println(sensorValue);
    delay(100);
    Serial.print(4);
   Serial.print("    ");
     sensorValue = analogRead(4);
    Serial.println(sensorValue);
    delay(100);
    Serial.print(14);
   Serial.print("    ");
     sensorValue = analogRead(14);
    Serial.println(sensorValue);
    delay(100);
    Serial.print(15);
   Serial.print("    ");
     sensorValue = analogRead(15);
    Serial.println(sensorValue);
    delay(100);
    Serial.print(13);
   Serial.print("    ");
     sensorValue = analogRead(13);
    Serial.println(sensorValue);
    delay(100);
    Serial.print(12);
    Serial.print("    ");
     sensorValue = analogRead(12);
    Serial.println(sensorValue);
    delay(100);
  }
  ConnectToAP(wifi, WIFI_COUNT);    
 WRITE_PERI_REG(SENS_SAR_START_FORCE_REG, reg_a);  // fix ADC registers
 WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
 WRITE_PERI_REG(SENS_SAR_MEAS_START2_REG, reg_c); 
goDeepSleep(5);
}
