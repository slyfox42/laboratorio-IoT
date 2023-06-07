#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

#define pinTempSensor A0 
#define ledPin A2

WiFiServer server(80);            //server socket
int status = WL_IDLE_STATUS;      //connection status
const int R0 = 100000;   // needed for temperature conversion
const int B = 4275;            // B value of the thermistor

WiFiClient client = server.available();

// calculate json object ram allocation + declare json response empty object
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 100;
DynamicJsonDocument jsonResponse(capacity);

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  while (!Serial);
  
  enable_WiFi();
  connect_WiFi();

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

// read temperature using the temp sensor
int readTemp(int pin) {
  int a = analogRead(pinTempSensor);

  float R = 1023.0/a-1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
  
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
      // TODO: implement more robust check
    String ledValue = url.substring(5);
    if (ledValue == "0" || ledValue == "1") {
      int intValue = ledValue.toInt();
      digitalWrite(ledPin, intValue); // turn the led on or off

      jsonResponse.clear(); // reset json object
      jsonResponse["bn"] =  "ArduinoGroupX";
      jsonResponse["e"][0]["t"] = int(millis()/1000);
      jsonResponse["e"][0]["n"] = "led"; // selected option
      jsonResponse["e"][0]["v"] = ledValue; // value
      jsonResponse["e"][0]["u"] = NULL; // no unit of measurement here
      serializeJson(jsonResponse, output);
      printResponse(client, 200, output);

    } else {
      printResponse(client, 400, "Invalid led value.");
    }

  } else if (url.startsWith("/temperature")) {
      int temperature = readTemp(pinTempSensor);
      jsonResponse.clear(); // reset json object
      jsonResponse["bn"] =  "ArduinoGroupX";
      jsonResponse["e"][0]["t"] = int(millis()/1000);
      jsonResponse["e"][0]["n"] = "temperature"; // selected option
      jsonResponse["e"][0]["v"] = temperature; // value
      jsonResponse["e"][0]["u"] = "Cel"; // no unit of measurement here
  } else {
    printResponse(client, 404, "Not found.");
  }

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
}
