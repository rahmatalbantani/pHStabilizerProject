#include <Wire.h>

#define SLAVE_ADDRESS 0x04

uint16_t var1 = 0;
uint16_t var2 = 123;
uint16_t var3 = 456;

void setup() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(requestEvent);
  Serial.begin(115200);
}

void loop() {
  var1 += 1; // Mengubah nilai var1 setiap loop


  // Menampilkan data yang akan dikirim ke Serial Monitor


  // Menunggu sebelum mengirim data
  delay(1000);
}

void requestEvent() {
  byte data[6];
  memcpy(data, &var1, sizeof(var1));  // Menyalin 2 byte dari var1 ke data
  memcpy(data + 2, &var2, sizeof(var2));  // Menyalin 2 byte dari var2 ke data
  memcpy(data + 4, &var3, sizeof(var3));  // Menyalin 2 byte dari var3 ke data

  Wire.write(data, 6); // Mengirim data ke master (Arduino Nano)
  Serial.print("Sending: ");
  Serial.print(var1);
  Serial.print(" ");
  Serial.print(var2);
  Serial.print(" ");
  Serial.println(var3);
}
