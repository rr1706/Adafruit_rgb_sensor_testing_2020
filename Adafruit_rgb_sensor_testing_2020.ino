/***************************************************************************
  This is a library for the APDS9960 digital proximity, ambient light, RGB, and gesture sensor

  This sketch puts the sensor in color mode and reads the RGB and clear values.

  Designed specifically to work with the Adafruit APDS9960 breakout
  ----> http://www.adafruit.com/products/3595

  These sensors use I2C to communicate. The device's I2C address is 0x39

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

//the pin that the interrupt is attached to
#define INT_PIN 3


#include "Adafruit_APDS9960.h"

// Add neopixel support so we can illuminate the target.
#include <Adafruit_NeoPixel.h>

#define NEO_PIN        6 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS     16 // Popular NeoPixel ring size

Adafruit_APDS9960 apds;
Adafruit_NeoPixel pixels(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pinMode(INT_PIN, INPUT_PULLUP);

  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  else Serial.println("Device initialized!");

  //enable color sensign mode
  apds.enableColor(true);

  //enable proximity mode
  apds.enableProximity(true);

  //set the interrupt threshold to fire when proximity reading goes above 10
  apds.setProximityInterruptThreshold(0, 10);

  //enable the proximity interrupt
  apds.enableProximityInterrupt();

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(255);

  pixels.clear(); // Set all pixel colors to 'off'
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    // Here we're using a moderately color that compensates for the slightly 
    // blueish tint if we set them to all the same color:
    pixels.setPixelColor(i, pixels.Color(60, 80, 50));

  }
  pixels.show();   // Send the updated pixel colors to the hardware.

}

void loop() {
  //create some variables to store the color data in
  uint16_t r, g, b, c;

  // check for when the interrupt pin goes low
  if(!digitalRead(INT_PIN)){
    //clear the interrupt
    apds.clearInterrupt();
  } else {
    // If we're not close to anything, then just pause for a little bit 
    // and try again from the start.
    delay(50);
    return;
  }
  
  //wait for color data to be ready
  while(!apds.colorDataReady()){
    delay(5);
  }

  //get the data and print the different channels
  apds.getColorData(&r, &g, &b, &c);

  // print raw color values;
  Serial.print("red: ");
  Serial.print(r);
  
  Serial.print(" green: ");
  Serial.print(g);
  
  Serial.print(" blue: ");
  Serial.print(b);
  
  Serial.println();

  // Normalize the values:
  int normRed   = (100 * r) / (r + g + b);
  int normGreen = (100 * g) / (r + g + b);
  int normBlue  = (100 * b) / (r + g + b);

  // This is using raw values, but we should switch to normalized values:
  if (g - r > 20 && g - b > 5) {
    Serial.println("GREEN");
  }

  if (r - g > 20 && abs(g - b) < 10) {
    Serial.println("RED");
  }

  if (r - g < 5 && g - b > 20) {
    Serial.println("YELLOW");
  }

  if (g - r > 10 && abs(g - b) < 5) {
    Serial.println("BLUE");
  }

  
  delay(100);
}
