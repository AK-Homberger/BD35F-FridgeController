# BD35F Fridge Controller
An electronic thermostat for a Danfoss/Secomp BD35F Compressor with ESP8266 (D1 Mini).

The Danfoss/Secomp BD35F compressor is a widely used DC powerd compressor. I'm using it on my boat to cool the fridge. Unfortunatel the original thermostat was defect. Therfore I decided to create an electronic alternative that has more fetuer the the standard mechanical one.

## Features:
- Uses ESP8266 (D1 Mini) as processor.
- Implements a Two-Poit with I controller to increase the accuracy.
- Supports variable compressor speed (2000 - 3500 RPM) to adjust cooling capacity and power consuption.
- Support Boost function to cool down with more power.
- Support automatic or manual Defrost routine.
- Wlan support (AP or client mode). Configurable via Wifi Manager.
- Browser interface and MDNS support (http://fridgecontrol-local).
- All parameters are configurable and stored in NVS of ESP8266
- PCB layout available in repository (KiCad). 

![Schematic](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController-Sch.png)

![PCB](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController/BD35F-FridgeController-3D.png)


