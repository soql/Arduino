// Henry's Bench Hello World - for use with Monochrome OLEDs

#include <U8glib.h>


//**************************************************
// Change this constructor to match your display!!!
U8GLIB_SH1106_128X64 u8g(4, 5, 6, 7);
//**************************************************

void setup() {  
  u8g.setFont(u8g_font_unifont);
  u8g.setColorIndex(1); // Instructs the display to draw with a pixel on. 
}

void loop() {  
  u8g.firstPage();
  do {  
    draw();
  } while( u8g.nextPage() );
  delay(1000);   
}
  
void draw(){
  u8g.drawStr( 0, 20, "Hello World");
    
}
