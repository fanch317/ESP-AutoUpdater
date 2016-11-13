#include "autoUpdaterVendor.h"

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

const char* host = "esp8266-webupdate";

struct autoUpdaterConfig{
  unsigned int currentVersion;
  char essid[32];
  char pass[32];
  char url[128];
};

void autoUpdater(){
  int result = 0;
  //result = checkUpdater();
  result = listenUpdater();
}

/*
 * Auto update from a file on Wi-Fi
 *  - Connect to a AP WiFI
 *  - Get a dhcp IP
 *  - Retreive and update bin file
 *  - Retry X times
 *
 *  Return:

 *    - 0: Update success (normaly never return because immediatly reset)
 *    - -1: No update available returned by server
 *    - -2: Fail to connect to wifi
 *    - -1XX: Error returned by server (-102: File not found), see ESP8266httpUpdate.h for details
 */
int checkUpdater() {

  USE_SERIAL.begin(115200);

  EEPROM.begin(512);

  autoUpdaterConfig configEeprom;
  EEPROM.get(REMOTECHECKUPDATE_EEPROM, configEeprom );

  if(configEeprom.essid == 0){
    USE_SERIAL.println("[AUTOUPDATER] Initialize configEeprom with defaults values");
    configEeprom.currentVersion = 1;
    strcpy(configEeprom.essid, "***REMOVED***");
    strcpy(configEeprom.pass, "***REMOVED***");
    strcpy(configEeprom.url, "http://192.168.1.71/file.bin");
    EEPROM.put( REMOTECHECKUPDATE_EEPROM, configEeprom );
    EEPROM.commit();
  }

  USE_SERIAL.printf("[AUTOUPDATER] currentVersion from eeprom: %d",configEeprom.currentVersion);


  USE_SERIAL.printf("[AUTOUPDATER] Set WiFi to %S...\n",REMOTECHECKUPDATE_WIFISSID);
  WiFiMulti.addAP(REMOTECHECKUPDATE_WIFISSID, REMOTECHECKUPDATE_WIFIPASS);

  ESPhttpUpdate.rebootOnUpdate(true);

  int loopCheckUpdate = 1;

  do {

    USE_SERIAL.printf("[AUTOUPDATER] Attempt to check update [%d of %d]...\n", loopCheckUpdate, REMOTECHECKUPDATE_MAXATTEMPTS);

    //WiFi connection required
    if ((WiFiMulti.run() == WL_CONNECTED)) {
      USE_SERIAL.println("[AUTOUPDATER] WiFi AP is connected !");

      //Run updateESP.getResetReason()
      t_httpUpdate_return ret = ESPhttpUpdate.update(REMOTECHECKUPDATE_URL);

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          USE_SERIAL.printf("[AUTOUPDATER] HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          return ESPhttpUpdate.getLastError();
          break;

        case HTTP_UPDATE_NO_UPDATES:
          USE_SERIAL.println("[AUTOUPDATER] HTTP_UPDATE_NO_UPDATES");
          return -1;
          break;

        case HTTP_UPDATE_OK:
          USE_SERIAL.println("[AUTOUPDATER] HTTP_UPDATE_OK");
          return 0;
          break;
      }
    }else{
      USE_SERIAL.println("[AUTOUPDATER] Wifi AP not connected");
    }

    delay(1000);
    loopCheckUpdate++;

  }while(loopCheckUpdate <= REMOTECHECKUPDATE_MAXATTEMPTS);

  USE_SERIAL.println("[AUTOUPDATER] Max attempt reached. Skip auto-update");

  return -2;
}

/**
 * Open a webserver for receive a file bin
 */
int listenUpdater() {

  USE_SERIAL.begin(115200);

  byte macAddr[6];
  WiFi.macAddress(macAddr);

  //Generate ESSID
  String essid = "ESP_";
  for (int i = 0; i < 6; ++i) {
    if (macAddr[i]<0x10) {essid += "0";}
    essid += String(macAddr[i],HEX);
  }
  //essid.toUpperCase();
  //Parse String to char
  char AP_NameChar[essid.length()];
  for(int i=0; i<essid.length(); i++){
    AP_NameChar[i] = essid.charAt(i);;
  }

  Serial.print("[AUTOUPDATER] Create access point: ");
  Serial.println(essid);

  WiFi.softAP(AP_NameChar);
  IPAddress myIP = WiFi.softAPIP();
  delay(5000);
  Serial.print("[AUTOUPDATER] Ip address: ");
  Serial.println(myIP);

  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("[AUTOUPDATER] HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);

  unsigned int loopWaitTime = 0;
  while(loopWaitTime < REMOTECHECKUPDATE_WAITSECONDS){
    httpServer.handleClient();
    loopWaitTime++;
    delay(1000);
  }

  httpServer.close();
  httpServer.stop();
  WiFi.softAPdisconnect(true);
  Serial.println("[AUTOUPDATER] Timeout ! Exit updater");
  return 0;

}
