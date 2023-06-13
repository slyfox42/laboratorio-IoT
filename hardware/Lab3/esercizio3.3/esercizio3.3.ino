#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include "utilities.h"

#define pinTempSensor A0
#define ledPin A2

WiFiClient wifi;
const int B = 4275;
String broker_address = "test.mosquitto.org";
int broker_port = 1883;
const String base_topic = "/tiot/group8";
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument jsonReceived(capacity);
DynamicJsonDocument jsonResponse(capacity);



void callback(char* topic, byte* payload, unsigned int length) {
  if (topic == (base_topic + "/led")) {
    DeserializationError err = deserializeJson(jsonReceived, (char*) payload);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }
    if (jsonReceived["e"][0]["n"] == "led") {
      int value = jsonReceived["e"][0]["v"];
      if (value == 1 || value == 0) {
        digitalWrite(ledPin, value);
        Serial.println("Led value set.");
        return;
      } else {
        Serial.println("Invalid LED value received.");
        return;
      }
    }
  }
  
}

PubSubClient client(broker_address.c_str(), broker_port, callback, wifi);

String senMlEncode(float temperature) {
  String body;
  jsonResponse.clear();
  jsonResponse["bn"] =  "ArduinoGroupX";
  jsonResponse["e"][0]["t"] = int(millis()/1000);
  jsonResponse["e"][0]["n"] = "temperature"; 
  jsonResponse["e"][0]["v"] = temperature; 
  jsonResponse["e"][0]["u"] = "Cel"; 
  serializeJson(jsonResponse, body);

  return body;
}

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  while (!Serial);

  enable_WiFi();
  connect_WiFi(wifiSsid, wifiPass);

  printWifiStatus();
}


float readTemp(int pin) {
  int a = analogRead(pinTempSensor);

  float R = 1023.0/a-1.0;
  float temperature = 1.0/(log(R)/B+1/298.15) - 273.15; // convert to temperature via datasheet
  
  Serial.print("temperature = ");
  Serial.println(temperature);

  return temperature;
}

void reconnect() {
  while (client.state() != MQTT_CONNECTED) {
    if (client.connect("TiotGroup8")) {
      client.subscribe((base_topic + String("/led")).c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  if (client.state() != MQTT_CONNECTED) {
    reconnect();
  }
  Serial.println("Reading temperature...");
  float temperature = readTemp(pinTempSensor);
  String body = senMlEncode(temperature);
  
  Serial.println("Publishing data to broker...");
  client.publish((base_topic + String("/temperature")).c_str(), body.c_str());
  
  Serial.println("Checking for new messages on subscribed topics...");
  client.loop();

  delay(10000);
}

