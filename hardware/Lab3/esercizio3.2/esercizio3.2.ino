#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "utilities.h"
#include "arduino_secrets.h"

#define pinTempSensor A0

WiFiClient wifi;
HttpClient catalogClient = HttpClient(wifi, serverAddress, catalogServerPort);
HttpClient temperatureClient = HttpClient(wifi, serverAddress, temperatureServerPort);

int registrationTime = -1;
const int registerTimeout = 60000;
const int B = 4275; // B value of the thermistor
const int capacity = 256;
String subscriptionAddress;
DynamicJsonDocument jsonResponse(capacity);
DynamicJsonDocument deviceData(capacity);
DynamicJsonDocument updateData(capacity);
DynamicJsonDocument subscriptionData(capacity);

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

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

String getSubscription() {
  catalogClient.beginRequest();
  catalogClient.get("/");
  int statusCode = catalogClient.responseStatusCode();
  String response = catalogClient.responseBody();
  subscriptionData.clear();
  deserializeJson(subscriptionData, response);

  return subscriptionData["subscriptions"]["REST"]["device"];
}

void registerDevice() {
  int timeNow = millis();
  String body;
  String deviceId = "ArduinoGroup8";
  if (registrationTime != -1) {
    if ((timeNow - registrationTime) < registerTimeout) {
      return;
    }
    Serial.println("Refreshing device registration...");
    serializeJson(deviceData, body);

    catalogClient.beginRequest();
    catalogClient.put("/device/" + deviceId);
    catalogClient.sendHeader("Content-Type", "application/json");
    catalogClient.sendHeader("Content-Length", body.length());
    catalogClient.beginBody();
    catalogClient.print(body);
    catalogClient.endRequest();
  } else {
    Serial.println("Registering device...");
    deviceData.clear();
    deviceData["deviceID"] = "ArduinoGroup8";
    deviceData["endPoints"]["MQTT"]["Led"] = "resources/led";
    deviceData["endPoints"]["MQTT"]["Temperature"] = "resources/temperature";
    deviceData["availableResources"][0] = "Motion Sensor";
    deviceData["availableResources"][1] = "Temperature";
    deviceData["timestamp"] = timeNow;
    serializeJson(deviceData, body);
    postData(catalogClient, subscriptionAddress, body);
  }
  registrationTime = timeNow;
  int responseCode = catalogClient.responseStatusCode();
  String responseBody = catalogClient.responseBody();
  Serial.print("Response code: ");
  Serial.println(responseCode);
  Serial.print("Response body: ");
  Serial.println(responseBody);
}

void postData(HttpClient client, String path, String body) {
  client.beginRequest();
  client.post(path);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", body.length());
  client.beginBody();
  client.print(body);
  client.endRequest();
}

void loop() {
  registerDevice();
  float temperature = readTemp(pinTempSensor);
  String temperatureData = senMlEncode(temperature);
  postData(temperatureClient, "/log", temperatureData);
  int responseCode = temperatureClient.responseStatusCode();
  String responseBody = temperatureClient.responseBody();
  Serial.print("Response code: ");
  Serial.println(responseCode);
  Serial.print("Response body: ");
  Serial.println(responseBody);
  delay(5000);
}