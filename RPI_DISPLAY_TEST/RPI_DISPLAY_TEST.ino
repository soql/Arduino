/*====================================================================================
  This sketch demonstrates loading images which have been stored as files in the
  built-in FLASH memory on a NodeMCU 1.0 (ESP8266 based, ESP-12E Module) rendering the
  images onto a 160 x 128 pixel TFT screen.
  The images are stored in the SPI FLASH Filing System (SPIFFS), which effectively
  functions like a tiny "hard drive". This filing system is built into the ESP8266
  Core that can be loaded from the IDE "Boards manager" menu option. This is at
  version 2.3.0 at the time of sketch creation.
  The size of the SPIFFS partition can be set in the IDE as 1Mbyte or 3Mbytes. Either
  will work with this sketch. Typically most sketches easily fit within 1 Mbyte so a
  3 Mbyte SPIFS partition can be used, in which case it can contain 100's of Jpeg
  full screem images.
  The Jpeg library can be found here:
  https://github.com/Bodmer/JPEGDecoder
 
  Images in the Jpeg format can be created using Paint or IrfanView or other picture
  editting software.
  Place the images inside the sketch folder, in a folder called "Data".  Then upload
  all the files in the folder using the Arduino IDE "ESP8266 Sketch Data Upload" option
  in the "Tools" menu:
  http://www.esp8266.com/viewtopic.php?f=32&t=10081
  https://github.com/esp8266/arduino-esp8266fs-plugin/releases
  
  This takes some time, but the SPIFFS content is not altered when a new sketch is
  uploaded, so there is no need to upload the same files again!
  Note: If open, you must close the "Serial Monitor" window to upload data to SPIFFS!
  The IDE will not copy the "data" folder with the sketch if you save the sketch under
  another name. It is necessary to manually make a copy and place it in the sketch
  folder.
  This sketch includes example images in the Data folder.
  Saving images, uploading and rendering on the TFT screen couldn't be much easier!
  Created by Bodmer 24th Jan 2017 - Tested in Arduino IDE 1.8.0 esp8266 Core 2.3.0
  ==================================================================================*/

//====================================================================================
//                                  Libraries
//====================================================================================
// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#include <WiFiClient.h>

// JPEG decoder library
#include <TFT_eFEX.h>
#include <HTTPClient.h>
// SPI library, built into IDE
#include <SPI.h>

// Call up the TFT library
#include <TFT_eSPI.h> // Hardware-specific library for ESP8266
// The TFT control pins are set in the User_Setup.h file <<<<<<<<<<<<<<<<< NOTE!
// that can be found in the "src" folder of the library

// Invoke TFT library
TFT_eSPI tft = TFT_eSPI();
HTTPClient http;
#define USE_SERIAL Serial
const char* ssid = "SoqlNet";
const char* password = "820813130882";
const char* host = "esp8266fs";
TFT_eFEX  fex = TFT_eFEX(&tft);
uint8_t PicArray[80950] = {0}; //Array for picture make sure under 15K
int pic = 1;
void GetPic() {
  HTTPClient http;
  long totalSize = 0;
  boolean chone = 1;
  // configure server and url update based on your URL
  http.begin("http://zjc.oth.net.pl:8765/picture/1/current?width=320");  //update based on your URL
  //http.begin("http://10.16.10.11:8980/tempgr/ac/BaboonL.jpg");
  // start connection and send HTTP header
  long httpCode = http.GET();
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {

      // get lenght of document (is -1 when Server sends no Content-Length header)
      long len = http.getSize();

      // create buffer for read
      uint8_t buff[2048] = { 0 };

      // get tcp stream
      WiFiClient * stream = http.getStreamPtr();

      // read all data from server
      while (http.connected() && (len > 0 || len == -1)) {
        size_t size = stream->available();
        if (size) {

          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          memcpy (PicArray + totalSize, buff, c);
          totalSize = totalSize + c;

          if (len > 0) {
            len -= c;
          }
        }

        yield();
      }
      USE_SERIAL.print("[HTTP] connection closed or file end.\n");

    }
  } else {
    USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  USE_SERIAL.println("TotalS:" + String(totalSize));

  USE_SERIAL.print("First 10 Bytes: ");
  for (int ipt = 0; ipt < 11; ipt++) {
    USE_SERIAL.print(PicArray[ipt], HEX);
    USE_SERIAL.print(",");
  }
  USE_SERIAL.print("\nLast 10 Bytes : ");
  for (int ipt = 10; ipt >= 0; ipt--) {
    USE_SERIAL.print(PicArray[totalSize - ipt], HEX);
    USE_SERIAL.print(",");
  }
  USE_SERIAL.println("");
  uint8_t scale = (uint8_t)JPEG_DIV_2;
  fex.drawJpg(PicArray, sizeof(totalSize), 0, 0);
}
//====================================================================================
//                                    Setup
//====================================================================================
void setup()
{
  Serial.begin(115200); // Used for messages and the C array generator
 if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.begin(ssid, password);
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  delay(10);
  Serial.println("NodeMCU decoder test!");

  tft.begin();
  tft.setRotation(3);  // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_BLACK);
 // tft.invertDisplay(true);

  
  Serial.println("\r\nInitialisation done.");

}

//====================================================================================
//                                    Loop
//====================================================================================
  
void loop()
{
  tft.setRotation(3);  // portrait   
  
  GetPic();

 
}
//====================================================================================
