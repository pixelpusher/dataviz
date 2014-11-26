// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
#include <Adafruit_NeoPixel.h>
#include "HSVColor.h";

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      16

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 1000; // delay for half a second

HSVColori myColor(100,255,10); // h,s,v

void setup() {
  Serial.begin(57600);
  pixels.begin(); // This initializes the NeoPixel library.
}

void loop() {
  

  //Serial.println(myColor.h);
  
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  for(int i=0;i<NUMPIXELS;i++){
    myColor.shiftHue(1);
    uint32_t c = myColor.toRGB();
    Serial.println(c,BIN);
    
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, c ); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(50);
  }
  //delay(delayval); // Delay for a period of time (in milliseconds).
}
