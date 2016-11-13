#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include <EEPROM.h>
#define USE_SERIAL Serial

/*
 * Config (420 Bytes)
 */
struct autoUpdaterConfig{
  //General (3 Bytes)
  char hostname[34];
  unsigned int dateProgram; //For future
  //For autoget (320 Bytes)
  char autoGetWifiEssid[32];
  char autoGetWifiPass[32];
  char autoGetUrl[256];
  unsigned char autoGetMaxAttempts;
  //For listener (66 Bytes)
  char listenerWifiEssid[32];
  char listenerWifiPass[32];
  unsigned int listenerTimeout;
};

/*
 * Address of config in eeprom
 */
#define AUTOUPDATER_EEPROMADDRESS 0 //Address of config in eeprom

/*
 * Globales options
 */

#define AUTOUPDATER_HOSTNAME "esp8266" //Default name on network bu mDNS

/*
 * Options for auto get last program from a AP WiFi
 */
#define AUTOUPDATER_AUTOGET_MAXATTEMPTS 10 //Default attempts to connect to wifi (10s)
#define AUTOUPDATER_AUTOGET_WIFIESSID "***REMOVED***" //Default essid
#define AUTOUPDATER_AUTOGET_WIFIPASS "***REMOVED***" //Default password
#define AUTOUPDATER_AUTOGET_URL "http://192.168.1.73/file.bin"


/*
 * Options for listen connexion for upload program
 */
#define AUTOUPDATER_LISTENER_WAITSECONDS 120 //Run program after 2 minuts without client
#define AUTOUPDATER_LISTENER_WIFIESSID "ESP8266" //Default essid
#define AUTOUPDATER_LISTENER_WIFIPASS "ESP8266" //Default wpa2 password , empty = none encryption


void autoUpdater();
int updateByAutoGet(autoUpdaterConfig configEeprom);
int listenUpdater(autoUpdaterConfig configEeprom);
void handleRoot();
void debugDisplayConfig(autoUpdaterConfig configEeprom);
