
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


// Add neopixel support so we can illuminate the target.
#include <Wire.h>
#include <VL53L1X.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_APDS9960.h>
#include "ColorSample.h"
#include "Storage.h"
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library

#define OLED_PIN_RESET 9
#define OLEED_DC_JUMPER 1

#define ONBOARD_NEO_PIN  13

#define PIN_OUT1          8
#define PIN_OUT2          9
#define PIN_VALID        10

#define RED_CALIBRATE_PIN 2
#define GREEN_CALIBRATE_PIN 3
#define BLUE_CALIBRATE_PIN 4
#define YELLOW_CALIBRATE_PIN 5
#define NUM_LIDAR 4

Adafruit_APDS9960 apds;
Adafruit_NeoPixel boardPixel(1, ONBOARD_NEO_PIN, NEO_GRB + NEO_KHZ800);
MicroOLED oled(OLED_PIN_RESET, OLEED_DC_JUMPER);    // I2C declaration
VL53L1X sensor[NUM_LIDAR];

long count = 0;
long timerStart = 0;
long readsAverage = 0;
long interruptTimeout = 0;
long goodCount = 0;

bool isOnRobot = true;

ColorSample targetColors[] = {
  {"Red",    45, 22, 31, 0, 0, 100, 0,   0},
  {"Green",  25, 37, 37, 0, 1, 0,   100, 0},
  {"Blue",   18, 34, 46, 1, 0, 0,   0,   100},
  {"Yellow", 39, 31, 28, 1, 1, 50,  50,  0}
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
    displayTargets();
  }
}

void displayTargets() {
  char buffer[50];
  oled.clear(PAGE);     // Clear the screen
  oled.setFontType(0);  // Set font to type 0
  oled.setCursor(0, 0); // Set cursor to top-left
  oled.println("  R  G  B");
  for (byte i = 0; i < 4; i++) {
    sprintf(buffer, "%c %02i %02i %02i", targetColors[i].name[0], targetColors[i].red, targetColors[i].green, targetColors[i].blue);
    oled.print(buffer);
  }
  oled.display();       // Refresh the display
}

void setup() {
  while (!Serial); // Wait for Serial port to initialize.
  Serial.begin(115200);
  Wire.begin();

  isOnRobot = checkForOnRobot();

  pinMode(PIN_OUT1, OUTPUT);
  pinMode(PIN_OUT2, OUTPUT);
  pinMode(PIN_VALID, OUTPUT);
  pinMode(RED_CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(GREEN_CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(BLUE_CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(YELLOW_CALIBRATE_PIN, INPUT_PULLUP);

  Wire.setClock(400000);
  //delay(1000);

  Serial.println("Starting...");
  if (!apds.begin()) {
    Serial.println("failed to initialize color sensing device! Please check your wiring.");
  } else {
    Serial.println("Device initialized!");
  }

  //enable color sensign mode
  apds.enableColor(true);
  apds.setADCGain(APDS9960_AGAIN_16X);
  apds.setADCIntegrationTime(4);

  boardPixel.begin();
  boardPixel.setBrightness(50);
  boardPixel.clear();
  boardPixel.setPixelColor(0, 255, 0, 255);
  boardPixel.show();

  oled.begin();      // Initialize the OLED
  oled.clear(ALL);   // Clear the display's internal memory
  oled.display();    // Display what's in the buffer (splashscreen)
  delay(500);        // Delay
  oled.clear(PAGE);  // Clear the buffer.


  initStorage();
  readCalibratedValues(targetColors);
  displayTargets();

  setMuxPort(0);  // Start by turning off all mux ports.
  for (byte i = 0; i < NUM_LIDAR; i++) {
    if (enableMuxPort(i)) {
      sensor[i].setTimeout(500);
      if (!sensor[i].init()) {
        Serial.print("Failed to detect and initialize sensor: "); Serial.println(i);
      }
      sensor[i].setDistanceMode(VL53L1X::Long);
      sensor[i].setMeasurementTimingBudget(33000);
      sensor[i].startContinuous(33);
      disableMuxPort(i);
    } else {
      Serial.print("failed to set mux port");
    }
  }

  Serial.println("Setup complete.");
}

void setRingColor(byte r, byte g, byte b) {
  Wire.beginTransmission(44); // transmit to device #44
  Wire.write(r);              // sends one byte
  Wire.write(g);              // sends one byte
  Wire.write(b);              // sends one byte
  Wire.endTransmission();     // stop transmitting
}

void sendDistance(int sensorNum, int inches) {
  Wire.beginTransmission(55);  // transmit to device #44
  Wire.write((byte)sensorNum); // sends one byte
  Wire.write((byte)inches);    // sends one byte
  Wire.endTransmission();      // stop transmitting
}

int lastDistInches = 0;

void loop() {
  //create some variables to store the color data in
  uint16_t r, g, b, c;

  for (int i = 0; i < NUM_LIDAR; i++) {
    enableMuxPort(i);
    if (sensor[i].dataReady()) {
      int distMM = sensor[i].read(false);  // Read but don't block.
      int distInches = (int)(((double)distMM) / 25.4);

      debug("Sensor %i = %i mm\n", i, distMM);

      if (distInches != lastDistInches) {
        lastDistInches = distInches;
        sendDistance(i, lastDistInches);
        debug("Sensor %i is %i inches\n", i, distInches); 
      }
    }
    disableMuxPort(i);
  }

  //wait for color data to be ready
  if (apds.colorDataReady()) {

    long currentTime = millis();
    if (currentTime - timerStart > 1000) {
      readsAverage = count;
      count = 0;
      timerStart = currentTime;
      setRingColor(230, 220, 100);
    }

    //get the data and print the different channels
    apds.getColorData(&r, &g, &b, &c);
    count++;

    // Check to make sure we're seeing enough light coming in.
    if (c < 20) {
      // Too dark, tell the rio and bail out.
      outputValues(0, 0, 0);
      debug("Too dark.\t c: %i r: %i g: %i b: %i sum: %i\n", c, r, g, b, r + g + b);
      delay(1);
      //return;
    }

    // Normalize the values:
    int normRed   = (100 * r) / (r + g + b);
    int normGreen = (100 * g) / (r + g + b);
    int normBlue  = (100 * b) / (r + g + b);

    // print raw color values;
    debug("red: %i (%i) %i (%i) %i (%i) sample rate: %i", normRed, r, normGreen, g, normBlue, b, readsAverage);

    int distance = 0;
    ColorSample *target = findClosestColor(targetColors, normRed, normGreen, normBlue, distance);
    if (distance < 15) {
      if (++goodCount > 3) {
        debug("\t%s  -->  %i", target->name, distance);
        outputValues(target->out1, target->out2, 1);
      } else {
        outputValues(0, 0, 0);
      }
      boardPixel.setPixelColor(0, boardPixel.Color(target->dispRed, target->dispGreen, target->dispBlue));
      boardPixel.show();
    } else {
      goodCount = 0;
      outputValues(0, 0, 0);
      boardPixel.setPixelColor(0, 0, 0, 0);
      boardPixel.show();
    }
    debug("\n");

    checkForCalibration(normRed, normGreen, normBlue);
  }
  
  checkForUserInput();
  delay(10);
}

bool checkForOnRobot() {
  Wire.beginTransmission(0X3D);  // Check for the small OLED screen
  return 0 != Wire.endTransmission();
}
