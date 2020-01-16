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
#include "Adafruit_NeoPixel.h"

#define NEO_PIN        6 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS     12 // Popular NeoPixel ring size

#define PIN_OUT1          8
#define PIN_OUT2          9
#define PIN_VALID        10

Adafruit_APDS9960 apds;
Adafruit_NeoPixel pixels(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel boardPixel(1, 40, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(PIN_OUT1, OUTPUT);
  pinMode(PIN_OUT2, OUTPUT);
  pinMode(PIN_VALID, OUTPUT);

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

  boardPixel.begin();
  boardPixel.setBrightness(127);
  boardPixel.clear();

}

void outputValues(int bit1, int bit2, int valid) {
  digitalWrite(PIN_OUT1, bit1); // Output bit 1
  digitalWrite(PIN_OUT2, bit2); // Output bit 2
  digitalWrite(PIN_VALID, valid);
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
    delay(5);
    //outputValues(0, 0);
    //boardPixel.setPixelColor(0, pixels.Color(0, 0, 0));
    //boardPixel.show();
    return;
  }
  
  //wait for color data to be ready
  while(!apds.colorDataReady()){
    delay(1);
  }

  //get the data and print the different channels
  apds.getColorData(&r, &g, &b, &c);

  // Normalize the values:
  int normRed   = (100 * r) / (r + g + b);
  int normGreen = (100 * g) / (r + g + b);
  int normBlue  = (100 * b) / (r + g + b);

  // print raw color values;
  Serial.print("red: ");
  Serial.print(normRed);
  
  Serial.print(" green: ");
  Serial.print(normGreen);
  
  Serial.print(" blue: ");
  Serial.print(normBlue);
  
  Serial.println();

  // This is using raw values, but we should switch to normalized values:
  if (normGreen - normRed > 20 && normGreen - normBlue > 5) {
    Serial.println("GREEN");
    outputValues(0, 1, 1);
    boardPixel.setPixelColor(0, pixels.Color(0, 100, 0));
    boardPixel.show();
  } else if (normRed - normGreen > 20 && abs(normGreen - normBlue) < 10) {
    Serial.println("RED");
    outputValues(0, 0, 1);
    boardPixel.setPixelColor(0, pixels.Color(100, 0, 0));
    boardPixel.show();
  } else if (abs(normRed - normGreen) < 5 && normGreen - normBlue > 10) {
    Serial.println("YELLOW");
    outputValues(1, 1, 1);
    boardPixel.setPixelColor(0, pixels.Color(50, 50, 0));
    boardPixel.show();
  } else if (normGreen - normRed > 10 && (normBlue - normGreen) > 10) {
    Serial.println("BLUE");
    outputValues(1, 0, 1);
    boardPixel.setPixelColor(0, pixels.Color(0, 0, 100));
    boardPixel.show();
  } else {
    outputValues(0, 0, 0);
    boardPixel.setPixelColor(0, pixels.Color(0, 0, 0));
    boardPixel.show();
  }

  //pixels.show();

  
  delay(5);
}
