#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include "utilities.h"

#define pinTempSensor A0

WiFiClient wifi;
const int B = 4275;
String broker_address = "test.mosquitto.org";
int broker_port = 1883;
int status = WL_IDLE_STATUS;
const String base_topic = "/tiot/8";
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec(capacity);



void callback(char* topic, byte* payload, unsigned int length) {
  DeserializationError err = deserializeJson(doc_rec, (char*) payload);
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }

}

PubSubClient client(broker_address.c_str(), broker_port, callback, wifi);


// String senMlEncode(float temperature) {
//   String body;
//   jsonResponse.clear();
//   jsonResponse["bn"] =  "ArduinoGroupX";
//   jsonResponse["e"][0]["t"] = int(millis()/1000);
//   jsonResponse["e"][0]["n"] = "temperature"; 
//   jsonResponse["e"][0]["v"] = temperature; 
//   jsonResponse["e"][0]["u"] = "Cel"; 
//   serializeJson(jsonResponse, body);

//   return body;
// }

void setup() {

  Serial.begin(9600);
  while (!Serial);

  enable_WiFi();
  connect_WiFi();

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


void loop() {
  float temperature = readTemp(pinTempSensor);

}