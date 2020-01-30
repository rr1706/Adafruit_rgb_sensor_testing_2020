#ifndef _COLORSAMPLE_H_
#define _COLORSAMPLE_H_

typedef struct ColorSample { 
  char  name[10];
  int   red;
  int   green;
  int   blue;
  int   out1;
  int   out2;
  int   dispRed;
  int   dispGreen;
  int   dispBlue;
} ColorSample;


ColorSample* findClosestColor(ColorSample* samples, int red, int green, int blue, int &distance);

#endif
