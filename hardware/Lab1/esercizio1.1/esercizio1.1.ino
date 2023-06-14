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
void setup() {
  // put your setup code here, to run once:
  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  ITimer1.setInterval(G_HALF_PERIOD * 1000, blinkGreen);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(RLED_PIN, redLedState);
  redLedState = !redLedState;
  delay(R_HALF_PERIOD);
}
