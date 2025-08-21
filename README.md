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
- Temperature is measured with DS18b20 sensor.

![Schematic](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController-Sch.png)

![PCB](https://github.com/AK-Homberger/BD35F-FridgeController/blob/main/BD35F-FridgeController/BD35F-FridgeController-3D.png)

## Hardware

The hardware is based on an ESP8266 (D1 Mini) and a few additional parts. The 5 Volt supply voltage for the D1 Mini is crated wit a step-down converter. 
The temperature is mesuered with a DS18B20 sensor. The compressor is controlled with an open collector transistor between the connection T and C on the BD35F. 

The BS35F is measuering the current flowing between T and C. C is GND level and T has a pull up resitor to 5 Volt. Only a few milliamper are flowing. A small transitor is sufficient here. There are two options. T Permanently GND level means 2000 RPM speed. A PWM signal (5 KHz) with a defined duty cycle is used to set seeds from 2000 to 3500 RPM.

The PCB has three connectors.

### Power 
Cconnect +/-to 12 Volt to the J1 connector. There is a protection against wrong cabling for +/-.

### DS18B20 Temperature Sensor

Connect the three cables from the DS18B20 to the connector J2. The data line has already the pull-up resistor on the PCB.
Place the Sensor in the Fridge in middle highth.

### BD35F Compressor

Conntect J3 to the compressor connector. Remove teh old thermostat cables and connect the cables C and  T to the compress or.
You need appropriate cable shoes for the connection.


