#include <math.h>

#define FAN_PIN A1
#define pinTempSensor A0 
#define minTemp 25
#define maxTemp 30

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k


void setup() {

  Serial.begin(9600);

  pinMode(FAN_PIN, OUTPUT);

}

void loop() {

  float speed = 0;

  int a = analogRead(pinTempSensor);

  float R = 1023.0/a-1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

  Serial.print("temperature = ");
  Serial.println(temperature);

  delay(100);
  
  if(temperature >= minTemp){
    if(temperature > maxTemp){
      analogWrite(FAN_PIN, 255);
    }
    else{
      speed = 255 - (maxTemp - temperature)*20;
      analogWrite(FAN_PIN, speed);
    }
  }
  Serial.println(speed);

  delay(2000);

}