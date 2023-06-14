#include <MBED_RPi_Pico_TimerInterrupt.h>

const int RLED_PIN = 2;
const int GLED_PIN = 3;

const long R_HALF_PERIOD = 1500L;
const long G_HALF_PERIOD = 3500L;

int redLedState = LOW;
int greenLedState = LOW;

MBED_RPI_PICO_Timer ITimer1(1);

void blinkGreen(uint alarm_num) {
  TIMER_ISR_START(alarm_num);
  digitalWrite(GLED_PIN, greenLedState);
  greenLedState = !greenLedState;
  TIMER_ISR_END(alarm_num);
}

void serialPrintStatus() {
  if(Serial.available() > 0) {
    int inByte = Serial.read();

    if(inByte == 'R') {
      Serial.print("LED 2 Status:");
      Serial.println((int)redLedState);
    } else if(inByte == 'L') {
      Serial.print("LED 3 Status:");
      Serial.println((int)redLedState);
    } else Serial.println("Invalid command");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Lab 1.2 Starting");

  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  ITimer1.setInterval(G_HALF_PERIOD * 1000, blinkGreen);
}

void loop() {
  serialPrintStatus();
  digitalWrite(RLED_PIN, redLedState);
  redLedState = !redLedState;
  delay(R_HALF_PERIOD);
}
