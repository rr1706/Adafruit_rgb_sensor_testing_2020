#include "Storage.h"

#if defined(__AVR_ATmega168__) ||defined(__AVR_ATmega168P__) ||defined(__AVR_ATmega328P__) ||defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

#include <EEPROM.h>
#define EEPROM_SIGNATURE 9962

void initStorage() {
  
}

void saveCalibratedColors(ColorSample targetColors[]) {
  int eepromSig = EEPROM_SIGNATURE;
    EEPROM.put(0, eepromSig);
    for (int i = 0; i < 4; i++) {
      ColorSample cs = targetColors[i];
      EEPROM.put(sizeof(int) + i * sizeof(ColorSample), cs);
    }
}

void readCalibratedValues(ColorSample targetColors[]) {
  int eepromSig = 0;
  EEPROM.get(0, eepromSig);
  if ( eepromSig == EEPROM_SIGNATURE ) {
    for (int i = 0; i < 4; i++) {
      ColorSample cs;
      EEPROM.get(sizeof(int) + i * sizeof(ColorSample), cs);
      targetColors[i].red = cs.red;
      targetColors[i].green = cs.green;
      targetColors[i].blue = cs.blue;
    }
  }
}

#else

#include <SPI.h>
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"

#if defined(__SAMD51__) || defined(NRF52840_XXAA)
  Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS, PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
#else
  #if (SPI_INTERFACES_COUNT == 1 || defined(ADAFRUIT_CIRCUITPLAYGROUND_M0))
    Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
  #else
    Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
  #endif
#endif

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

File myFile;

void initStorage() {
  // Init external flash
  flash.begin();

  // Init file system on the flash
  fatfs.begin(&flash);

  Serial.println("initialization done.");  
}

void saveCalibratedColors(ColorSample targetColors[]) {

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = fatfs.open("colors.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to colors.txt...");
    for (int i = 0; i < 4; i++) {
      myFile.print(targetColors[i].name);
      myFile.print(":");
      myFile.print(targetColors[i].red);
      myFile.print(" ");
      myFile.print(targetColors[i].green);
      myFile.print(" ");
      myFile.print(targetColors[i].blue);
      myFile.println();
    }
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening colors.txt");
  }

}

void readCalibratedValues(ColorSample targetColors[]) {
  // Init external flash
  flash.begin();

  // Init file system on the flash
  fatfs.begin(&flash);

  Serial.println("initialization done.");

  myFile = fatfs.open("colors.txt");
  String buf;
  String name;
  if (myFile) {
    Serial.println("colors.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char c = myFile.read();
      if (c > 13) {
        buf += c;
      } else {
        int nameEnd = buf.indexOf(":");
        int redEnd = buf.indexOf(" ");
        int greenEnd = buf.lastIndexOf(" ");
        if (nameEnd > 0) {
          name = buf.substring(0, nameEnd);
          int red = buf.substring(nameEnd + 1, redEnd).toInt();
          int green = buf.substring(redEnd + 1, greenEnd).toInt();
          int blue = buf.substring(greenEnd + 1).toInt();

          for (int i = 0; i < 4; i++) {
            if (name.equalsIgnoreCase(targetColors[i].name)) {
              targetColors[i].red = red;
              targetColors[i].green = green;
              targetColors[i].blue = blue;
            }
          }
        }
        buf = "";
      }
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening colors.txt");
  }

}

#endif
