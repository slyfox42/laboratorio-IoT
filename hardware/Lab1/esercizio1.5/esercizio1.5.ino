const int TEMP_PIN = A1;
const int B = 4275;
const long int R0 = 100000;

void setup() {
  Serial.begin(9600);

  while(!Serial);
  Serial.println("Lab 1.5 starting");
}

void loop() {
  int a = analogRead(TEMP_PIN);
  float R = 1023.0/a-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
  
  Serial.print("temperature = ");
  Serial.println(temperature);
  delay(3000);
}
