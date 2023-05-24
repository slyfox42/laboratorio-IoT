#include <math.h>

#define FAN_PIN A1
#define LED_PIN A2
#define PIR_PIN A3
#define pinTempSensor A0 
#define minTempL 25
#define maxTempL 35
#define minTempF 25
#define maxTempF 35

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
int pirState = LOW;
int people = 0;
int timeout_pir = 5000;
int detectedTime = -1;


void setup() {

  Serial.begin(9600);

  pinMode(FAN_PIN, OUTPUT);

}

// code for grove temp sensor to acquire and convert temperature to Celsius
int readTemp(int pin) {
  int a = analogRead(pinTempSensor);

  float R = 1023.0/a-1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
  
  Serial.print("temperature = ");
  Serial.println(temperature);

  return temperature;
}

int checkPresence() {
  int val = digitalRead(PIR_PIN); 

  if (people > 0 && pirState == LOW &&
      millis() - detectedTime >= (timeout_pir)) {
        people = 0;
  }

  if (val == HIGH) {	// check if the input is HIGH   
    if (pirState == LOW) {
      Serial.println("Motion detected!");	// print on output change
      pirState = HIGH;
      people += 1;
    }
  } 
  else {
    if (pirState == HIGH) {
      Serial.println("Motion ended!");	// print on output change
      detectedTime = millis();
      pirState = LOW;
    }
  }
}

void loop() {

  float speed = 0;
  float brightness = 255;

  int temperature = readTemp(pinTempSensor);

  delay(100);
  // Set Led Brightness proportional to temperature
  if(temperature > minTempL){
    if(temperature >= maxTempL){
      brightness = 0;
    }
    else{
      brightness = 255 - (maxTempL - temperature)*30;
    }
  }
  analogWrite(LED_PIN, brightness);
  
  // Set Fan Speed proportional to temperature
  if(temperature >= minTempF){
    if(temperature > maxTempF){
      speed = 255;
    }
    else{
      speed = 255 - (maxTempF - temperature)*20;
    }
  }
  // analogWrite(FAN_PIN, speed);
  Serial.print("Speed: ");
  Serial.println(speed);
  Serial.print("Brightness: ");
  Serial.println(brightness);

  checkPresence();
  Serial.print("People: ");
  Serial.println(people);

  delay(1000);

}