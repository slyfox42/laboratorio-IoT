#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"
#include "utilities.h"
#include "UUID.h"

#define pinTempSensor A0
#define ledPin A2

UUID uuid;
WiFiClient wifi;
HttpClient catalogClient = HttpClient(wifi, serverAddress, serverPort);
PubSubClient mqttClient(wifi);

String subscriptionAddress;
String mqtt_base_topic: "tiot/group8"
const int B = 4275;
int registrationTime = -1;
const int registerTimeout = 60000;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
const int subscriptionCapacity = 256;

DynamicJsonDocument jsonReceived(capacity);
DynamicJsonDocument jsonResponse(capacity);
DynamicJsonDocument deviceData(capacity);
DynamicJsonDocument subscriptionData(subscriptionCapacity);

void callback(char *topic, byte *payload, unsigned int length) {
  if (topic == (mqtt_base_topic + "/led")) {
    DeserializationError err = deserializeJson(jsonReceived, (char *)payload);
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


String senMlEncode(float temperature) {
  String body;
  jsonResponse.clear();
  jsonResponse["bn"] = "ArduinoGroup8";
  jsonResponse["e"][0]["t"] = int(millis() / 1000);
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

  float R = 1023.0 / a - 1.0;
  float temperature = 1.0 / (log(R) / B + 1 / 298.15) - 273.15; // convert to temperature via datasheet

  Serial.print("temperature = ");
  Serial.println(temperature);

  return temperature;
}

// get available subscriptions from Catalog
// "subscription": {
//   "MQTT": {
//     "device": {
//       "hostname": "test.mosquitto.org",
//       "port": "1883",
//       "topic": "tiot/group8/catalog/devices/subscription"
//     }
//   }
// }
String getSubscription() {
  catalogClient.beginRequest();
  catalogClient.get("/");
  int statusCode = catalogClient.responseStatusCode();
  String response = catalogClient.responseBody();
  subscriptionData.clear();
  deserializeJson(subscriptionData, response);

  subscriptionAddress = subscriptionData["subscriptions"]["MQTT"]["device"]["topic"].as<String>(); // needed to remove ambiguity when extracting data from json
  String broker_address = subscriptionData["subscriptions"]["MQTT"]["device"]["hostname"];
  int broker_port = subscriptionData["subscriptions"]["MQTT"]["device"]["port"];

  mqttClient.setServer(broker_address.c_str(), broker_port);
  mqttClient.setCallback(callback);
}

// register the device using the subscription endpoints obtained from catalog. Otherwise, refresh the subscription every minute
void registerDevice() {
  int timeNow = millis();
  String body;
  String deviceId = uuid.toCharArray();
  if (registrationTime != -1) {
    if ((timeNow - registrationTime) < registerTimeout) {
      return;
    }
    Serial.println("Refreshing device registration...");
    serializeJson(deviceData, body);

    mqttClient.publish(subscriptionAddress.c_str(), body.c_str());
  } else {
    Serial.println("Registering device...");
    getSubscription();
    deviceData.clear();
    deviceData["deviceID"] = deviceId;
    deviceData["endPoints"][0] = "temperature";
    deviceData["endPoints"][1] = "led";
    deviceData["sensors"][0] = "Motion Sensor";
    deviceData["sensors"][1] = "Temperature";
    serializeJson(deviceData, body);
    mqttClient.publish(subscriptionAddress.c_str(), body.c_str());
  }
  registrationTime = timeNow;
  int responseCode = catalogClient.responseStatusCode();
  String responseBody = catalogClient.responseBody();
  Serial.print("Response code: ");
  Serial.println(responseCode);
  Serial.print("Response body: ");
  Serial.println(responseBody);
}

void postData(HttpClient mqttClient, String path, String body) {
  mqttClient.beginRequest();
  mqttClient.post(path);
  mqttClient.sendHeader("Content-Type", "application/json");
  mqttClient.sendHeader("Content-Length", body.length());
  mqttClient.beginBody();
  mqttClient.print(body);
  mqttClient.endRequest();
}

void reconnect() {
  while (mqttClient.state() != MQTT_CONNECTED) {
    if (mqttClient.connect("TiotGroup8")) {
      mqttClient.subscribe((mqtt_base_topic + String("/led")).c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  registerDevice();
  if (mqttClient.state() != MQTT_CONNECTED) {
    reconnect();
  }
  Serial.println("Reading temperature...");
  float temperature = readTemp(pinTempSensor);
  String body = senMlEncode(temperature);

  Serial.println("Publishing data to broker...");
  mqttClient.publish((mqtt_base_topic + String("/temperature")).c_str(), body.c_str());

  Serial.println("Checking for new messages on subscribed topics...");
  mqttClient.loop();

  delay(10000);
}
