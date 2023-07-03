# Smarthome - Thermostat module
ESP32 microcontroller code for Domoticz control of hysteresis thermostat.

This is part of UMPAnUMiW (SUT) project.

## Hardware 
* ESP32 (ESP8266 can also be used, tho the part of code responsible for stepper steps has to be changed to pulsating as ESP8266 crashes if core is busy for too long)
* Any linear thermistor or temperature sensitive resistor
* 2.2V - 3.6V power supply for ESP32 or 5V via USB port
* Stable wi-fi connection

## Setup and Usage
### Software
Create a headers/secret.h file with this structure:
```cpp
#define SSID "<<Wifi network SSID>>";
#define PASSWORD "<<Wifi network password>>";
#define MQTT_USERNAME "<<'umpanumiw' in our case or any other for your own purpose>>";
#define MQTT_PASSWORD "<<'umpanumiw' in our case or any other for your own purpose>>";
```
Modify following values in src/main.cpp according to your thermistor/resistor specification and mqtt server parameters:
```cpp
#define R_at_0 100.0 // resistance at 273.15 K
#define R_for_1 0.385 // resistance raise for each 1K
#define MAX_R 150 // max resistance for termistor
#define DELAY 1 // in seconds - suggested: 1 - debugging, >=60 - normal work

#define MQTT_SERVER "1.tcp.eu.ngrok.io" // mqtt server address
#define MQTT_PORT 21589 // mqtt server port
```

### Hardware
Connect your thermistor to ESP32 in the following way:
* 1 pin to 3V3 or 5V pin (depending on used voltage)
* second pin to GPIO36

### Debugging
Measured, sent and received data can be seen at runtime using a serial motnitor. Logs are sent using 115200 boundrate.
