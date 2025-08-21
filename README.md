# BD35F Fridge Controller
An electronic thermostat for a Danfoss/Secomp BD35F Compressor with ESP8266 (D1 Mini).

The Danfoss/Secomp BD35F compressor is a widely used DC powerd compressor. I'm using it on my boat to cool the fridge. Unfortunately, the original thermostat was defect. Therfore, I decided to create an electronic alternative that has more features than the standard mechanical one.

## Features:
- Uses ESP8266 (D1 Mini) as processor.
- Easy upload of firmware without Arduino IDE possible.
- Temperature is measured with [DS18B20](https://encrypted-tbn0.gstatic.com/shopping?q=tbn:ANd9GcRNmu3D8hUJPGhuSA7nLRoa413b-gMmKO3SzPfPqZvY6wbsBIp207lLPlJ23y0RRw-2xIi-4CoqkTE98FSd--icjlcXm1W_YQ) sensor.
- Implements a Two-Point with I controller to increase the accuracy.
- Supports variable compressor speeds (2000 - 3500 RPM) to adjust cooling capacity and power consuption.
- Support Boost function to cool down with more power.
- Support automatic or manual Defrost routine.
- Wlan support (AP or client mode). Configurable via Wifi Manager.
- Browser interface and MDNS support (http://fridgecontrol-local).
- All parameters are configurable with browser and stored in NVS of ESP8266
- PCB layout available in repository (KiCad).


![Schematic](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController-Sch.png)

![PCB](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController/BD35F-FridgeController-3D.png)

## Hardware

The hardware is based on an ESP8266 (D1 Mini) and a few additional parts. The 5 Volt supply voltage for the D1 Mini is crated wit a step-down converter. 
The temperature is mesuered with a DS18B20 sensor. The compressor is controlled with an open collector transistor between the connection T and C on the BD35F. 

The BS35F is measuering the current between T and C. C is GND level and T has a pull-up resitor to 5 Volt. Only a few milliamper are flowing. A small transitor (BC337) is sufficient here. There are two options. T Permanently GND level means 2000 RPM speed. A PWM signal (5 KHz) with a defined duty cycle is used to set seeds from 2000 to 3500 RPM.

The PCB has three connectors:

### 1. Power 
Cconnect +/-to 12 Volt to the J1 connector. There is a protection against wrong cabling for +/-.

### 2. DS18B20 Temperature Sensor
Connect the three cables from the DS18B20 to the connector J2. The data line has already the pull-up resistor on the PCB.
Place the Sensor in the Fridge in middle highth.

### 3. BD35F Compressor
Connect J3 to the compressor connector. Remove the old thermostat cables and connect the cables C and  T to the compressor.
You need appropriate cable shoes for the connection.

## Software
There are two ways to install the software to the D1 Mini. Either you installan uste the Arduino IDE or you direcly flash the BIN-file to the device.

### Arduino IDE
Just download the IDE (version 2.x works well). Then add the Boad URL in preferences: "https://arduino.esp8266.com/stable/package_esp8266com_index.json" and install the Board software in the IDE. Then install the following libraries with the libray manager:

- WiFiManager
- ArduinoJson
- OneWire
- DallasTemperature
- Preferences

Then download the sketch from GitHud (Button "Code§ and download as ZIP) and extract it. Then move to the "FridgeControl.ino" file and open the sketch.
Connect the D1 mini with an USB-Cable to the PC and upload the sketch to the D1 Mini. Later uploades can be done via OTA function viy network.

### NodeMCU-PyFlasher
This works without installation of the IDE. Simply download the [NodeMCU-PyFlasher](https://github.com/nodemcu/nodemcu-firmware/releases) ans start it. 






## Usage

After upload of the firmware you have to re-power the device to start the Wifi-Manager (within 2 minutes after restart). Just look for a new WLAN natwork wirh the name "Fridgeconfig_AP" and connect to it, withot a password. The browser will start automativally. Then selct the network (ssid) you want to connect to an enter the password. 
If you don't configure or press "Exit" the device remain in AP mode with a changed nam "Fridgecontrol_AP".

You can use this to test the device without a real network to connect too. But this is an open connection don't use the permannetly.

To start the web interface enter "http://fridgecontrol.local" in the browser. The interface looks then like this:

![Web1](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/FridgeControlWeb1.png)




![Web2](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/FridgeControlWeb2.png)

