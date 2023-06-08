#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#define pinTempSensor A0

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;    
int status = WL_IDLE_STATUS;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument jsonResponse(capacity);

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server_address, server_port);

void setup() {

  Serial.begin(9600);
  while (!Serial);
  
  enable_WiFi();
  connect_WiFi();

  printWifiStatus();

}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

}

void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass); // connect to wifi using name and pass

    // wait 10 seconds for connection:
    delay(10000);
  }
}

float readTemp(int pin) {
  int a = analogRead(pinTempSensor);

  float R = 1023.0/a-1.0;
  R = R0*R;
  // TODO: check what is R0 for because it looks completely useless
  float temperature = 1.0/(log(R/R0)/B+1/298.15) - 273.15; // convert to temperature via datasheet
  
  Serial.print("temperature = ");
  Serial.println(temperature);

  return temperature;
}

void loop() {
  float temperature = readTemp(pinTempSensor);
  String body;
  jsonResponse.clear();
  jsonResponse["bn"] =  "ArduinoGroupX";
  jsonResponse["e"][0]["t"] = int(millis()/1000);
  jsonResponse["e"][0]["n"] = "temperature"; 
  jsonResponse["e"][0]["v"] = temperature; 
  jsonResponse["e"][0]["u"] = "Cel"; 
  serializeJson(jsonResponse, body);

  client.beginRequest();
  client.post("/log");
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", body.length());
  client.beginBody();
  client.print(body);
  client.endRequest();
  int ret = client.responseStatusCode();

  delay(3000);
}