#include "autoUpdater.h"

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;


void debugDisplayConfig(autoUpdaterConfig configEeprom){
  USE_SERIAL.println("[AUTOUPDATER] EEPROM Values :");
  USE_SERIAL.printf("[AUTOUPDATER] dateProgram : %d\n",configEeprom.dateProgram);
  USE_SERIAL.printf("[AUTOUPDATER] hostname : %s\n",configEeprom.hostname);
  USE_SERIAL.printf("[AUTOUPDATER] autoGetWifiEssid : %s\n",configEeprom.autoGetWifiEssid);
  USE_SERIAL.printf("[AUTOUPDATER] autoGetWifiPass : %s\n",configEeprom.autoGetWifiPass);
  USE_SERIAL.printf("[AUTOUPDATER] autoGetMaxAttempts : %d\n",configEeprom.autoGetMaxAttempts);
  USE_SERIAL.printf("[AUTOUPDATER] listenerWifiEssid : %s\n",configEeprom.listenerWifiEssid);
  USE_SERIAL.printf("[AUTOUPDATER] listenerWifiPass : %s\n",configEeprom.listenerWifiPass);
  USE_SERIAL.printf("[AUTOUPDATER] listenerTimeout : %d\n",configEeprom.listenerTimeout);
}


/*
 *
 */
void autoUpdater(){

  autoUpdaterConfig configEeprom;

  USE_SERIAL.begin(115200);

  EEPROM.begin(512);

  EEPROM.get(AUTOUPDATER_EEPROMADDRESS, configEeprom );

  debugDisplayConfig(configEeprom);
  if(configEeprom.autoGetWifiEssid == 0){
    USE_SERIAL.println("[AUTOUPDATER] Initialize configEeprom with defaults values");
    resetConfigEeprom(configEeprom);
  }

  debugDisplayConfig(configEeprom);

  USE_SERIAL.printf("[AUTOUPDATER] dateProgram from eeprom: %d\n",configEeprom.dateProgram);

  pinMode(AUTOUPDATER_GPIOA, INPUT_PULLUP);
  pinMode(AUTOUPDATER_GPIOB, INPUT_PULLUP);

  //Check if GPIO is press
  if(digitalRead(AUTOUPDATER_GPIOA)==0 && digitalRead(AUTOUPDATER_GPIOB)==0){
    unsigned char timeout;
    bool exit = false;
    timeout = 10;
    do{
      USE_SERIAL.printf("[AUTOUPDATER] Control key is pressed. Reset config ins %d second(s)\n",timeout);
      delay(1000);
      if(digitalRead(AUTOUPDATER_GPIOA)!=0 || digitalRead(AUTOUPDATER_GPIOB)!=0 ){
        exit = true;
      }
      timeout--;
    } while(timeout > 0 && !exit);
    delay(500); //Wait for all release buttons


    /*
     * Reset config
     */
    if(digitalRead(AUTOUPDATER_GPIOA)==0 && digitalRead(AUTOUPDATER_GPIOB)==0){
      //Do a reset Eeprom config
      resetConfigEeprom(configEeprom);
      USE_SERIAL.println("[AUTOUPDATER] Do a update (AutoGet then Listener if fail)");
      int result = 0;
      //Update by autoget (connect to a wifi)
      result = updateByAutoGet(configEeprom);
      if(result != 0){
        //Listen to accept a new firmware by creating a new AP
        result = listenUpdater(configEeprom);
      }
    }

    /*
     * Update only by autoGet
     */
    if(digitalRead(AUTOUPDATER_GPIOA)==0 && digitalRead(AUTOUPDATER_GPIOB)!=0){
      USE_SERIAL.println("[AUTOUPDATER] Do a update by AutoGet");
      updateByAutoGet(configEeprom);
    }
    /*
     * Update only listener
     */
    if(digitalRead(AUTOUPDATER_GPIOA)!=0 && digitalRead(AUTOUPDATER_GPIOB)==0){
      USE_SERIAL.println("[AUTOUPDATER] Do a update by Listener");
      listenUpdater(configEeprom);
    }
    /*
     * Update normal (autoget then listener if fail)
     */
    else{
      USE_SERIAL.println("[AUTOUPDATER] Do a update (AutoGet then Listener if fail)");
      int result = 0;
      //Update by autoget (connect to a wifi)
      result = updateByAutoGet(configEeprom);
      if(result != 0){
        //Listen to accept a new firmware by creating a new AP
        result = listenUpdater(configEeprom);
      }
    }
  }

  listenUpdater(configEeprom);
}




/*
 * Reinitialize Eeprom config
 */
void resetConfigEeprom(autoUpdaterConfig configEeprom){
  USE_SERIAL.println("[AUTOUPDATER] Force to reset config with defaults values...");
  debugDisplayConfig(configEeprom);
  configEeprom.dateProgram = 0;
  strcpy(configEeprom.hostname, AUTOUPDATER_HOSTNAME);
  strcpy(configEeprom.autoGetWifiEssid, AUTOUPDATER_AUTOGET_WIFIESSID);
  strcpy(configEeprom.autoGetWifiPass, AUTOUPDATER_AUTOGET_WIFIPASS);
  strcpy(configEeprom.autoGetUrl, AUTOUPDATER_AUTOGET_URL);
  strcpy(configEeprom.listenerWifiEssid, AUTOUPDATER_LISTENER_WIFIESSID);
  strcpy(configEeprom.listenerWifiPass,  AUTOUPDATER_LISTENER_WIFIPASS);
  configEeprom.listenerTimeout = AUTOUPDATER_LISTENER_WAITSECONDS;
  EEPROM.put( AUTOUPDATER_EEPROMADDRESS, configEeprom );
  EEPROM.commit();
  debugDisplayConfig(configEeprom);
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
int updateByAutoGet(autoUpdaterConfig configEeprom) {

  USE_SERIAL.printf("[AUTOUPDATER] Set WiFi to %s...\n",configEeprom.autoGetWifiEssid);

  USE_SERIAL.printf("[AUTOUPDATER] dateProgram from eeprom: %d",configEeprom.dateProgram);

  WiFiMulti.addAP(configEeprom.autoGetWifiEssid, configEeprom.autoGetWifiPass);

  ESPhttpUpdate.rebootOnUpdate(true);

  int loopCheckUpdate = 1;

  do {

    USE_SERIAL.printf("[AUTOUPDATER] Attempt to check update [%d of %d]...\n", loopCheckUpdate, configEeprom.autoGetMaxAttempts);

    //WiFi connection required
    if ((WiFiMulti.run() == WL_CONNECTED)) {
      USE_SERIAL.println("[AUTOUPDATER] WiFi AP is connected !");

      //Run updateESP.getResetReason()
      t_httpUpdate_return ret = ESPhttpUpdate.update(configEeprom.autoGetUrl);

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

  }while(loopCheckUpdate <= configEeprom.autoGetMaxAttempts);

  USE_SERIAL.println("[AUTOUPDATER] Max attempt reached. Skip auto-update");

  return -2;
}

/**
 * Open a webserver for receive a file bin
 */
int listenUpdater(autoUpdaterConfig configEeprom) {

  String essid;

  //Bypass essid
  if(configEeprom.listenerWifiEssid != ""){
    essid = configEeprom.listenerWifiEssid;
  }else{
    //Generate ESSID
    byte macAddr[6];
    WiFi.macAddress(macAddr);
    String essid = "ESP_";
    for (int i = 0; i < 6; ++i) {
      if (macAddr[i]<0x10) {essid += "0";}
      essid += String(macAddr[i],HEX);
    }
  }

  //Parse String to char
  char AP_NameChar[essid.length()];
  for(int i=0; i<essid.length(); i++){
    AP_NameChar[i] = essid.charAt(i);
  }

  Serial.print("[AUTOUPDATER] Create access point: ");
  Serial.println(essid);

  WiFi.softAP(AP_NameChar);
  IPAddress myIP = WiFi.softAPIP();
  delay(5000);
  Serial.print("[AUTOUPDATER] Ip address: ");
  Serial.println(myIP);

  MDNS.begin(configEeprom.hostname);
  httpUpdater.setup(&httpServer);


  httpServer.on("/settings", HTTP_GET, [](){
      httpServer.send(200, "text/html", HTMLPAGESETTINGS);
    });
  httpServer.on("/", HTTP_GET, [](){
      httpServer.send(200, "text/html", HTMLPAGEINDEX);
    });

  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("[AUTOUPDATER] HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", configEeprom.hostname);


  unsigned int loopWaitTime = 0;
  while(loopWaitTime < AUTOUPDATER_LISTENER_WAITSECONDS){
    httpServer.handleClient();
    loopWaitTime++;
    delay(1000);
  }

  httpServer.close();
  httpServer.stop();
  WiFi.softAPdisconnect(true);
  Serial.println("[AUTOUPDATER] Timeout! No client connected to webserver! Exit updater");
  return 0;

}
