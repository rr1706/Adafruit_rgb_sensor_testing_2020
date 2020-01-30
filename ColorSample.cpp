
#include "ColorSample.h"
#include <stdlib.h>
#include <Arduino.h>

ColorSample* findClosestColor(ColorSample* samples, int red, int green, int blue, int &distance) {
  ColorSample *closest;
  int closestDistance = 10000;

  for (int i = 0; i < 4; i++) {
    int dr = abs(samples[i].red   - red);
    int dg = abs(samples[i].green - green);
    int db = abs(samples[i].blue  - blue);
    int dist = sqrt( dr * dr + dg * dg + db * db);
    /*
    Serial.print("\t\t\t");Serial.print(samples[i].name);
    Serial.print(" dr: "); Serial.print(dr);
    Serial.print(" dg: "); Serial.print(dg);
    Serial.print(" db: "); Serial.print(db);
    Serial.print(" dist: "); Serial.print(dist);
    Serial.println();
    */
    if (i == 0 || dist < closestDistance) {
      closest = &samples[i];
      closestDistance = dist;
    }
  }

  distance = closestDistance;
  return closest;
}
