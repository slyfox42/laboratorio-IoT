const int FAN_PIN = 1;
const int STEP = 10;

float current_speed = 0;

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, (int) current_speed);

  Serial.begin(9600);
  while (!Serial);
  Serial.println("Lab 1.4 Starting");
}

void loop() {
  if(Serial.available() > 0) {
    int inByte = Serial.read();

    if(inByte == '+') {
      if(current_speed == 255){
        Serial.println("Already at max speed");
      }
      else{
        current_speed += STEP;
        if(current_speed > 255) current_speed = 255;
        analogWrite(FAN_PIN, (int) current_speed);
        Serial.print("Increasing speed: ");
        Serial.println(current_speed);
      }
    } else if(inByte == '-') {
      if(current_speed == 0){
        Serial.println("Already at min speed");
      }
      else{
        current_speed -= STEP;
        if(current_speed < 0) current_speed = 0;
        analogWrite(FAN_PIN, (int) current_speed);
        Serial.print("Decreasing speed: ");
        Serial.println(current_speed);
      }
    } else Serial.println("Invalid command");   //set No Line Ending to avoid errors
  }
  delay(500);
}
