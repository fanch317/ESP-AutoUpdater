/**
   httpUpdate.ino

    Created on: 27.11.2015

*/

#include <Arduino.h>

#include "autoUpdaterVendor.h"


void setup() {

  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);
  
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  USE_SERIAL.flush();
  
  autoUpdater();

}


void loop() {
 
  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  while (true) {
    USE_SERIAL.println("run");
    //digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
    //digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    delay(5000);
  }
  
}

