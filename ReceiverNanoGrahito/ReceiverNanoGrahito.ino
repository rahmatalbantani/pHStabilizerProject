#include "SoftwareSerial.h"
#include <Wire.h>
#include <LCD_I2C.h>
#define MAX_ARRAY_SIZE 6
#define ESP32_ADDRESS 0x04

LCD_I2C lcd(0x27, 20, 4);
String dt[MAX_ARRAY_SIZE];  // Deklarasi array data
String dataIn;
bool parsing = false;
String receivedData = "";
bool newDataAvailable = false; // Flag untuk menandakan data baru sudah tersedia


SoftwareSerial mySerial(4, 12);  //pin rx tx

const int sensorpH = A1;  //pin pH
float Po = 0;
float Pobefore=0;
float nosumpo=0;
int iterasi=1;
float kalibration = 0.0802;
// Definisi variabel untuk fungsi keanggotaan pH
float pH_low = 4.5;
float pH_high = 8.5;
float pHValue;
// Definisi variabel untuk fungsi keanggotaan kecepatan motor
uint8_t speed_low = 100;
uint8_t speed_high = 255;
uint8_t pengaduk = 4;  //Pin adukan
uint8_t tinggi_default = 5;
uint16_t set_point_nutrisi, set_point_ph;
uint32_t nutrisi, pHplus, pHmin;
//mendefinisikan pin yang digunakan untuk control pin
// Mendefinisikan pin yang terhubung ke Motor Driver L298N
const int enA = 6;  // Pin PWM untuk kontrol kecepatan motor A
const int in1 = 7;  // Pin kontrol arah motor A
const int in2 = 8;  // Pin kontrol arah motor A

const int enB = 11;  // Pin PWM untuk kontrol kecepatan motor B
const int in3 = 9;   // Pin kontrol arah motor B
const int in4 = 10;  // Pin kontrol arah motor B

unsigned long waktu_sekarang;
// Fungsi keanggotaan untuk pH
//Saat Kondisi Low
float lowAcid(float pH) {
  if (pH <= pH_low) {
    return 1.0;
  } else if (pH > pH_low && pH < 7.0) {
    return (7.0 - pH) / (7.0 - pH_low);
  } else {
    return 0.0;
  }
}

//Saat Kondisi ideal
float ideal(float pH) {
  if (pH <= pH_low || pH >= pH_high) {
    return 0.0;
  } else if (pH >= 7.0) {
    return (pH - 7.0) / (pH_high - 7.0);
  } else {
    return (7.0 - pH) / (7.0 - pH_low);
  }
}

float highAlkaline(float pH) {
  if (pH >= pH_high) {
    return 1.0;
  } else if (pH > 7.0 && pH < pH_high) {
    return (pH - 7.0) / (pH_high - 7.0);
  } else {
    return 0.0;
  }
}

// Fungsi keanggotaan untuk kecepatan motor
int lowSpeed(float speed) {
  if (speed <= speed_low) {
    return 255;  // Inversi karena semakin rendah pH, semakin tinggi kecepatan motor
  } else if (speed > speed_low && speed < 200) {
    return map(speed, speed_low, 200, 255, 100);
  } else {
    return 100;
  }
}

int mediumSpeed(float speed) {
  if (speed <= 100 || speed >= 200) {
    return 0;
  } else if (speed >= 100 && speed < 150) {
    return map(speed, 100, 150, 0, 255);
  } else {
    return map(speed, 150, 200, 255, 0);
  }
}

int highSpeed(float speed) {
  if (speed <= 150) {
    return 0;
  } else if (speed > 150 && speed < speed_high) {
    return map(speed, 150, speed_high, 0, 255);
  } else {
    return 255;
  }
}

// Fungsi untuk mengatur kecepatan motor berdasarkan nilai pH
int adjustMotorSpeed(float pH) {
  int speed = 0;
  if (pH <= 7.0) {
    speed = lowSpeed(pH);
  } else {
    speed = highSpeed(pH);
  }
  return speed;
}

float bacaNilaiSensorPH() {
  int pengukuranPh = analogRead(sensorpH);
  double TeganganPh = 5 / 1024.0 * pengukuranPh;
  //Po = 7.00 + ((teganganPh7 - TeganganPh) / PhStep);
  nosumpo= 7.00 + ((3.385 - TeganganPh) / kalibration);
  Pobefore = Pobefore + nosumpo;
  Po = Pobefore/iterasi;

  //Serial.print("Nilai PH cairan: ");
  
  Serial.println(Po, 3);
  iterasi++;
  delay(500);
}

void sendpH(){
   if (millis() - waktu_sekarang >= 3000) {
    waktu_sekarang = millis();
    mySerial.println(Po);
  }


}

// Fungsi untuk mengatur kecepatan motor
void setMotorSpeedPlus(int speed) {
  // Batasi nilai kecepatan antara 0 dan 255
  speed = constrain(speed, 0, 255);
  // Atur kecepatan motor dengan nilai PWM
  analogWrite(enA, speed);
}

// Fungsi untuk mengatur kecepatan motor
void setMotorSpeedMinus(int speed) {
  // Batasi nilai kecepatan antara 0 dan 255
  speed = constrain(speed, 0, 255);
  // Atur kecepatan motor dengan nilai PWM
  analogWrite(enB, speed);
}

void update_lcd(String layar1, String layar2, String layar3, String layar4) {



    // Bersihkan LCD sebelum memperbarui isinya
    if(newDataAvailable){
    lcd.clear();
    
    lcd.setCursor(0, 0);  // Kolom 1
    lcd.print("pH:");
    lcd.setCursor(4, 0);
    lcd.print(layar1);
    
    lcd.setCursor(0, 1);  // Kolom 2
    lcd.print("Nutrisi:");
    lcd.setCursor(9, 1);
    lcd.print(layar2);
    lcd.setCursor(12, 1);
    lcd.print("Cm");
    
    lcd.setCursor(0, 2);  // Kolom 3
    lcd.print("+:");
    lcd.setCursor(3, 2);
    lcd.print(layar3);
    lcd.setCursor(6, 2);
    lcd.print("Cm");
    
    lcd.setCursor(0, 3);  // Kolom 4
    lcd.print("-:");
    lcd.setCursor(3, 3);
    lcd.print(layar4);
    lcd.setCursor(6, 3);
    lcd.print("Cm");
    newDataAvailable = false; // Reset flag setelah menampilkan data
    }
    else{//Serial.println("Data Belum Tersedia");
    }
  
}


// void send_serial() {
//   if (millis() - waktu_sekarang >= 3000) {
//     waktu_sekarang = millis();
//     mySerial.print("*");
//     mySerial.print(String(pHValue));
//     mySerial.print("#");
//   }
// }


void updateData() {
    static unsigned long previousMillis = 0;
  const long interval = 1000; // Interval waktu dalam milidetik (1 detik)
  
  // Dapatkan waktu saat ini
  unsigned long currentMillis = millis();

  // Periksa apakah sudah waktunya untuk memperbarui LCD
  if (currentMillis - previousMillis >= interval) {
    // Simpan waktu pembaruan terakhir
    previousMillis = currentMillis;
  // Meminta data baru dari ESP32
  Wire.requestFrom(ESP32_ADDRESS, 6);

  // Memeriksa apakah ada data yang diterima dari ESP32
  if (Wire.available() >= 6) {
    byte data[6];
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }

    // Memparsing data yang diterima
    nutrisi = *((uint16_t*)&data[0]);
    pHmin = *((uint16_t*)&data[2]);
    pHplus = *((uint16_t*)&data[4]);

    // Set flag bahwa data baru sudah tersedia
    newDataAvailable = true;
  }
  }
}


void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
  Wire.begin();
  // Mengatur pin sebagai output
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(pengaduk, OUTPUT);
  pinMode(sensorpH, INPUT);
    // Set motor 1 to move forward
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  // Set motor 2 to move forward
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("==Welcome===");
  Serial.println("Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  bacaNilaiSensorPH();
  sendpH();
  updateData();
  // Baca nilai pH dari sensor
  pHValue = Po;  // Fungsi ini harus Anda ganti sesuai dengan sensor pH yang Anda gunakan
  // Atur kecepatan motor berdasarkan nilai pH
  int motorSpeed = adjustMotorSpeed(pHValue);

update_lcd(String(pHValue), String(nutrisi), String(pHplus), String(pHmin));  //Update Tampilan



  if (pHValue < 7) {
    setMotorSpeedPlus(motorSpeed);  //Untuk pH naik
  } else if (pHValue >= 7) {
    setMotorSpeedMinus(motorSpeed);  //Untuk pH turun
  }

  if (pHValue != set_point_ph) { //Pengaduk disetting ketika tidak sesuai setpoint
    digitalWrite(pengaduk, LOW);
  } else {
    digitalWrite(pengaduk, HIGH);
  }

  // send_serial();  // kirim data serial ke nodemcu
}