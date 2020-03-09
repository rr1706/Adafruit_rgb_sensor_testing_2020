char serialCommandBuffer[100] = "";
bool debugging = false;

void debug(char *format, ...) {
  if (debugging) {
    char buffer[127];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    Serial.print(buffer);
    va_end(args);
  }
}


void printTargetColors() {
  for (int i = 0; i < 4; i++) {
    char buf[50];
    sprintf(buf, "%s %i %i %i", targetColors[i].name, targetColors[i].red, targetColors[i].green, targetColors[i].blue);
    Serial.println(buf);
  }
}

void checkForUserInput() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    char *name;
    int r, g, b;

    if (c < 32) {
      if (!strcmp(serialCommandBuffer, "colors")) {
        printTargetColors();
      } else if (startsWith(serialCommandBuffer, "debug")) {
        debugging = !debugging;
      } else {
        char * pch = strtok (serialCommandBuffer, " ");
        if (pch != NULL) {
          name = pch;
          r = atoi(strtok (NULL, " "));
          g = atoi(strtok (NULL, " "));
          b = atoi(strtok (NULL, " "));

          for (int i = 0; i < 4; i++) {
            if (!strcmp(name, targetColors[i].name)) {
              targetColors[i].red = r;
              targetColors[i].green = g;
              targetColors[i].blue = b;
              saveCalibratedColors(targetColors);
              Serial.println("Color saved.");
            }
          }
        }
      }
      serialCommandBuffer[0] = '\0';
    } else {
      int l = strlen(serialCommandBuffer);
      serialCommandBuffer[l] = c;
      serialCommandBuffer[l + 1] = '\0';
    }
  }
}


bool startsWith(const char *str, const char *pre)
{
  size_t lenpre = strlen(pre),
         lenstr = strlen(str);
  return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}
