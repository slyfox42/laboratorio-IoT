const int LED_PIN = 2;
const int PIR_PIN = 4;

volatile int tot_count = 0;


void setup() {
  Serial.print(Lab 1.3 Starting);
  pinMode(PIR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN, checkPresence, CHANGE))

}

void loop() {
  // put your main code here, to run repeatedly:

}
