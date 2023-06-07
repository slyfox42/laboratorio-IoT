#include <LiquidCrystal_PCF8574.h>

LiquidCrystal_PCF8574 lcd(0x20);

const int TEMP_PIN = A1;
const int B = 4275;
const long int R0 = 100000;

void setup() {
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("Temperature:");
}

void loop() {
  int a = analogRead(TEMP_PIN);
  float R = 1023.0/a-1.0;
  R = R0*R;
  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
  
  lcd.home();
  lcd.clear();
  lcd.print("Temperature:");
  lcd.print(temperature);
  delay(3000);
}
