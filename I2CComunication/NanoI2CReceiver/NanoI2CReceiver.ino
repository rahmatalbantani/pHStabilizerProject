#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ESP32_ADDRESS 0x04

uint16_t var1 = 0;
uint16_t var2 = 0;
uint16_t var3 = 0;

bool newDataAvailable = false; // Flag untuk menandakan data baru sudah tersedia

LiquidCrystal_I2C lcd(0x27, 16, 2); // Sesuaikan alamat LCD I2C dan ukuran

void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  lcd.init(); // Inisialisasi LCD
  lcd.backlight(); // Mengaktifkan backlight LCD
  lcd.print("I2C Receiver Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Memanggil fungsi updateData untuk memperbarui data dari ESP32
  updateData();

  // Jika ada data baru yang tersedia, tampilkan di LCD
    displayLCD();
  
}

void displayLCD() {
  if(newDataAvailable){
  
  lcd.setCursor(0, 0);
  lcd.print("var1: ");
  lcd.print(var1);
  
  lcd.setCursor(0, 1);
  lcd.print("var2: ");
  lcd.print(var2);
  lcd.print(" var3: ");
  lcd.print(var3);
  newDataAvailable = false; // Reset flag setelah menampilkan data

  }
  else{Serial.println("Data Belum Tersedia");}
}

void updateData() {
  // Meminta data baru dari ESP32
  Wire.requestFrom(ESP32_ADDRESS, 6);

  // Memeriksa apakah ada data yang diterima dari ESP32
  if (Wire.available() >= 6) {
    byte data[6];
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }

    // Memparsing data yang diterima
    var1 = *((uint16_t*)&data[0]);
    var2 = *((uint16_t*)&data[2]);
    var3 = *((uint16_t*)&data[4]);

    // Set flag bahwa data baru sudah tersedia
    newDataAvailable = true;
  }
}
