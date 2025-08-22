/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// Fridge Control for Danfoss/Secomp BD35F with WLAN and compressor speed setting
// Implements Two-Point with I controller to improve precision of temperature control

// DS18B20 temperature sensor on D5 pin, Relay / PWM output on pin D6
// Use 4k7 Ohm resitor as Pull-Up on D5 (to 3,3 Volt)
// Supports relay output (On/Off) or PWM with NPN transistor to control compressor speed between 2000 and 3500 RPM
// Set PWM to 0 for relay mode
// Connect Relay output (NO) or NPN Transitor (Open Collector) between T and C connection (C = GND, T at 5 Volt level with resistor)
// Current between T and C is only a few mA. A small transistor (eg. BC337) is sufficient. Use 1 kOhm between output pin and basis.

// Version 1.0, 20.08.2025, AK-Homberger

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

// External libraries. Install via Library Manager in IDE
#include <WiFiManager.h>    // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>

// Local include
#include "fridge_html_de.h"  // HTML and Javascript code German
#include "fridge_html_en.h"  // HTML and Javascript code English

#define On true
#define Off false

// GPIOs
const int SensorPin = D5;  // DS1820 Sensor
const int FridgePin = D6;  // Relay pin or PWM pin with NPN Transistor

// Put IP address details here
IPAddress AP_local_ip(192, 168, 4, 1);  // Static address for AP
IPAddress AP_gateway(192, 168, 4, 1);
IPAddress AP_subnet(255, 255, 255, 0);

// Fridge
unsigned Language = 0;                  // Language 0=English 1=German
unsigned CurrentRPM = 0;                // Initial RPM. Current value is stored in NVS. 0 = Relay mode
double FridgeTemp = -100;               // Measured fridge temperature
double FridgeTempAvg = -100;            // Average temperature over about one hour (low pass filter)
double FridgeTempLevel = 6;             // Initial temperature level. Current value is stored in NVS
double FridgeHyst = 2;                  // Hysteresis / 2. Max deviation from set temperaure level
double FridgeMaxHigh = 10;              // Maximum temperature. Compressor will switch on above this temperature
double FridgeMaxLow = 2;                // Minimum temperature. Compressor will switch off below this temperature
double FridgeAvgError = 0.1;            // Maximum allowed deviation from average temperatur (forms I part of the control)
bool FridgeAuto = true;                 // Fridge in automatic mode
bool FridgeBoost = false;               // Fridge in boost mode
bool FridgeDefrost = false;             // Fridge in defrost mode
unsigned FridgeDefrostTime = 20;        // Defrost time
unsigned FridgeCurrentDefrostTime = 0;  // Current defrost time
unsigned FridgeMaxRuntime = 60;         // Maximum runtime for compresser. Then defrost
unsigned FridgeCurrentBoostTime = 0;    // Current boost time (counts up to boost time)
double FridgeBoostTemp = 5;             // Fridge boost temperatur (with fixed hysteresis/deviation of 1 K)
double FridgeBoostHyst = 1;             // Set hsyteresis for boost
unsigned FridgeBoostTime = 60;          // Set boost time
unsigned FridgeBoostRPM = 3000;         // Set boost speed for compressor
unsigned FridgeOnTime = 0;              // Current on time of compressor. Counts up.
unsigned FridgeOffTime = 0;             // Current off time of compressor. Counts up.
unsigned FridgeLastOnTime = 0;          // Last on time of compressor to calculate duty cycle
unsigned FridgeLastOffTime = 0;         // Last off time of compressor to calculate duty cycle
bool FridgeOnOff = Off;                 // Current compessor state
bool f_update = false;                  // NVS values changed. Store new values.

// Web Server
ESP8266WebServer web_server(80);  // Web Server at port 80

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(SensorPin);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Nonvolatile storage on ESP
Preferences preferences;

//*****************************************************************************
void setup() {
  // Sensor and Thermostat pin
  // pinMode(SensorPin, INPUT_PULLUP);  // Comment out if you use an external pull up resistor
  pinMode(FridgePin, OUTPUT);        // Sets relay/PWM pin as output
  analogWriteFreq(5000);             // BD35F expects 5 kHz for PWM with duty cycle control

  // Init serial
  Serial.begin(115200);
  Serial.println();

  WiFiManager wm;
  wm.setConfigPortalTimeout(120);
  
  // Set HTTP request events
  web_server.on("/", HandleRoot);                       // This is the Main display page
  web_server.on("/f_temp", GetFridgeData);              // To get fridge temperature and states
  web_server.on("/f_up", FridgeUp);                     // Fridge + 0.1
  web_server.on("/f_down", FridgeDown);                 // Fridge - 0.1
  web_server.on("/f_auto", Fridge_Auto);                // Fridge Auto state
  web_server.on("/f_boost", Fridge_Boost);              // Fridge Boost state
  web_server.on("/f_defrost", Fridge_Defrost);          // Fridge Defrost state
  web_server.on("/f_on", FridgeOn);                     // Switch fridge On
  web_server.on("/f_off", FridgeOff);                   // Switch fridge Off
  web_server.on("/f_slider", FridgeSlider);             // Handle fridge slider move
  web_server.on("/f_settings", handleSettings);         // Settings page
  web_server.on("/f_get_settings", handleGetSettings);  // JS get settings
  web_server.on("/f_set_settings", handleSetSettings);  // JS save settings

  web_server.onNotFound(HandleNotFound);

  if (!wm.autoConnect("Fridgeconfig_AP")) {
    Serial.println("Start AP");
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Fridgecontrol_AP");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);    
  }

  // Start OneWire
  sensors.begin();
  sensors.setWaitForConversion(false);  // Set to asynchronus mode
  sensors.requestTemperatures();        // Initial request right now

  // Start web server
  web_server.begin();
  Serial.println("HTTP Server started");

  // Start OTA
  ArduinoOTA.setHostname("FridgeControl");  // Arduino OTA config and start
  ArduinoOTA.begin(true);                   // With MDNS (open with http://fridgecontrol.local)

  // Read stored data
  preferences.begin("nvs", false);  // Open nonvolatile storage (nvs)
  Language = preferences.getInt("Language", 0);
  FridgeTempLevel = preferences.getFloat("FridgeTempLevel", 6);
  FridgeHyst = preferences.getFloat("FridgeHyst", 2);
  FridgeMaxHigh = preferences.getFloat("FridgeMaxHigh", 10);
  FridgeMaxLow = preferences.getFloat("FridgeMaxLow", 2);
  FridgeAvgError = preferences.getFloat("FridgeAvgError", 0);
  CurrentRPM = preferences.getInt("RPM", 0);
  FridgeBoostTime = preferences.getInt("FridgeBoostTime", 60);
  FridgeBoostTemp = preferences.getFloat("FridgeBoostTemp", 5);
  FridgeBoostHyst = preferences.getFloat("FridgeBoostHyst", 1);
  FridgeBoostRPM = preferences.getInt("FridgeBoostRPM", 3200);
  FridgeMaxRuntime = preferences.getInt("FridgeMaxRuntime", 60);
  FridgeDefrostTime = preferences.getInt("FridgeDefrostTime", 20);
  preferences.end();
}


//****************************************************************************************
// Read DS1820 sensor once per second (non blocking mode)

void GetTemperature() {
  static float FTO = 0;
  static unsigned long LastTime = 0;
  double tmp = 0;

  if (millis() - LastTime > 1000) {
    tmp = sensors.getTempCByIndex(0);
    //Serial.println(tmp);
    if (tmp != -127) {
      if (abs(FTO - tmp) < 1) FridgeTemp = tmp;
      FTO = tmp;
    }
    LastTime = millis();
    sensors.requestTemperatures();  // Send the command to get temperatures
  }
}


//****************************************************************************************
void HandleRoot() {
  if (Language == 0) web_server.send(200, "text/html", FRIDGE_page_en);  //Send web page
  if (Language == 1) web_server.send(200, "text/html", FRIDGE_page_de);  //Send web page
}


//****************************************************************************************
void handleSettings() {
  if (Language == 0) web_server.send(200, "text/html", Settings_page_en);  //Send settings web page
  if (Language == 1) web_server.send(200, "text/html", Settings_page_de);  //Send settings web page
}


//****************************************************************************************
void handleGetSettings() {
  String Text;
  JsonDocument root;

  root["hyst"] = FridgeHyst;
  root["level"] = FridgeTempLevel;
  root["rpm"] = CurrentRPM;
  root["avgerror"] = FridgeAvgError;
  root["maxhigh"] = FridgeMaxHigh;
  root["maxlow"] = FridgeMaxLow;
  root["boosttime"] = FridgeBoostTime;
  root["boosttemp"] = FridgeBoostTemp;
  root["boosthyst"] = FridgeBoostHyst;
  root["boostrpm"] = FridgeBoostRPM;
  root["maxruntime"] = FridgeMaxRuntime;
  root["defrosttime"] = FridgeDefrostTime;
  root["language"] = Language;

  serializeJsonPretty(root, Text);
  web_server.send(200, "text/plain", Text);  //Send values to client ajax request
}


//****************************************************************************************
void handleSetSettings() {
  float tmp;

  if (web_server.args() == 12) {

    tmp = web_server.arg(0).toFloat();
    if (tmp >= 1 && tmp <= 5) FridgeHyst = tmp;

    tmp = web_server.arg(1).toInt();
    if ((tmp == 0) || (tmp >= 2000 && tmp <= 3500)) CurrentRPM = tmp;  // 0 = Relay mode

    tmp = web_server.arg(2).toFloat();
    if (tmp >= 0 && tmp <= 2) FridgeAvgError = tmp;

    tmp = web_server.arg(3).toFloat();
    if (tmp >= 0 && tmp <= 5) FridgeMaxLow = tmp;

    tmp = web_server.arg(4).toFloat();
    if (tmp >= 5 && tmp <= 12) FridgeMaxHigh = tmp;

    tmp = web_server.arg(5).toInt();
    if (tmp >= 10 && tmp <= 120) FridgeBoostTime = tmp;

    tmp = web_server.arg(6).toInt();
    if (tmp >= 2000 && tmp <= 3500) FridgeBoostRPM = tmp;

    tmp = web_server.arg(7).toFloat();
    if (tmp >= 1 && tmp <= 6) FridgeBoostTemp = tmp;

    tmp = web_server.arg(8).toInt();
    if (tmp >= 10 && tmp <= 60) FridgeMaxRuntime = tmp;

    tmp = web_server.arg(9).toInt();
    if (tmp >= 10 && tmp <= 60) FridgeDefrostTime = tmp;

    tmp = web_server.arg(10).toFloat();
    if (tmp >= 0.5 && tmp <= 2) FridgeBoostHyst = tmp;

    tmp = web_server.arg(11).toInt();
    if (tmp >= 0 && tmp <= 1) Language = tmp;

    web_server.send(200, "text/html");
    f_update = true;
  }
}


//****************************************************************************************
// Handle UP button

void FridgeUp() {
  FridgeTempLevel += 0.1;
  if (FridgeTempLevel > 10) FridgeTempLevel = 10;
  f_update = true;
  web_server.send(200);
}


//****************************************************************************************
// Handle Down button

void FridgeDown() {
  FridgeTempLevel -= 0.1;
  if (FridgeTempLevel < 0) FridgeTempLevel = 0;
  f_update = true;
  web_server.send(200);
}


//****************************************************************************************
// Handle Auto button

void Fridge_Auto() {
  FridgeAuto = true;
  FridgeBoost = false;
  FridgeDefrost = false;
  web_server.send(200);
}


//****************************************************************************************
// Handle Boost button

void Fridge_Boost() {

  FridgeBoost = true;
  FridgeAuto = false;
  FridgeDefrost = false;
  FridgeCurrentBoostTime = 0;

  if (FridgeOnOff == On) {
    FridgeSwitch(Off);
    delay(500);
    FridgeSwitch(On);
  }
  web_server.send(200);
}


//****************************************************************************************
// Handle Defrost button

void Fridge_Defrost() {

  FridgeDefrost = true;
  FridgeAuto = false;
  FridgeBoost = false;
  FridgeCurrentDefrostTime = 0;
  FridgeSwitch(Off);
  web_server.send(200);
}


//****************************************************************************************
// Handle On button

void FridgeOn() {
  FridgeSwitch(On);
  FridgeAuto = false;
  FridgeBoost = false;
  FridgeDefrost = false;
  web_server.send(200);
}


//****************************************************************************************
// Handle Off button

void FridgeOff() {
  FridgeSwitch(Off);
  FridgeAuto = false;
  FridgeBoost = false;
  FridgeDefrost = false;
  web_server.send(200);
}


//****************************************************************************************
// Handle slider event

void FridgeSlider() {
  if (web_server.args() > 0) {
    FridgeTempLevel = web_server.arg(0).toFloat();
  }
  f_update = true;
  web_server.send(200);
}


//****************************************************************************************
// Send temperature, level and status as JSON string

void GetFridgeData() {
  String Text, State;
  char buf[30];
  double dutyCycle = 0;
  JsonDocument root;

  if (FridgeDefrost == true) {
    snprintf(buf, sizeof(buf), "Defrost (%d)", FridgeDefrostTime - (FridgeCurrentDefrostTime / 60));
    State = buf;
  } else if (FridgeBoost == true) {
    if (FridgeOnOff == On) {
      snprintf(buf, sizeof(buf), "Boost (%d)", FridgeBoostTime - (FridgeCurrentBoostTime / 60));
      State = buf;
    }
    if (FridgeOnOff == Off) {
      snprintf(buf, sizeof(buf), "Boost (%d)", FridgeBoostTime - (FridgeCurrentBoostTime / 60));
      State = buf;
    }
  } else if (FridgeAuto) {
    if (Language == 0 && FridgeOnOff == On) State = "Auto (On)";
    if (Language == 1 && FridgeOnOff == On) State = "Auto (An)";
    if (Language == 0 && FridgeOnOff == Off) State = "Auto (Off)";
    if (Language == 1 && FridgeOnOff == Off) State = "Auto (Aus)";
  } else {
    if (Language == 0 && FridgeOnOff) State = "On";
    if (Language == 1 && FridgeOnOff) State = "An";
    if (Language == 0 && !FridgeOnOff) State = "Off";
    if (Language == 1 && !FridgeOnOff) State = "Aus";
  }

  root["status"] = State;

  snprintf(buf, sizeof(buf), "%4.1f", FridgeTemp);
  root["temp"] = buf;

  snprintf(buf, sizeof(buf), "%4.1f", FridgeTempLevel);
  root["level"] = buf;

  snprintf(buf, sizeof(buf), "%4.1f", FridgeTempAvg);
  root["avg"] = buf;

  if ((FridgeLastOffTime + FridgeLastOnTime) != 0) {
    dutyCycle = FridgeLastOnTime / (FridgeLastOnTime + FridgeLastOffTime) * 100;
  }

  snprintf(buf, sizeof(buf), "%3.0f %% %1.0f/%1.0f", dutyCycle, FridgeLastOnTime / 60.0, FridgeLastOffTime / 60.0);
  root["dutycycle"] = buf;

  serializeJsonPretty(root, Text);
  web_server.send(200, "text/plain", Text);  //Send sensors values to client ajax request
}


//****************************************************************************************
void HandleNotFound() {  // Unknown request. Send error 404
  web_server.send(404, "text/plain", "File Not Found\n\n");
}


//****************************************************************************************
// Handle Fridge

void HandleFridgeControl() {
  static unsigned long LastTime = 0;

  if (millis() - LastTime > 1000) {

    if ((FridgeOnTime > FridgeMaxRuntime * 60) && FridgeDefrost == false) {  // Compressor runs more than an hour -> Defrost
      FridgeSwitch(Off);
      FridgeDefrost = true;
      FridgeCurrentDefrostTime = 0;
    }

    if (FridgeOnOff == On) {
      FridgeOnTime++;
    } else {
      FridgeOffTime++;
    }

    // Set initial average temperature after start to set temperature level when first temperature measuered
    if (FridgeTempAvg == -100 && FridgeTemp != -100) FridgeTempAvg = FridgeTempLevel;

    // Calculate average temperature with low pass filter (K = 1/3600). Means average over about one hour.
    FridgeTempAvg = FridgeTempAvg + ((FridgeTemp - FridgeTempAvg) / 3600.0);

    if (f_update) {  // Store values if changed
      preferences.begin("nvs", false);
      preferences.putInt("Language", Language);
      preferences.putFloat("FridgeTempLevel", FridgeTempLevel);
      preferences.putFloat("FridgeHyst", FridgeHyst);
      preferences.putFloat("FridgeMaxHigh", FridgeMaxHigh);
      preferences.putFloat("FridgeMaxLow", FridgeMaxLow);
      preferences.putFloat("FridgeAvgError", FridgeAvgError);
      preferences.putInt("RPM", CurrentRPM);
      preferences.putFloat("FridgeBoostTemp", FridgeBoostTemp);
      preferences.putFloat("FridgeBoostHyst", FridgeBoostHyst);
      preferences.putInt("FridgeBoostTime", FridgeBoostTime);
      preferences.putInt("FridgeBoostRPM", FridgeBoostRPM);
      preferences.putInt("FridgeMaxRuntime", FridgeMaxRuntime);
      preferences.putInt("FridgeDefrostTime", FridgeDefrostTime);
      preferences.end();
      f_update = false;
    }

    if (FridgeDefrost == true) {  // Defrost for  FridgeDefrostTime in minutes
      FridgeCurrentDefrostTime++;
      if (FridgeCurrentDefrostTime > FridgeDefrostTime * 60) {
        FridgeDefrost = false;
        FridgeBoost = false;
        FridgeAuto = true;
        FridgeOnTime = 0;
        FridgeOffTime = 0;
      }
    } else if (FridgeBoost == true) {  // Switch On/Off depending on temperature for boost
      FridgeCurrentBoostTime++;

      if (FridgeTemp >= FridgeBoostTemp + FridgeBoostHyst) FridgeSwitch(On);
      if (FridgeTemp <= FridgeBoostTemp - FridgeBoostHyst) FridgeSwitch(Off);

      if (FridgeCurrentBoostTime > FridgeBoostTime * 60) {
        FridgeBoost = false;
        FridgeDefrost = false;
        FridgeAuto = true;
        FridgeOnTime = 0;
        FridgeOffTime = 0;
      }
    } else if (FridgeAuto == true) {  // Switch On/Off depending on temperatures

      if (((FridgeTemp <= FridgeTempLevel - FridgeHyst) && (FridgeTempAvg <= FridgeTempLevel + FridgeAvgError)) || (FridgeTemp <= FridgeMaxLow)) {
        FridgeSwitch(Off);
      }
      if (((FridgeTemp >= FridgeTempLevel + FridgeHyst) && (FridgeTempAvg >= FridgeTempLevel - FridgeAvgError)) || (FridgeTemp >= FridgeMaxHigh)) {
        FridgeSwitch(On);
      }
    }
    LastTime = millis();
  }
}


//****************************************************************************************
// Handle Fridge On/Off and duty cycle calculation

void FridgeSwitch(bool state) {
  unsigned DutyCycle = 0, RPM = 0;
  const unsigned MinDutyCycle = 220;  // 2000 RPM
  const unsigned MaxDutyCycle = 90;   // 3500 RPM

  FridgeOnOff = state;

  RPM = CurrentRPM;
  if (FridgeBoost == true) RPM = FridgeBoostRPM;

  if (RPM >= 2000) {
    DutyCycle = map(RPM, 2000, 3500, MinDutyCycle, MaxDutyCycle);
  } else {
    DutyCycle = 255;  // Relay mode. Duty cycle 255 = permanently On
  }

  if (state == On) {
    analogWrite(FridgePin, DutyCycle);  // On with duty cycle for RPM or permanently for relay
  } else {
    analogWrite(FridgePin, 0);  // Off
  }

  // Measure On/Off times for fridge duty cycle
  if (state == Off && FridgeOnTime != 0) {
    FridgeLastOnTime = FridgeOnTime;
    FridgeOnTime = 0;
  }

  if (state == On && FridgeOffTime != 0) {
    FridgeLastOffTime = FridgeOffTime;
    FridgeOffTime = 0;
  }
}


//*****************************************************************************
void loop() {
  GetTemperature();
  HandleFridgeControl();
  web_server.handleClient();
  ArduinoOTA.handle();
}
