#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secret.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#define R_at_0 100.0 // resistance at 273.15 K
#define R_for_1 0.385 // resistance raise for each 1K
#define MAX_R 150 // max resistance for termistor - set to show how it works
#define DELAY 1 // in seconds

#define MQTT_SERVER "1.tcp.eu.ngrok.io"
#define MQTT_PORT 21589

float current_temperature;
float set_temperature = 2137.f;
int clima_heating_state = 0;
unsigned long long cpu_prev_time;


WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("", MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.print("connected as ");
      Serial.println(MQTT_USERNAME);
      client.subscribe("esp/out/15");
    } else {
      Serial.print("failed (");
      Serial.print(client.state());
      Serial.println(") trying again in 3 seconds ...");
      delay(3000);
    }
  }
}

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received (");
  Serial.print(topic);
  Serial.print(") ");
  for (int i = 0; i < length; ++i) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // deserialize json
  StaticJsonDocument<244> document;
  deserializeJson(document, payload, length);

  // read values
  unsigned int idx = document["idx"];
  if(idx == 15) {
    set_temperature = document["svalue1"];
    Serial.println(set_temperature);
  }
}

void requestStateChange() {
  // publish state
  StaticJsonDocument<244> doc;
  doc["command"] = "switchlight";
  doc["idx"] = 16;
  doc["switchcmd"] = "Set Level";
  doc["level"] = String(clima_heating_state);
  char buffer[244];
  serializeJson(doc, buffer);
  client.publish("domoticz/in", buffer);
  Serial.print("send: ");
  Serial.println(buffer);
}

void requestTemperatureChange() {
  // publish state
  StaticJsonDocument<244> doc;
  char buffer[244];
  doc["idx"] = 14;
  doc["nvalue"] = 0; 
  doc["svalue"] = String(current_temperature);
  serializeJson(doc, buffer);
  client.publish("domoticz/in", buffer);
  Serial.print("send: ");
  Serial.println(buffer);
}

void setup() {
  Serial.begin(115200);
  delay(256);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(SSID);
  Serial.print(" ..");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to ");
  Serial.print(SSID);
  Serial.print(" as ");
  Serial.println(WiFi.localIP());

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  cpu_prev_time = esp_timer_get_time();
}

void loop() {
  if (cpu_prev_time + 1000000  * DELAY < esp_timer_get_time())
  {
    cpu_prev_time = esp_timer_get_time();
    int analogValue = analogRead(36);
    float resistance = floatMap(analogValue, 0, 4095, 100, MAX_R); // value 100 for debug purposes
    Serial.print("Resistance: ");
    Serial.println(resistance);
    float temperature = (resistance - R_at_0) / R_for_1;
    Serial.print("Temperature: ");
    Serial.println(temperature);
    current_temperature = temperature;
    requestTemperatureChange();
    if (set_temperature < 2137.f){
      if (set_temperature - current_temperature > 2.f){
        clima_heating_state = 10; //grzanie
      }
      else if (set_temperature - current_temperature < -2.f){
        clima_heating_state = 20; //chłodzenie
      }
      else if (clima_heating_state == 10){
        if (set_temperature - current_temperature < -1.f){
          clima_heating_state = 0; //wyłącz
        }
      } 
      else if (clima_heating_state == 20){
        if (set_temperature - current_temperature > 1.f){
          clima_heating_state = 0; //wyłącz
        } 
      }
    }
    requestStateChange();
  }
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
