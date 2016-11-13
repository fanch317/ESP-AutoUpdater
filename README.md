Update over the air on ESP8266
===

About
---

This updater will autoget new program on a URL by connecting WIFI when you press GPIO04 and GPIO05 to GND.
If the WiFi is not available, a AP WiFi is created for upload by a webpage on ESP.

I don't check version of program. So you must force update when you want overwrite you program.

Usage
---

Include autoUpdaterVendor.h and autoUpdaterVendor.cpp on you project.
Place this 2 lines on your arduino/platformIO code.

  \#include "autoUpdaterVendor.h"
  autoUpdater();


Special at boot
---

##### Reset to default settings AutoUpdater
* Hold GPIO04 and GPIO05 on normal boot
* Release GPIO04 and GPIO05 after 30s

##### Launch AutoGetDownloader
* Hold GPIO04 and GPIO05 on normal boot
* Release GPIO04 and GPIO05

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
