/***************************************************************************
  This is sample code to start using the APDS9660 sensor from Adafruit with 
  an FRC robot.
  
  This sketch puts the sensor in color mode and reads the RGB and clear values.

  Designed specifically to work with the Adafruit APDS9960 breakout
  ----> http://www.adafruit.com/products/3595

  These sensors use I2C to communicate. The device's I2C address is 0x39

  @NOTE: To use this code, install the Adafruit_APDS9960 and Adafruit_NeoPixel
         libraries using the Arduino IDE library manager.

 ***************************************************************************/

//the pin that the interrupt is attached to
#define INT_PIN 13

#include <Adafruit_APDS9960.h>

// Add neopixel support so we can illuminate the target.
#include <Adafruit_NeoPixel.h>
#include "ColorSample.h"
#include "Storage.h"

#define NEO_PIN        6 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS     16 // Popular NeoPixel ring size
#define ONBOARD_NEO_PIN  40

#define PIN_OUT1          8
#define PIN_OUT2          9
#define PIN_VALID        10

#define RED_CALIBRATE_PIN 2
#define GREEN_CALIBRATE_PIN 3
#define BLUE_CALIBRATE_PIN 4
#define YELLOW_CALIBRATE_PIN 5

Adafruit_APDS9960 apds;
Adafruit_NeoPixel pixels(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel boardPixel(1, ONBOARD_NEO_PIN, NEO_GRB + NEO_KHZ800);

long count = 0;
long timerStart = 0;
long readsAverage = 0;
long interruptTimeout = 0;
long goodCount = 0;

ColorSample targetColors[] = {
  {"Red", 63, 12, 24, 0, 0, 100, 0, 0},
  {"Green", 15, 44, 42, 0, 1, 0, 100, 0},
  {"Blue", 8, 32, 58, 1, 0, 0, 0, 100},
  {"Yellow", 36, 38, 25, 1, 1, 50, 50, 0}
  };

void outputValues(int bit1, int bit2, int valid) {
  digitalWrite(PIN_OUT1, bit1); // Output bit 1
  digitalWrite(PIN_OUT2, bit2); // Output bit 2
  digitalWrite(PIN_VALID, valid);
}

void checkForCalibration(int r, int g, int b) {
  boolean updated = false;
  if (!digitalRead(RED_CALIBRATE_PIN)) {
    Serial.println("Calibrating Red.");
    targetColors[0].red = r;
    targetColors[0].green = g;
    targetColors[0].blue = b;
    updated = true;
  } else if (!digitalRead(GREEN_CALIBRATE_PIN)) {
    Serial.println("Calibrating Green.");
    targetColors[1].red = r;
    targetColors[1].green = g;
    targetColors[1].blue = b;
    updated = true;
  } else if (!digitalRead(BLUE_CALIBRATE_PIN)) {
    Serial.println("Calibrating Blue.");
    targetColors[2].red = r;
    targetColors[2].green = g;
    targetColors[2].blue = b;
    updated = true;
  } else if (!digitalRead(YELLOW_CALIBRATE_PIN)) {
    Serial.println("Calibrating Yellow.");
    targetColors[3].red = r;
    targetColors[3].green = g;
    targetColors[3].blue = b;
    updated = true;
  }
  if (updated) {
    saveCalibratedColors(targetColors);
  }
}

  
void setup() {
  while (!Serial); // Wait for Serial port to initialize.
  Serial.begin(115200);
  pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(PIN_OUT1, OUTPUT);
  pinMode(PIN_OUT2, OUTPUT);
  pinMode(PIN_VALID, OUTPUT);

  pinMode(RED_CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(GREEN_CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(BLUE_CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(YELLOW_CALIBRATE_PIN, INPUT_PULLUP);

  Wire.setClock(1000000);

  Serial.println("Starting...");
  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  } else {
    Serial.println("Device initialized!");
  }

  //enable color sensign mode
  apds.enableColor(true);

  //enable proximity mode
  apds.enableProximity(true);

  //set the interrupt threshold to fire when proximity reading goes above 10
  apds.setProximityInterruptThreshold(0, 5);
  apds.setADCIntegrationTime(2);

  //enable the proximity interrupt
  apds.enableProximityInterrupt();

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(255);

  pixels.clear(); // Set all pixel colors to 'off'
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    // Here we're using a moderately color that compensates for the slightly 
    // blueish tint if we set them to all the same color:
    pixels.setPixelColor(i, 70, 60, 50);

  }
  pixels.show();   // Send the updated pixel colors to the hardware.

  Serial.println("NeoPixel Ring Started");
  boardPixel.begin();
  boardPixel.setBrightness(50);
  boardPixel.clear();

  initStorage();
  readCalibratedValues(targetColors);

  Serial.println("Setup complete.");
}


void loop() {
  //create some variables to store the color data in
  uint16_t r, g, b, c;

  // check for when the interrupt pin goes low
  if(!digitalRead(INT_PIN)){
    //clear the interrupt
    apds.clearInterrupt();
    interruptTimeout = 0;
  } else {
    // If we're not close to anything, then just pause for a little bit 
    // and try again from the start.
    delay(1);
    // If we've been far away for several cycles, then make sure the 
    // RoboRio knows.
    if (++interruptTimeout > 100) {
      outputValues(0, 0, 0);
    }
    return;
  }
  
  //wait for color data to be ready
  while(!apds.colorDataReady()){
    //delay(1);
  }

  long currentTime = millis();
  if (currentTime - timerStart > 1000) {
    readsAverage = count;
    count = 0;
    timerStart = currentTime;
  }
  
  //get the data and print the different channels
  apds.getColorData(&r, &g, &b, &c);
  count++;

  // Check to make sure we're seeing enough light coming in.
  if (c < 20) {
    // Too dark, tell the rio and bail out.
    outputValues(0, 0, 0);
    Serial.print("Too dark.\t");
    Serial.print(c);
    Serial.print(" ");
    Serial.println(r + g + b);
    delay(1);
    return;
  }
  
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

  Serial.print(" sample rate: ");
  Serial.print(readsAverage);

  int distance = 0;
  ColorSample *target = findClosestColor(targetColors, normRed, normGreen, normBlue, distance);
  if (distance < 5) {
    if (++goodCount > 3) {
      Serial.print("\t");Serial.print(target->name);Serial.print("  -->  "); Serial.print(distance);
      outputValues(target->out1, target->out2, 1);
    } else {
      outputValues(0, 0, 0);
    }
    boardPixel.setPixelColor(0, pixels.Color(target->dispRed, target->dispGreen, target->dispBlue));
    boardPixel.show();
  } else {
    goodCount = 0;
    outputValues(0, 0, 0);
    boardPixel.setPixelColor(0, 0, 0, 0);
    boardPixel.show();
  }
  Serial.println();

  checkForCalibration(normRed, normGreen, normBlue);
  
  delay(1);
}
