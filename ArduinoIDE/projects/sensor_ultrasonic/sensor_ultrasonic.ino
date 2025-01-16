#include <Arduino.h>

const int TRIGPIN = 16; //? PIN D0 pada Wemos D1 R2          
const int ECHOPIN = 5; //? PIN D1 pada Wemos D1 R2
long timer;
int jarak;

void setup()
{
  Serial.begin(115200);
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
}

void loop()
{
  digitalWrite(TRIGPIN, LOW);                   
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);                  
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);                   

  timer = pulseIn(ECHOPIN, HIGH);
  jarak = timer / 58;  //? Menghitung jarak dalam cm
  delay(1000);

  if (jarak >= 100) {  
    //? Jika jarak mencapai 1 meter atau lebih
    float jarakMeter = jarak / 100.0;  //! Konversi ke meter
    Serial.print("Jarak = ");
    Serial.print(jarak);
    Serial.print(" cm / ");
    Serial.print(jarakMeter, 2);  //! Tampilkan 2 angka di belakang koma
    Serial.println(" meter");
  } else {  
    //? Jika jarak kurang dari 1 meter
    Serial.print("Jarak = ");
    Serial.print(jarak);
    Serial.println(" cm");
  }
}
