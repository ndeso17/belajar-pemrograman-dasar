#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include <base64.hpp>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <PCF8575.h>
#include "MUX74HC4067.h"
#include <Servo.h>

//? Servo
Servo myServo;
//? Multiplexer
//! MUX74HC4067 DigitalMux(7,6,5,4,3) ; // Pin en, s0,s1,s2,s3
//! int PIN;
MUX74HC4067 AnalogMux(16,5,4,14,12);
//? Pin L298N Analog / PWMValue
int EN_A;
int EN_B;

//? Speed
#define SPEED_25 64   
#define SPEED_50 128  
#define SPEED_75 192  
#define SPEED_100 255 
//? Extender PCF8575
#define PCF8575_ADDRESS1 0x20
PCF8575 pcf8575(PCF8575_ADDRESS1);
//? Extender PCF8575 Pin L298N Channel A dan B
#define CHA1 0
#define CHA2 1
#define CHB1 2
#define CHB2 3
//? Extender PCF8575 Pin Servo
#define SERVO1 4
#define SERVO2 5
#define SERVO3 6
//? Extender PCF8575 Pin Ultrasonik & Buzzer
#define TRIG_PIN 7
#define ECHO_PIN 8
#define BUZZER_PIN 9

bool start; 
int speed;
int nomorservo;

const char* ssid = "Kamar Internet";          
const char* password = "080825kamarinternet";
//? NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
//? URL server nodemcu
const char* serverUrl = "http://192.168.17.250:1711/nodemcu";
//? Middleware Quest
String jwt = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6InVzZXIiLCJyb2xlIjoidXNlciIsImlhdCI6MTczNTg4NTM5MCwiZXhwIjoxNzM2NDkwMTkwfQ.USxKaKEc59nDo-YB8hhex-KH18ZDDfLvzoThvs-jhMtEdHm_4m1v0gk47OAsOj8TwbYF3SnulK3rG4ix2-hkyiG1yyhHZUMmJJkRR-6lUEZwkG93buK2ADOLv8VIOtDLWAilfclqzXY6FtaWmkMzwMniJ7jbm9WC8FCo0z4AnPFnx9HHoKR2YP68KtFytYB60rh75a9ndgWP9i1LGM19aI8gf157yrng8lp62aG1QdlKwKCQbg-QJnLYDBexKuRftcbe01POSE7Mr8PoTiLgXoN2cGF3UB91Gv-vigfHgiACZsOGHMcCskZ5fQQT7fmg7gVBC97DRax4nwdAeGFE9Q";
const char* rsaPublicKey = "-----BEGIN PUBLIC KEY-----\\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAic16mlNgYJZSxNHCTYu9\\nE9szwGWiJ+l2/fo+pRpK04oUPI5eX4fq36nvyWjf7AcqBO4t/p5EHL/ATVzVN0+4\\nRblUewXRgYny2DN+4tMLQNtM1tp1CIM2T3n+aol+lsbT0KiQzxM1q4E03ptDtPMV\\nBL23OJzNK9seMQhJICxHnQdTm58U6wa5Fw3gi9LBF7F/++8vUGvcUXWea7NKEscj\\ncuyFtuQqI5Q24vZZWFCSs4F0bPcFJBUuFkvXVhHAWXzWqn7EtMMPGCeDCiReOU53\\nZKUwaYj7jXLyB2HAIZf1zdSrfjiG/3SYqe6K9akFN/Z5sTtPae7znO3bGfI08uZ4\\nSwIDAQAB\\n-----END PUBLIC KEY-----";

//? Fungsi untuk decoding payload JWT
void decodeJwtPayload(const char* jwt, char* decodedPayload, size_t bufferSize) {
  String jwtString = String(jwt);
  int firstDot = jwtString.indexOf('.');
  int secondDot = jwtString.indexOf('.', firstDot + 1);

  if (firstDot == -1 || secondDot == -1) {
    Serial.println("Invalid JWT format");
    return;
  }

  //? Ambil bagian payload (base64)
  String payloadBase64 = jwtString.substring(firstDot + 1, secondDot);

  //? Base64 decoding membutuhkan padding '=' jika panjangnya bukan kelipatan 4
  while (payloadBase64.length() % 4 != 0) {
    payloadBase64 += '=';
  }

  //? Decode payload base64
  unsigned char payloadBase64Char[payloadBase64.length() + 1];
  payloadBase64.toCharArray((char*)payloadBase64Char, payloadBase64.length() + 1);
  unsigned int decodedLength = decode_base64(payloadBase64Char, (unsigned char*)decodedPayload);
  decodedPayload[decodedLength] = '\0';  // Null-terminate string
}
//? Fungsi untuk memeriksa apakah JWT sudah expired
bool isJwtExpired(const char* jwt) {
  char decodedPayload[512];  //? Buffer untuk payload yang sudah di-decode
  decodeJwtPayload(jwt, decodedPayload, sizeof(decodedPayload));

  //? Parse JSON dari payload
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, decodedPayload);
  if (error) {
    //! Serial.print("deserializeJson() failed: ");
    //! Serial.println(error.c_str());
    return true;  //? Jika gagal parse, anggap JWT expired
  }

  //? Ambil nilai 'exp' dari JSON
  if (!doc.containsKey("exp")) {
    //! Serial.println("JWT does not contain 'exp' field");
    return true;  //? Jika tidak ada 'exp', anggap JWT expired
  }

  long exp = doc["exp"];  //? Nilai waktu kedaluwarsa (UNIX timestamp)
  long currentTime = timeClient.getEpochTime();  //? Waktu saat ini (UNIX timestamp)

  //! Serial.print("Current Time: ");
  //! Serial.println(currentTime);
  //! Serial.print("JWT Expiration Time: ");
  //! Serial.println(exp);

  //? Periksa apakah JWT sudah kedaluwarsa
  if (currentTime >= exp) {
    return true;  //? JWT sudah kedaluwarsa
  } else {
    return false;  //? JWT masih valid
  }
}

String cekJwt() {
  bool jwtExpired = isJwtExpired(jwt.c_str());
  const String status = jwtExpired ? "true" : "false";
  return status;
}
//! getAllValue start,speed,nomorservo
void getAllValue(bool &start, int &speed, int &nomorservo) {
  nomorservo = 0;
  start = false;
  speed = 0;

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    const char* endpoint = "/getAllValue";
    String fullUrl = String(serverUrl) + String(endpoint);
    http.begin(client, fullUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(jwt));

    //? Payload JSON
    String jsonPayload = "{\"nodemcuid\":\"krenova2024\","
                         "\"kunciPublik\":\"" + String(rsaPublicKey) + "\"}";
    int httpResponseCode = http.POST(jsonPayload);

    //? Tampilkan hasil respons
    if (httpResponseCode > 0) {
      //! Serial.print("HTTP Response Code: ");
      //! Serial.println(httpResponseCode);

      String response = http.getString();
      //? Parsing JSON respons
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);
      if (!error) {
        if (doc.containsKey("data")) {
          start = doc["data"]["start"];
          speed = doc["data"]["speed"];
          nomorservo = doc["data"]["nomorservo"];
        } else {
          Serial.println("Field 'data' tidak ditemukan dalam respons JSON. Void getNomorServo.");
        }
      } else {
        Serial.print("Void getNomorServo. Error parsing JSON: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("Void getNomorServo. Error on sending POST: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Void getNomorServo. Wi-Fi not connected");
  }
}
//! Request JWT
void requestNewJwt() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    const char* apiUrl = "http://192.168.17.254:1711";
    const char* endpoint = "/generateJWT";
    String fullUrl = String(apiUrl) + String(endpoint);
    http.begin(client, fullUrl);
    http.addHeader("Content-Type", "application/json");
    String jsonPayload = "{\"username\":\"user\","
                         "\"password\":\"user123\","
                         "\"publicKey\":\"" + String(rsaPublicKey) + "\"}";
    int httpResponseCode = http.POST(jsonPayload);
    //? Tampilkan hasil respons
    if (httpResponseCode > 0) {
      //! Serial.print("HTTP Response Code: ");
      //! Serial.println(httpResponseCode);

      String response = http.getString();
      //! Serial.println("Server Response:");
      //! Serial.println(response);

      //? Parsing JSON respons
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);
      if (!error && doc.containsKey("data")) {
        jwt = doc["data"].as<String>();  //? Perbarui nilai JWT
        //! Serial.println("JWT diperbarui:");
        //! Serial.println(jwt);
        //! String statusJwt = cekJwt();
        //! Serial.println("Status Exp JWT:");
        //! Serial.println(statusJwt);
      } else {
        Serial.println("Gagal mem-parsing JSON atau field 'data' tidak ditemukan.");
      }
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Wi-Fi not connected");
  }
}

//! Void start conveyor
void startConveyor(int speed, bool direction) {
  int pwmValue;

  switch (speed) {
    case 1: pwmValue = SPEED_25; break;
    case 2: pwmValue = SPEED_50; break;
    case 3: pwmValue = SPEED_75; break;
    case 4: pwmValue = SPEED_100; break;
    default: pwmValue = 0; //? Jika speed = 0 atau tidak valid
  }
  //! Atur arah motor berdasarkan direction
  pcf8575.digitalWrite(CHA1, direction ? HIGH : LOW); 
  pcf8575.digitalWrite(CHA2, direction ? LOW : HIGH);
  analogWrite(EN_A, pwmValue);
  pcf8575.digitalWrite(CHB1, direction ? HIGH : LOW); 
  pcf8575.digitalWrite(CHB2, direction ? LOW : HIGH);
  analogWrite(EN_B, pwmValue);
  if (pwmValue == 0) {
    Serial.println("Conveyor berhenti");
  } else {
    Serial.println("Conveyor berjalan dengan kecepatan: " + String(speed));
  }
}
//! Void Servo
void setServoAngle(uint8_t pin, int angle) {
  int pulseWidth = map(angle, 0, 180, 500, 2500);  //? Konversi sudut ke lebar pulsa
  pcf8575.digitalWrite(pin, HIGH);                //? Sinyal HIGH untuk servo
  delayMicroseconds(pulseWidth);                  //? Pulsa sesuai lebar
  pcf8575.digitalWrite(pin, LOW);                 //? Sinyal LOW
  delay(20 - pulseWidth / 1000);                  //? Tunggu hingga periode 20 ms selesai
}
void servoController(uint8_t nomorservo, int angle) {
  //? Array untuk menyimpan pin servo
  uint8_t servoPins[] = {SERVO1, SERVO2, SERVO3};

  if (nomorservo >= 1 && nomorservo <= 3) {
    uint8_t selectedPin = servoPins[nomorservo - 1];  //? Pilih pin servo berdasarkan nomor
    setServoAngle(selectedPin, angle);               //? Setel sudut servo
  } else {
    Serial.println("Nomor servo tidak valid!");
  }
}
//! Void Ultrasonik & Buzzer
void ultraBuzzer(){
  // Kirim sinyal ultrasonik
  pcf8575.digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  pcf8575.digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  pcf8575.digitalWrite(TRIG_PIN, LOW);

  // Hitung durasi pantulan
  // long duration = pulseIn(ECHO_PIN, HIGH);
   // Deteksi awal pulsa (menunggu sinyal HIGH)
  unsigned long startTime = 0;
  unsigned long endTime = 0;
  // Tunggu hingga pin Echo menjadi HIGH (awal pulsa)
  while (pcf8575.digitalRead(ECHO_PIN) == LOW) {
    startTime = micros(); // Catat waktu awal pulsa
  }
  // Tunggu hingga pin Echo kembali LOW (akhir pulsa)
  while (pcf8575.digitalRead(ECHO_PIN) == HIGH) {
    endTime = micros(); // Catat waktu akhir pulsa
  }
  // Hitung durasi pulsa
  unsigned long duration = endTime - startTime;
  // Konversi durasi ke jarak (cm)
  float distance = duration * 0.034 / 2;

  // Tampilkan jarak di Serial Monitor
  Serial.print("Jarak: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Periksa apakah ada objek dalam jarak tertentu
  if (distance > 0 && distance < 20) { // Jika jarak objek kurang dari 20 cm
    pcf8575.digitalWrite(BUZZER_PIN, HIGH);    // Nyalakan buzzer
    delay(500);                        // Tunggu 0,5 detik
    pcf8575.digitalWrite(BUZZER_PIN, LOW);     // Matikan buzzer
  } else {
    pcf8575.digitalWrite(BUZZER_PIN, LOW);     // Pastikan buzzer mati jika tidak ada objek
  }

  //delay(100); // Tunggu sebelum pembacaan berikutnya
}
//! Main Void
void setup() {
  Serial.begin(115200);
  //? WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  //? NTP
  timeClient.begin();
  while (!timeClient.update()) {
    yield();  //? Hindari WDT reset selama sinkronisasi NTP
    delay(100);
  }

  Serial.println("\nConnected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  //? Inisialisasi pin PCF8575
  Wire.begin(0, 2);
  pcf8575.begin();
  //? L298N
  pcf8575.pinMode(CHA1, OUTPUT);
  pcf8575.pinMode(CHA2, OUTPUT);
  pcf8575.pinMode(CHB1, OUTPUT);
  pcf8575.pinMode(CHB2, OUTPUT);
  //? Servo
  pcf8575.pinMode(SERVO1, OUTPUT);
  pcf8575.pinMode(SERVO2, OUTPUT);
  pcf8575.pinMode(SERVO3, OUTPUT);
  //? Ultrasonik & Buzzer
  pcf8575.pinMode(TRIG_PIN, OUTPUT);
  pcf8575.pinMode(ECHO_PIN, INPUT);
  pcf8575.pinMode(BUZZER_PIN, OUTPUT);
  //? Inisialisasi pin MUX
  AnalogMux.signalPin(A0,OUTPUT,ANALOG);
  
  String statusJwt = cekJwt();
  if (statusJwt == "false") {
    getAllValue(start, speed, nomorservo);
  } else {
    Serial.println("Token JWT sudah kadaluarsa");
    requestNewJwt();
  }

  
}

void loop() {
  // ultraBuzzer();
  static unsigned long lastRequestTime = 0;  //? Melacak waktu terakhir request
  unsigned long currentTime = millis();

  //? Periksa apakah sudah 2 detik sejak request terakhir
  if (currentTime - lastRequestTime >= 2000) {
    lastRequestTime = currentTime;

    // Cek status JWT
    String statusJwt = cekJwt();
    if (statusJwt == "true") {
      Serial.println("Token JWT sudah kadaluarsa, memperbarui...");
      requestNewJwt();

      // Gunakan timer berbasis millis untuk jeda 3 menit, bukan delay
      static unsigned long jwtRenewalStart = millis();
      while (millis() - jwtRenewalStart < 180000) {
        // Tetap memproses pembaruan waktu atau operasi lainnya
        timeClient.update();
      }
    } else {
      getAllValue(start, speed, nomorservo);
      //? Control Conveyor
      if (start) {
        startConveyor(speed, start);
      } else {
        Serial.println("Conveyor tidak aktif");
      }
      //? Control Servo
      if (nomorservo <= 0 || nomorservo > 3) { 
        Serial.print("Servo Tidak Dikenali, nomorservo = ");
        Serial.println(nomorservo);
      } else {
        Serial.print("Mengontrol servo nomor: ");
        Serial.println(nomorservo);
        servoController(nomorservo, 90); //? Gerakkan servo ke sudut 90
      }
    }
  }

  // Perbarui waktu NTP secara periodik
  timeClient.update();
}
