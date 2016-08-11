#include <Arduino.h>
  
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <EEPROM.h>

#define USE_SERIAL Serial

#define REMOTECHECKUPDATE_MAXATTEMPTS 10 //Check every seconds, 0 for no boot-update
#define REMOTECHECKUPDATE_WIFISSID "***REMOVED***"
//#define REMOTECHECKUPDATE_WIFISSID "Thomson9CBE25"
#define REMOTECHECKUPDATE_WIFIPASS "***REMOVED***"
#define REMOTECHECKUPDATE_URL "http://192.168.1.71/file.bin"
#define REMOTECHECKUPDATE_EEPROM 0 //Address of config in eeprom (194 bytes)


int checkUpdate();
