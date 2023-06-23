#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include "utilities.h"

#define pinTempSensor A0
#define ledPin A2

WiFiServer server(80); // server socket
const int B = 4275;    // B value of the thermistor

WiFiClient client = server.available();

// calculate json object ram allocation + declare json response empty object
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument jsonResponse(capacity);

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  while (!Serial);

  enable_WiFi();
  connect_WiFi(wifiSsid, wifiPass);

  server.begin();
  printWifiStatus();
}

void loop() {
  client = server.available();

  delay(1000);
  if (client) {
    process(client);
  }
}

// read temperature using the temp sensor
float readTemp(int pin) {
  int a = analogRead(pinTempSensor);
  float R = 1023.0 / a - 1.0;
  float temperature = 1.0 / (log(R) / B + 1 / 298.15) - 273.15; // convert to temperature via datasheet

  return temperature;
}

// process incoming requests to the arduino
void process(WiFiClient client) {
  String output;
  String reqType = client.readStringUntil(' ');
  reqType.trim();
  String url = client.readStringUntil(' ');
  url.trim();
  if (url.startsWith("/led/")) {
    String ledValue = url.substring(5, 6);
    if (ledValue == "0" || ledValue == "1") {
      int intValue = ledValue.toInt();
      digitalWrite(ledPin, intValue); // turn the led on or off

      String body = senMlEncode("led", intValue, "");

      printResponse(client, 200, body);
    } else {
      printResponse(client, 400, "Invalid led value.");
    }
  } else if (url.startsWith("/temperature")) {
    int temperature = readTemp(pinTempSensor);
    String body = senMlEncode("temperature", temperature, "Cel");
    printResponse(client, 200, body);
  } else {
    printResponse(client, 404, "Not found.");
  }
}

String senMlEncode(String option, float value, String unit) {
  if (option == "led") {
    value = int(value);
  }
  String body;
  jsonResponse.clear();
  jsonResponse["bn"] = "ArduinoGroup8";
  jsonResponse["e"][0]["t"] = int(millis() / 1000);
  jsonResponse["e"][0]["n"] = option;
  jsonResponse["e"][0]["v"] = value;
  jsonResponse["e"][0]["u"] = unit;
  serializeJson(jsonResponse, body);

  return body;
}

// print message response with code and body
void printResponse(WiFiClient client, int code, String body) {
  client.println("HTTP/1.1 " + String(code));
  if (code == 200) {
    client.println("Content-type: application/json; charset=utf-8");
    client.println();
    client.println(body);
  } else {
    client.println();
  }
  client.stop();
}
