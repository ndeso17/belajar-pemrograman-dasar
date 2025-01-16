#include <Arduino.h>

const int pirPin = 16;  //? Pin Sensor PIR pada Wemos D1 R2, 16 = D0

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
}

void loop() {
  int pirState = digitalRead(pirPin);
  
  if (pirState == HIGH) {
    Serial.println("Gerakan terdeteksi!");
  } else {
    Serial.println("Tidak ada gerakan.");
  }

  delay(500);
}
