#include <math.h>
#include <PDM.h>
#include <LiquidCrystal_PCF8574.h>

#define FAN_PIN A1
#define LED_PIN A2
#define PIR_PIN 2
#define pinTempSensor A0

#define minTempL 25
#define maxTempL 35
#define minTempF 25
#define maxTempF 35

#define minTempLP 22
#define maxTempLP 32
#define minTempFP 22
#define maxTempFP 32

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int soundThreshold = 200; // sound of snapping finger
const int soundInterval = 10000; //timeout for Microphone
const int nSoundEvents = 10;    //number of events for check a person in soundInterval
const int timeoutPir = 5000;          //timeout for PIR sensor
int pirState = LOW;
int motionPeople = 0;       //flag for people using PIR sensor
int soundPeople = 0;  //flag for people using microphone
volatile int people = 0;       //general flag for people
int nSoundDetected = 0;
int detectedTime = -1;
int soundDetectedTime = -1;
short sampleBuffer[256];
volatile int samplesRead;
int minTempLED = minTempL;
int maxTempLED = maxTempL;
int minTempFan = minTempF;
int maxTempFan = maxTempF;
int isFirstDisplay = 1;

LiquidCrystal_PCF8574 lcd(0x20);

// code for grove temp sensor to acquire and convert temperature to Celsius
float readTemp(int pin) {
  int a = analogRead(pinTempSensor);

  float R = 1023.0/a-1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
  
  Serial.print("temperature = ");
  Serial.println(temperature);

  return temperature;
}

// callback function for microphone 
void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();

  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}

void checkSound() {
  if (samplesRead) {
    for (int i = 0; i < samplesRead; i++) {
      if (sampleBuffer[i]>=soundThreshold){
        Serial.println("Sound detected");     //debug for detected sound
        //Serial.println(sampleBuffer[i]);    //volume of sound detected
        nSoundDetected += 1;
      }
    // clear the read count
    samplesRead = 0;
    }
  }
  if (soundPeople == 0 && nSoundDetected >= nSoundEvents){     //no people yet detected from mic, but min number of event detected
      //Serial.println("People detected by microphone");
      soundPeople += 1;
      soundDetectedTime = millis();                            //timestamp
    }
  else if (soundPeople == 1 && nSoundDetected > nSoundEvents){  //people detected and another sound detected
    //Serial.println("People made another sound")
    soundDetectedTime = millis();                               //timestamp
    nSoundDetected = nSoundEvents;                              //counter back to nSoundEvents
  }
  if (nSoundDetected > 0 && millis() - detectedTime >= (soundInterval)) {   //no sounds detected for soundInterval
        //Serial.println("Sounds not detected for a long time");
        soundPeople = 0;
        nSoundDetected = 0;
  }
}

void changeThreshold() {
  if(motionPeople > 0 || soundPeople > 0){  //people detected from sensors
    people = 1;
    minTempLED = minTempLP;
    maxTempLED = maxTempLP;
    minTempFan = minTempFP;
    maxTempFan = maxTempFP;
  }
  else {                                    //people not detected from sensors
    people = 0;
    minTempLED = minTempL;
    maxTempLED = maxTempL;
    minTempFan = minTempF;
    maxTempFan = maxTempF;
  }
}

void displayOnLCD(float temperature, float brightness, float speed) {
  lcd.home();
  lcd.clear();
  if(isFirstDisplay){
    lcd.print("T: ");
    lcd.print(temperature);
    lcd.print(" Pres: ");
    lcd.print(people);
    lcd.setCursor(0, 1);
    lcd.print("AC: ");
    float ACpercentage = (speed / 255) * 100;
    lcd.print(ACpercentage, 0);
    lcd.print("% ");
    lcd.print("HT: ");
    float HTpercentage = (brightness / 255) * 100;
    lcd.print(HTpercentage, 0);
    lcd.print("%");
  }
  else{
    lcd.print("AC m: ");
    lcd.print(minTempFan);
    lcd.print(" M: ");
    lcd.print(maxTempFan);
    lcd.setCursor(0, 1);
    lcd.print("HT m: ");
    lcd.print(minTempLED);
    lcd.print(" M: ");
    lcd.print(maxTempLED);
  }
  isFirstDisplay = !isFirstDisplay;
}
// sets motionPeople to 1 in case the PIR sensor detects movement
void setPresent() {
  motionPeople = 1;
  detectedTime = millis();
}

// checks the value of motionPeople and resets if timeout has been reached
void checkPresent() {
  if (motionPeople == 1 &&
      millis() - detectedTime >= (timeoutPir)) {
        motionPeople = 0;
  }
}

void setup() {

  Serial.begin(9600);

  pinMode(FAN_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), setPresent, CHANGE);
  PDM.onReceive(onPDMdata);

  if (!PDM.begin(1, 16000)) {
    Serial.println("Failed to start PDM");
    while(1);
  }

  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
}

void loop() {


  float speed = 0;
  float brightness = 0;

  checkPresent();

  float temperature = readTemp(pinTempSensor);


  delay(100);
  // Set Led Brightness proportional to temperature
  if(temperature < maxTempLED){
    if(temperature <= minTempLED){
      brightness = 255;
    }
    else{
      brightness = 255 * (maxTempLED - temperature)/(maxTempLED - minTempLED);
    }
  }
  analogWrite(LED_PIN, brightness);
  
  // Set Fan Speed proportional to temperature
  if(temperature >= minTempFan){
    if(temperature > maxTempFan){
      speed = 255;
    }
    else{
      speed = 255 * (temperature - minTempFan)/(maxTempFan - minTempFan);
    }
  }
  // analogWrite(FAN_PIN, speed);
  
  // Serial.print("Speed: ");
  // Serial.println(speed);
  // Serial.print("Brightness: ");
  // Serial.println(brightness);

  // Serial.print("People: ");
  // Serial.println(people);

  //checkSound();
  Serial.println(isFirstDisplay);
  changeThreshold();
  displayOnLCD(temperature, brightness, speed);
  delay(3000);

}