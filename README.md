# BD35F Fridge Controller
An electronic thermostat for a Danfoss/Secop [BD35F](https://assets.danfoss.com/documents/latest/178661/AI376451289296en-000101.pdf) Compressor with ESP8266 (D1 Mini).

![Secomp](https://www.secop.com/fileadmin/_processed_/3/d/csm_bd-p-housing-image-center_53edf1a5b6.png)

The Danfoss/Secop BD35F compressor is a widely used DC powered compressor. I'm using it on my boat to cool the fridge. Unfortunately, the original thermostat was defect. Therfore, I decided to create an electronic alternative that has more features than the standard mechanical one.

## Features:
- Uses ESP8266 (D1 Mini) as processor.
- Dual language support (English/German).
- Easy upload of firmware without Arduino IDE possible.
- Temperature is measured with [DS18B20](https://encrypted-tbn0.gstatic.com/shopping?q=tbn:ANd9GcRNmu3D8hUJPGhuSA7nLRoa413b-gMmKO3SzPfPqZvY6wbsBIp207lLPlJ23y0RRw-2xIi-4CoqkTE98FSd--icjlcXm1W_YQ) sensor.
- Implements a Two-Point with I controller to increase the accuracy.
- Supports variable compressor speeds (2000 - 3500 RPM) to adjust cooling capacity and power consumption.
- Supports Boost function to cool down with more power.
- Supports automatic or manual Defrost routine.
- WLAN support (AP or client mode). Configurable via Wifi Manager.
- Browser interface and MDNS support (http://fridgecontrol.local).
- All parameters are configurable with a browser and stored in flash memory of ESP8266.
- [PCB](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController/BD35F-FridgeController-PCB.pdf) layout is available in repository (KiCad) and can be orderd from [Aisler.net](https://aisler.net/p/VBBOGMUR).

![Schematic](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController-Sch.png)

![PCB](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController/BD35F-FridgeController-3D.png)

## Hardware
The hardware is based on an ESP8266 (D1 Mini) and a few additional parts. The 5 Volt supply voltage for the D1 Mini is crated wit a [step-down](https://www.reichelt.de/de/de/shop/produkt/dc_dc-wandler_r78e_5_v_500_ma_single-159161) converter. 
The temperature is measuered with a [DS18B20](https://www.analog.com/media/en/technical-documentation/data-sheets/ds18b20.pdf) sensor. The compressor is controlled with an open collector transistor between the connection T and C on the BD35F. 

The BS35F is measuering the current between T and C. C is on GND level and T has a pull-up resistor to 5 Volt. Only a few milliampere are flowing. A small transitor (BC337) is sufficient here. There are two options: T permanently on GND level means 2000 RPM speed. A PWM signal (5 KHz) with a defined duty-cycle is used to set speeds from 2000 to 3500 RPM.

The PCB has three connectors:

### 1. Power 
Connect +/-to 12 Volt to the J1 connector. There is a protection against wrong cabling for +/-.

### 2. DS18B20 Temperature Sensor
Connect the three cables from the DS18B20 to the connector J2. The data line has already the pull-up resistor on the PCB. Red = VDD, Yello = D, Blue = GND.
Place the Sensor in the Fridge in middle highth.

### 3. BD35F Compressor
Connect J3 to the compressor connector. Remove the old thermostat cables and connect the cables C and  T to the compressor. You need appropriate cable shoes for the connection.

## Software
There are two ways to install the software to the D1 Mini. Either you install the Arduino IDE or you direcly flash the BIN-file to the device.

### Arduino IDE
Just [download](https://www.arduino.cc/en/software/) the IDE (version 2.x works well). Then add the Boad URL in preferences: "https://arduino.esp8266.com/stable/package_esp8266com_index.json" and install the Board software in the IDE. Then install the following libraries with the library manager:

- WiFiManager
- ArduinoJson
- OneWire
- DallasTemperature
- Preferences

Then download the sketch from GitHud (Button "Code" and download as ZIP) and extract it. Then move to the [FridgeControl.ino](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/FridgeControl/FridgeControl.ino) file and open the sketch.
Connect the D1 mini with an USB-Cable to the PC and upload the sketch to the D1 Mini. Later uploads can be done with the OTA function via network.

### NodeMCU-PyFlasher
This works without installation of the IDE. Simply download the [NodeMCU-PyFlasher](https://github.com/nodemcu/nodemcu-firmware/releases) ans start it. 

![Flasher](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/Flasher.png)

Select appropriate Serial port and then move to the bin file in the deownloaded code. Use the seetings as shown fo the upload.

## Usage

After upload of the firmware you have to re-power the device to start the [Wifi-Manager](https://github.com/tzapu/WiFiManager) (within 2 minutes after restart). Just look for a new WLAN network with the name "Fridgeconfig_AP" and connect to it, without a password. The browser will start automatically. Then select the network (ssid) you want to connect to and enter the password. 

If you don't configure or press "Exit" the device remain in AP mode with a changed name "Fridgecontrol_AP".

You can use this to test the device without a real network to connect to. But this is an open connection. Don't use this permanently.

To start the web interface enter "http://fridgecontrol.local" in the browser. The interface looks then like this:

![Web1](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/FridgeControlWeb1.png)

The page shows the current fridge temeperature, the average temperature, the desired Set Temperature. Duty cycle shows the percentage of the on time compared to the last on and off time cycle. The values are also shown in minutes (On/Off). The status filed shows the staus of the controller (Auto, On, Off, Boost or Defrost).

With the slidebar and +/- you can modify the the set temperature.

With the buttos you can set the mode:

- Auto: Standard mode to control the temperature.
- ON: Switch on the compressor permanently.
- Off: Switch off the compressor permanently.
- Boost: Cool down with a different set point and hysteresis.
- Defrost: Manually start a dfrost cycle.
- Set: Opens a settings page.

![Web2](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/FridgeControlWeb2.png)

With the settings page you can control the behaviour of the controller without a need to recompile the code. The following settings are available:

- Language:   0=English 1= German language
- Hysteresis: Sets the Hysteresis/deviation for the two point controller. 2 means +/- 2 K. For a set temperature of 6 that means the temperature will between 4 and 8 degrees.
- RPM:         The desired speed of the compressor. Set as low as possible to reduce power consumption (2000 = 3,5A, 3500 = 7A). "0" means T permanently to GND and 2000 RPM (original mechanical thermostat behaviour).
- AVG Error:   This is the allowed deviation from the average. 0 means the compressor will cool down in every cycle until the average temperature reaches the set temperature. The average temperature is calculated with a low pass filter with K of 1/3600. Means average over about one hour.
- Max          High: The maximum high temperture. The compressor will be switched off, if ths level is reached. Independent of average temperature.
- Max Low:     The maximum low temperture. The compressor will be switched on, if ths level is reached. Independent of average temperature.
- Boost Temp.: Desired temperature for the boost function.
- Boost Hyst.: Hysteresis/deviation for the boost function.
- Boost Time   Time in minutes for the boost function.
- Boost RPM:   Compressor speed for the boost function.
- Max Runtime: Used to detect a needed defrost cycle. If the compressor runs for the defined time, a mandatory defrost cycle will start.
- Defrost Time: Set the time for the defrost cycle. Time depends on local fridge details.

After changes press the "Set" butten. The entered value will be checked against the allowed ranges. If everyting is OK you can use "Back" to go back to the main page.

# Parts
- PCB [Aisler](https://aisler.net/p/VBBOGMUR)
- U1 D1 Mini [Link](https://www.reichelt.de/de/en/d1-mini-esp8266-v3-0-d1-mini-p253978.html?&nbc=1)
- U2 Voltage regulator [Link](https://www.reichelt.de/de/de/shop/produkt/dc_dc-wandler_r78e_5_v_500_ma_single-159161)
- R1 Resistor 1 KOhm [Link](https://www.reichelt.de/de/en/carbon-film-resistor-1-4-w-5-1-0-kilo-ohms-1-4w-1-0k-p1315.html?&trstct=pos_2&nbc=1)
- R2 Resistor 4,7 KOhm [Link](https://www.reichelt.com/de/en/shop/product/carbon_film_resistor_1_4_w_5_4_7_kohm-1425)
- Q1 Transistor BC337 [Link](https://www.reichelt.de/de/en/transistor-to-92-bl-npn-45v-800ma-bc-337-25-dio-p219125.html?&nbc=1)
- D1 Diode 1N4001 [Link](https://www.reichelt.de/de/en/rectifier-diode-do41-50-v-1-a-1n-4001-p1723.html?&nbc=1)
- J1/J3 Connector 2-pin (*2) [Link](https://www.reichelt.de/de/en/2-pin-terminal-strip-spacing-5-08-akl-101-02-p36605.html?&nbc=1)
- J2 Connector 3-pin [Link](https://www.reichelt.com/de/en/shop/product/3-pin_terminal_strip_spacing_5_08-36606)
- DS18B20 [Link](https://www.reichelt.com/de/en/shop/product/shelly_temperature_sensor_ds18b20-287127)

