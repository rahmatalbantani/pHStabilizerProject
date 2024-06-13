#include "SoftwareSerial.h"
#include <LCD_I2C.h>
LCD_I2C lcd(0x27, 20, 4);
#define MAX_ARRAY_SIZE 6
String dt[MAX_ARRAY_SIZE];  // Deklarasi array data
String dataIn;
bool parsing = false;
String receivedData = "";

SoftwareSerial mySerial(4, 12);  //pin rx tx

const int sensorpH = A1;  //pin pH
float Po = 0;
float kalibration = 0.205;
// Definisi variabel untuk fungsi keanggotaan pH
float pH_low = 4.5;
float pH_high = 8.5;
float pHValue;
// Definisi variabel untuk fungsi keanggotaan kecepatan motor
uint8_t speed_low = 100;
uint8_t speed_high = 255;
uint8_t pengaduk = 2;  //Pin adukan
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
  Po = 7.00 + ((2.676 - TeganganPh) / kalibration);
  //Serial.print("Nilai PH cairan: ");
  // Serial.println(Po, 3);
  delay(300);
  mySerial.write(Po);
  delay(200);
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
  // Variabel statis untuk melacak waktu terakhir LCD diperbarui
  static unsigned long previousMillis = 0;
  const long interval = 1000; // Interval waktu dalam milidetik (1 detik)
  
  // Dapatkan waktu saat ini
  unsigned long currentMillis = millis();

  // Periksa apakah sudah waktunya untuk memperbarui LCD
  if (currentMillis - previousMillis >= interval) {
    // Simpan waktu pembaruan terakhir
    previousMillis = currentMillis;

    // Bersihkan LCD sebelum memperbarui isinya
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

void getdata(){
    // Check if data is available on the serial port
  if (mySerial.available() > 0) {
    // Read the incoming data
    char incomingByte = mySerial.read();
    
    // If the character is a newline, process the data
    if (incomingByte == '\n') {
      // Remove the '#' characters and split the data
      int firstHash = receivedData.indexOf('#');
      int secondHash = receivedData.indexOf('#', firstHash + 1);
      int thirdHash = receivedData.indexOf('#', secondHash + 1);

      // Extract the values from the received string
      if (firstHash >= 0 && secondHash >= 0 && thirdHash >= 0) {
        nutrisi = receivedData.substring(firstHash + 1, secondHash).toInt();
        pHmin = receivedData.substring(secondHash + 1, thirdHash).toInt();
        pHplus = receivedData.substring(thirdHash + 1).toInt();
        Serial.println(String()+"Nutrisi : "+nutrisi+"\t -: "+pHmin+"\t +: "+pHplus);
      }

      // Clear the received data string
      receivedData = "";
    } else {
      // Add the incoming byte to the received data string
      receivedData += incomingByte;
    }
  }
}
//    set_point_ph = dataIn[0];
//    set_point_nutrisi = dataIn[1];


void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  // Mengatur pin sebagai output
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(pengaduk, OUTPUT);
  pinMode(sensorpH, INPUT);

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("==Welcome===");
  delay(2000);
  lcd.clear();
}

void loop() {
  getdata();
  bacaNilaiSensorPH();
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