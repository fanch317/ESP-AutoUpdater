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
 * Address of config in eeprom
 */
#define AUTOUPDATER_EEPROMADDRESS 0 //Address of config in eeprom

/*
 * GPIO for force update
 */
#define AUTOUPDATER_GPIOA 4
#define AUTOUPDATER_GPIOB 5

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
#define AUTOUPDATER_AUTOGET_URL "http://domainname/file.bin"


/*
 * Options for listen connexion for upload program
 */
#define AUTOUPDATER_LISTENER_WAITSECONDS 120 //Run program after 2 minuts without client
#define AUTOUPDATER_LISTENER_WIFIESSID "Esp8266" //Default essid will be ESP_{MACADDRR}
#define AUTOUPDATER_LISTENER_WIFIPASS "ESP8266" //Default wpa2 password , empty = none encryption

#define HTMLPAGESETTINGS "<form method='POST' action='/settings' enctype='multipart/form-data'><input type='text' name='essid_name'><input type='submit' value='Update'></form>"
#define HTMLPAGEINDEX "<ul><li><a href='/settings'>Settings</a></li><li><a href='/update'>Update firmware</a></li></ul>"

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

void autoUpdater();
void resetConfigEeprom(autoUpdaterConfig configEeprom);
int updateByAutoGet(autoUpdaterConfig configEeprom);
int listenUpdater(autoUpdaterConfig configEeprom);
void handleRoot();
void debugDisplayConfig(autoUpdaterConfig configEeprom);
