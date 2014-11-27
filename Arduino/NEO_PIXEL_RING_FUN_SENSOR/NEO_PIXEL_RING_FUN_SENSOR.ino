// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
#include <Adafruit_NeoPixel.h>
#include "HSVColor.h";

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            6

// How many NeoPixels are attached to the Arduino?
const int NUM_PIXELS   =   16;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 200; // delay for half a second

HSVColori myColor(100,255,10); // h,s,v

void setup() {
  Serial.begin(57600);
  pixels.begin(); // This initializes the NeoPixel library.
}


int pixelIndex = 0;
int minData = 1024;
int maxData = 0;

void loop() {

  int data = analogRead(A1);
// calibrate data
  minData = min(minData, data);
  maxData = max(maxData, data);


  int val = map(data, minData, maxData, 0, 120);
  /*
  Serial.print("min:");
  Serial.print(minData);
  Serial.print(", max:");
  Serial.println(maxData);

  Serial.println(val);
*/
  myColor.v = val;

  //myColor.shiftHue(1);
  uint32_t c = myColor.toRGB();
  //Serial.println(c,BIN);

  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  pixels.setPixelColor(pixelIndex, c ); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.



  pixelIndex = (pixelIndex + 1) % NUM_PIXELS;

  delay(delayval); // Delay for a period of time (in milliseconds).
}

