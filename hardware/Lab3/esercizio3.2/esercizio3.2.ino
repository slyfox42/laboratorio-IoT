#define pinTempSensor A0

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k

void setup() {

  Serial.begin(9600);

}

float readTemp(int pin) {
  int a = analogRead(pinTempSensor);

  float R = 1023.0/a-1.0;
  R = R0*R;
  // TODO: check what is R0 for because it looks completely useless
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
  
  Serial.print("temperature = ");
  Serial.println(temperature);

  return temperature;
}

void loop() {
  
}