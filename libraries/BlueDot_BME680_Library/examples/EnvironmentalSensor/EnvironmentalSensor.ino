
/***************************************************************************
  Example for BME680 Weather Station
  written by Thiago Barros for BlueDot UG (haftungsbeschränkt)
  BSD License

  This sketch was written for the Bosch Sensor BME680.
  The BME680 is a MEMS device for measuring temperature, humidity and atmospheric pressure.
  For more technical information on the BME680, please go to ------> http://www.bluedot.space

  DISCLAIMER: 
  This Sketch is based on an UNFINISHED library!
  With the current version (18 February 20118) of the BlueDot BME680 library you can read temperature, pressure, humidity and gas sensor resistance.
  However only through the I2C interface.
  For more details, please refer to the comments on the ".cpp" and ".h" files.

 ***************************************************************************/


#include <Wire.h>
#include <avr/wdt.h>
#include "BlueDot_BME680.h"
BlueDot_BME680 bme680 = BlueDot_BME680();


void setup() {
  Serial.begin(9600);
  Serial.println(F("Weather Station and Gas Sensor"));

  //*********************************************************************
  //*************BASIC SETUP - READ BEFORE GOING ON!*********************
    
  //Set the I2C address of your breakout board  
  
  //0x76:       Alternative I2C Address (SDO pin connected to GND)
  //0x77:       Default I2C Address (SDO pin unconnected)
    Wire.begin();     
    bme680.parameter.I2CAddress = 0x77;                  //Choose I2C Address

    

  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Now the device will be set to forced mode
  //This is the default setup
  //Please note that unlike the BME280, BME680 does not have a normal mode

  //0b01:     In forced mode a single measured is performed and the device returns automatically to sleep mode
  
    bme680.parameter.sensorMode = 0b01;                   //Default sensor mode

    
                                                        
  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Great! Now set up the internal IIR Filter
  //The IIR (Infinite Impulse Response) filter suppresses high frequency fluctuations
  //In short, a high factor value means less noise, but measurements are also less responsive
  //You can play with these values and check the results!
  //In doubt just leave on default

  //0b000:      factor 0 (filter off)
  //0b001:      factor 1
  //0b010:      factor 3
  //0b011:      factor 7
  //0b100:      factor 15 (default value)
  //0b101:      factor 31
  //0b110:      factor 63
  //0b111:      factor 127 (maximum value)

    bme680.parameter.IIRfilter = 0b100;                                   //Setting IIR Filter coefficient to 15 (default)


  
  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************

  //Next you'll define the oversampling factor for the humidity measurements
  //Again, higher values mean less noise, but slower responses
  //If you don't want to measure humidity, set the oversampling to zero

  //0b000:      factor 0 (Disable humidity measurement)
  //0b001:      factor 1
  //0b010:      factor 2
  //0b011:      factor 4
  //0b100:      factor 8
  //0b101:      factor 16 (default value)

    bme680.parameter.humidOversampling = 0b101;                           //Setting Humidity Oversampling to factor 16 (default) 



  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Now define the oversampling factor for the temperature measurements
  //You know now, higher values lead to less noise but slower measurements
  
  //0b000:      factor 0 (Disable temperature measurement)
  //0b001:      factor 1
  //0b010:      factor 2
  //0b011:      factor 4
  //0b100:      factor 8
  //0b101:      factor 16 (default value)

    bme680.parameter.tempOversampling = 0b101;                            //Setting Temperature Oversampling factor to 16 (default)
    


  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Finally, define the oversampling factor for the pressure measurements
  //For altitude measurements a higher factor provides more stable values
  //On doubt, just leave it on default
  
  //0b000:      factor 0 (Disable pressure measurement)
  //0b001:      factor 1
  //0b010:      factor 2
  //0b011:      factor 4
  //0b100:      factor 8
  //0b101:      factor 16 (default value)

    bme680.parameter.pressOversampling = 0b101;                           //Setting Pressure Oversampling to factor 16 (default) 
  

  

  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //For precise altitude measurements please put in the current pressure corrected for the sea level
  //On doubt, just leave the standard pressure as default (1013.25 hPa)
  
    bme680.parameter.pressureSeaLevel = 1013.25;                          //default value of 1013.25 hPa

  //Now write here the current average temperature outside (yes, the outside temperature!)
  //You can either use the value in Celsius or in Fahrenheit, but only one of them (comment out the other value)
  //In order to calculate the altitude, this temperature is converted by the library into Kelvin
  //For slightly less precise altitude measurements, just leave the standard temperature as default (15°C)
  //Remember, leave one of the values here commented, and change the other one!
  //If both values are left commented, the default temperature of 15°C will be used
  //But if both values are left uncommented, then the value in Celsius will be used    
  
    bme680.parameter.tempOutsideCelsius = 15;                            //default value of 15°C
  //bme680.parameter.tempOutsideFahrenheit = 59;                         //default value of 59°F
  
    
  
  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
   
  //Here we need to set the target temperature of the gas sensor hot plate
  //According to the datasheet, the target temperature is typically set between 200°C and 400°C
  //The default value is 320°C
  
    bme680.parameter.target_temp = 320;



  //*********************************************************************
  //*************ADVANCED SETUP IS OVER - LET'S CHECK THE CHIP ID!*******

  if (bme680.init() != 0x61)
  {    
    //If the Arduino fails to identify the BME680, program stops and shows the Troubleshooting guide
      
    
    Serial.println(F("Ops! BME680 could not be found!"));
    Serial.println(F("Please check your connections."));
    Serial.println();
    Serial.println(F("Troubleshooting Guide"));
    Serial.println(F("*************************************************************"));
    Serial.println(F("1. Let's check the basics: Are the VCC and GND pins connected correctly? If the BME680 is getting really hot, then the wires are crossed."));
    Serial.println();
    Serial.println(F("2. Are you using the I2C mode? Did you connect the SDI pin from your BME680 to the SDA line from the Arduino?"));
    Serial.println();
    Serial.println(F("3. And did you connect the SCK pin from the BME680 to the SCL line from your Arduino?"));
    Serial.println();
    Serial.println(F("4. Are you using the alternative I2C Address(0x76)? Did you remember to connect the SDO pin to GND?"));
    Serial.println();
    Serial.println(F("5. If you are using the default I2C Address (0x77), did you remember to leave the SDO pin unconnected?"));
    Serial.println();
    Serial.println(F("6. Are you using the SPI mode? Did you connect the Chip Select (CS) pin to the pin 10 of your Arduino (or to wherever pin you choosed)?"));
    Serial.println();
    Serial.println(F("7. Did you connect the SDI pin from the BME680 to the MOSI pin from your Arduino?"));
    Serial.println();
    Serial.println(F("8. Did you connect the SDO pin from the BME680 to the MISO pin from your Arduino?"));
    Serial.println();
    Serial.println(F("9. And finally, did you connect the SCK pin from the BME680 to the SCK pin from your Arduino?"));
    Serial.println();
    
    while(1);
  }   
    else
  {
    Serial.println(F("BME680 detected!"));
  }
  Serial.println();
  Serial.println();

}
  //*********************************************************************
  //*************NOW LET'S START MEASURING*******************************
void loop()
{

  bme680.writeCTRLMeas();

  Serial.print(F("Duration in Seconds:\t\t"));
  Serial.println(millis()/1000);  

  Serial.print(F("Temperature in Celsius:\t\t")); 
  Serial.println(bme680.readTempC());

  Serial.print(F("Humidity in %:\t\t\t")); 
  Serial.println(bme680.readHumidity());

  Serial.print(F("Pressure in hPa:\t\t")); 
  Serial.println(bme680.readPressure());

  Serial.print(F("Altitude in Meters:\t\t"));
  Serial.println(bme680.readAltitudeMeter());

  Serial.print(F("Gas Resistance in Ohms:\t\t"));
  Serial.println(bme680.readGas());
  Serial.println();

  
  delay(2000);


}
