#include <math.h>
#include <PDM.h>
#include <LiquidCrystal_PCF8574.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <PubSubClient.h>

#include <ArduinoJson.h>
#include <UUID.h>
#include "arduino_secrets.h"
#include "utilities.h"

#define FAN_PIN A1
#define LED_PIN A2
#define PIR_PIN 2
#define pinTempSensor A0

UUID uuid;
WiFiClient wifi;
HttpClient catalogClient = HttpClient(wifi, serverAddress, catalogServerPort);
String broker_address = "test.mosquitto.org";
int broker_port = 1883;
const String base_topic = "/tiot/group8";
String subscriptionAddress;
const int capacity = 256;
DynamicJsonDocument jsonReceived(capacity);
DynamicJsonDocument jsonResponse(capacity);
DynamicJsonDocument subscriptionData(capacity);
DynamicJsonDocument deviceData(capacity);
PubSubClient mqttClient(wifi);

int registrationTime = -1;
const int registerTimeout = 60000;
const int B = 4275;               // B value of the thermistor
const int soundThreshold = 100;   // sound of snapping finger
const int soundInterval = 10000;  // timeout for Microphone
const int nSoundEvents = 10;      // number of events for check a person in soundInterval
const int timeoutPir = 5000;      // timeout for PIR sensor
int motionPeople = 0;             // flag for people detected by PIR sensor
int soundPeople = 0;              // flag for people detected by microphone
volatile int people = 0;          // general flag for people
int nSoundDetected = 0;           // number of sounds detected
long pirDetectedTime = -1;        // time at which the PIR sensor detected movement
long soundDetectedTime = -1;      // time at which the microphone detected movement
short sampleBuffer[256];
volatile int samplesRead;
float speed = 0;
float brightness = 0;
String lcdMessage;

LiquidCrystal_PCF8574 lcd(0x20);

/*
Abbiamo deciso di portare nel cloud
changeThreshold: in questo modo la modifica delle soglie può essere più agevole dal lato utente,
soprattutto nel caso si volessero modificare determinati parametri o si volessero applicare
le modifiche a più dispositivi contemporaneamente

Abbiamo deciso di mantenere nell'Arduino
Sensing
Calcolo della temperatura: essendo l'esito del calcolo potenzialmente utile anche per altre funzioni,
abbiamo ritenuto conveniente mantenerlo nello sketch
Conteggio delle persone tramite movimento/suono: per limitare il numero di dati che vengono inviati al cloud.
Parte del display su LCD: per avere un caso di default
*/

void callback(char* topic, byte* payload, unsigned int length) {
  if (topic == (base_topic + "/led")) {
    DeserializationError err = deserializeJson(jsonReceived, (char*) payload);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }
    if (jsonReceived["e"][0]["n"] == "brightness") {
      int value = jsonReceived["e"][0]["v"];
      if (value >= 0 || value <= 255) {
        analogWrite(LED_PIN, value);
        Serial.print("Led value set to");
        Serial.println(value);
        return;
      } else {
        Serial.println("Invalid LED value received.");
        return;
      }
    }
  }
  if (topic == (base_topic + "/fan")) {
    DeserializationError err = deserializeJson(jsonReceived, (char*) payload);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }
    if (jsonReceived["e"][0]["n"] == "speed") {
      int value = jsonReceived["e"][0]["v"];
      if (value >= 0 || value <= 255) {
        analogWrite(FAN_PIN, value);
        Serial.print("Fan value set to");
        Serial.println(value);
        return;
      } else {
        Serial.println("Invalid Fan value received.");
        return;
      }
    }
  }
  if (topic == (base_topic + "/lcd")) {
    DeserializationError err = deserializeJson(jsonReceived, (char*) payload);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }
    if (jsonReceived["e"][0]["n"] == "message") {
      lcdMessage = jsonReceived["e"][0]["v"].as<String>();
      return;
    }
  }
}

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

  return subscriptionData["subscriptions"]["REST"]["device"].as<String>();
}

String senMlEncodeTemp(float temperature) {
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

String senMlEncodeSound(int soundPeople) {
  String body;
  jsonResponse.clear();
  jsonResponse["bn"] =  "ArduinoGroupX";
  jsonResponse["e"][0]["t"] = int(millis()/1000);
  jsonResponse["e"][0]["n"] = "Sound"; 
  jsonResponse["e"][0]["v"] = soundPeople; 
  jsonResponse["e"][0]["u"] = (char*)NULL;
  serializeJson(jsonResponse, body);

  return body;
}

String senMlEncodeMotion(int motionPeople) {
  String body;
  jsonResponse.clear();
  jsonResponse["bn"] =  "ArduinoGroupX";
  jsonResponse["e"][0]["t"] = int(millis()/1000);
  jsonResponse["e"][0]["n"] = "Motion"; 
  jsonResponse["e"][0]["v"] = motionPeople; 
  jsonResponse["e"][0]["u"] = (char*)NULL;
  serializeJson(jsonResponse, body);

  return body;
}

void reconnect() {
  while (mqttClient.state() != MQTT_CONNECTED) {
    if (mqttClient.connect("TiotGroup8")) {
      mqttClient.subscribe((base_topic + String("/led")).c_str());
      mqttClient.subscribe((base_topic + String("/fan")).c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds...");
      delay(5000);
    }
  }
}

void registerDevice() {
  int timeNow = millis();
  String body;
  if (registrationTime != -1) {
    if ((timeNow - registrationTime) < registerTimeout) {
      return;
    }
    Serial.println("Refreshing device registration...");
    serializeJson(deviceData, body);

    catalogClient.beginRequest();
    catalogClient.put(subscriptionAddress);
    catalogClient.sendHeader("Content-Type", "application/json");
    catalogClient.sendHeader("Content-Length", body.length());
    catalogClient.beginBody();
    catalogClient.print(body);
    catalogClient.endRequest();
  } else {
    Serial.println("Registering device...");
    subscriptionAddress = getSubscription();
    jsonResponse.clear();
    deviceData["deviceID"] =  uuid;
    deviceData["endPoints"]["MQTT"]["Led"] = "resources/led";
    deviceData["endPoints"]["MQTT"]["Fan"] = "resources/fan";
    deviceData["endPoints"]["MQTT"]["Temperature"] = "resources/temperature";
    deviceData["availableResources"][0] = "Motion Sensor"; 
    deviceData["availableResources"][1] = "Temperature";
    deviceData["availableResources"][2] = "Microphone";
    deviceData["availableResources"][3] = "Fan";
    deviceData["availableResources"][4] = "Led";
    deviceData["availableResources"][5] = "LCD";
    serializeJson(deviceData, body);
    postData(catalogClient, "/device", body);
  }
  int responseCode = catalogClient.responseStatusCode();
  String responseBody = catalogClient.responseBody();
  Serial.print("Response code: ");
  Serial.println(responseCode);
  Serial.print("Response body: ");
  Serial.println(responseBody);
}

void postData(HttpClient catalogClient, String path, String body) {
  catalogClient.beginRequest();
  catalogClient.post(path);
  catalogClient.sendHeader("Content-Type", "application/json");
  catalogClient.sendHeader("Content-Length", body.length());
  catalogClient.beginBody();
  catalogClient.print(body);
  catalogClient.endRequest();
}

float readTemp(int pin) {
  int a = analogRead(pinTempSensor);
  float R = 1023.0/a-1.0;
  float temperature = 1.0/(log(R)/B+1/298.15)-273.15;
  return temperature;
}

// callback function for microphone 
void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2;
}

void checkSound() {
  if (samplesRead) {
    for (int i = 0; i < samplesRead; i++) {
      if (sampleBuffer[i]>=soundThreshold) {
        Serial.println("Sound detected");
        nSoundDetected += 1;
        soundDetectedTime = millis();    
      }
    samplesRead = 0;
    }
  }
  // no people yet detected from mic, but min number of event detected
  if (soundPeople == 0 && nSoundDetected >= nSoundEvents) {
    soundPeople += 1;
    String body = senMlEncodeSound(soundPeople);
  
    Serial.println("Publishing data to broker...");
    mqttClient.publish((base_topic + String("/sound")).c_str(), body.c_str());
  }
  // no sounds detected for soundInterval
  if (nSoundDetected > 0 && ((millis() - soundDetectedTime) >= (soundInterval))) {
    soundPeople = 0;
    nSoundDetected = 0;
    String soundBody = senMlEncodeSound(soundPeople);
  
    Serial.println("Publishing data to broker...");
    mqttClient.publish((base_topic + String("/sound")).c_str(), soundBody.c_str());
  }
}

void displayOnLCD(float temperature) {
  lcd.home();
  lcd.clear();
  lcd.print(lcdMessage);
}

// sets motionPeople to 1 in case the PIR sensor detects movement
void setPresent() {
  pirDetectedTime = millis();
  if(motionPeople == 0) {
    motionPeople = 1;
    String motionBody = senMlEncodeMotion(motionPeople);
    Serial.println("Publishing data to broker...");
    mqttClient.publish((base_topic + String("/motion")).c_str(), motionBody.c_str());
  }
}

// checks the value of motionPeople and resets if timeout has been reached
void checkPresent() {
  if (motionPeople == 1 &&
      millis() - pirDetectedTime >= (timeoutPir)) {
        motionPeople = 0;
        String motionBody = senMlEncodeMotion(motionPeople);
        Serial.println("Publishing data to broker...");
        mqttClient.publish((base_topic + String("/motion")).c_str(), motionBody.c_str());
  }
}

void setup() {
  Serial.begin(9600);

  while (!Serial);
  
  Serial.println("Lab 2 Starting");
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), setPresent, CHANGE);
  PDM.onReceive(onPDMdata);

  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM");
    while(1);
  }

  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();

  enable_WiFi();
  connect_WiFi(wifiSsid, wifiPass);

  printWifiStatus();
}

void loop() {
  if (mqttClient.state() != MQTT_CONNECTED) {
    reconnect();
  }

  registerDevice();
  
  float temperature = readTemp(pinTempSensor);

  checkPresent();
  checkSound();

  String body = senMlEncodeTemp(temperature);
  
  Serial.println("Publishing data to broker...");
  mqttClient.publish((base_topic + String("/temperature")).c_str(), body.c_str());
  
  Serial.println("Checking for new messages on subscribed topics...");
  mqttClient.loop();

  //Arduino only sends temperature via MQTT, and sets speed and brightness based on JSON received from cloud.

  //Arduino is not changing its thresholds anymore.
  displayOnLCD(temperature);
  delay(3000);

}