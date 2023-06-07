const int LED_PIN = 2;
const int PIR_PIN = 4;

volatile int tot_count = 0;

void checkPresence() {
  tot_count += digitalRead(PIR_PIN);
  digitalWrite(LED_PIN, digitalRead(PIR_PIN));
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Lab 1.3 Starting");
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), checkPresence, CHANGE);
}

void loop() {
  Serial.print("Total people count: ");
  Serial.println(tot_count);
  delay(30000);
}
