Update over the air on ESP8266
===

This updater will autoget new program on a URL by connecting WIFI.
If the WiFi is not available, a AP WiFi is created for upload by a webpage on ESP.

I don't check version of program. So you must force update when you want overwrite you program.

Usage on your arduino/platformIO code
---
1. Include lib
#include "autoUpdaterVendor.h"

2. Call updater on your setup function (or anywhere)
autoUpdater();


Special at boot
---

##### Reset to default settings AutoUpdater
  * Hold GPIO04 and GPIO05 on normal boot
  * Wait 30s until blue led is quickly blinking
  * Release when led stop blinking

##### Force to Launch AutoGetDownloader (when disabled)
 * Hold GPIO04 and GPIO05 on normal boot
 * Release GPIO04 and GPIO05 when led quickly blinking

Note : If you release only GPIO05, AutoUpdate will launch update by AutoGet
Note : If you release only GPIO04, AutoUpdate will launch update by Listener


Memo for programming ESP8266
---
  * GPIO0 at GND to programming
  * GPIO0 at VDD to normal BOOT

Assure at boot
  * CH_PD to VDD (also no boot/prog)
  * GPIO02 to VDD (also no boot/prod because boot in unknow verbose mode)
  * GPIO15 to GND (also no boot/prog)
