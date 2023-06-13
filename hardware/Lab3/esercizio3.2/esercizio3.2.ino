#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "utilities.h"
#include "arduino_secrets.h"
#define pinTempSensor A0

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, serverPort);

const int B = 4275;               // B value of the thermistor
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument jsonResponse(capacity);


void setup() {

  Serial.begin(9600);
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

void postTemperature(String body) {
  client.beginRequest();
  client.post("/log");
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", body.length());
  client.beginBody();
  client.print(body);
  client.endRequest();
}

void loop() {
  float temperature = readTemp(pinTempSensor);
  String body = senMlEncode(temperature);
  postTemperature(body);
  int responseCode = client.responseStatusCode();
  String responseBody = client.responseBody();
  Serial.print("Response code: ");
  Serial.println(responseCode);
  Serial.print("Response body: ");
  Serial.println(responseBody);
  delay(5000);
}