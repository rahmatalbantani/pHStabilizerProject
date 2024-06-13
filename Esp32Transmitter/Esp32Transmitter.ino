#include <NewPing.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;



#define WIFI_SSID "UNTIRTA"
#define WIFI_PASSWORD "untirtajawara"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyA-qvRfmHv7OqGoWk2_k3ygbzp5F-jdevQ"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "rahmatgumilang123@gmail.com"
#define USER_PASSWORD "@Quickcepat1234"
#define DATABASE_URL "https://grahitota-default-rtdb.asia-southeast1.firebasedatabase.app"

void asyncCB(AsyncResult &aResult);

void printResult(AsyncResult &aResult);

DefaultNetwork network; // initilize with boolean parameter to enable/disable network reconnection
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
RealtimeDatabase Database;
AsyncResult aResult_no_callback;

TaskHandle_t FirebaseTask; // Handle untuk task Firebase
bool taskComplete = false;
unsigned long previousMillis = 0;
const long interval = 3000; // interval dalam milidetik (2 detik)
unsigned long waktu_sekarang = 0;
unsigned long lastFirebaseUpdate = 0;
unsigned long firebaseInterval = 3000; // interval to update Firebase in milliseconds


void setupfirebase(){
  ssl_client.setInsecure();
  initializeApp(aClient, app, getAuth(user_auth), asyncCB, "authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
  Serial.println("Asynchronous Set... ");

}

// Define pins
const int trigPin1 = 23;
const int echoPin1 = 22;
const int trigPin2 = 4;
const int echoPin2 = 5;
const int trigPin3 = 13;
const int echoPin3 = 14;
int Nutrisi, pHdown, pHup, st_nutrisi, st_pH, pH;
int buzzer = 18; // pin Buzzer
String tst_nutrisi;



int measureDistance(int trigPin, int echoPin) {
  long duration;
  int distance;
  
  // Clear the trigPin by setting it LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Trigger the sensor by setting the trigPin HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance (in cm) based on the speed of sound
  distance = duration * 0.034 / 2;

  return distance;
}

int pHPlusCapacity() {
  int value = measureDistance(trigPin1, echoPin1);
  return value; 
}

int pHMinCapacity() {
  int value = measureDistance(trigPin2, echoPin2);
  return value;
}

int NutrisiCapacity() {
  int value = measureDistance(trigPin3, echoPin3); 
  return value;
}

void UltrasonicInitialize() {
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
}

void send_serial() {
  if (millis() - waktu_sekarang >= 500) {
    waktu_sekarang = millis();
    Serial2.println(String() + "#" + Nutrisi + "#" + pHdown + "#" + pHup);
    Serial.println(String() + "Data Terkirim" + "#" + Nutrisi + "#" + pHdown + "#" + pHup+"\t|| NutrientLimit : "+st_nutrisi);
  }
}

void baca_kapasitas() {
  Nutrisi = NutrisiCapacity();
  delay(10);
  pHdown = pHMinCapacity();
  delay(10);
  pHup = pHPlusCapacity();
  delay(10);    
}

void connect_wifi() {
  // Initialize WiFiManager
  WiFiManager wifiManager;
  // Start WiFiManager configuration portal
  if (!wifiManager.autoConnect("Hidroponik Grahito")) {
    delay(3000);
    // Reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }
}

void sendfirebase(void * parameter) {
  for (;;) {
    // Memeriksa apakah aplikasi Firebase siap
    if (app.ready()) {
      Serial.println("Asynchronous Set... ");

      // Set int
      Database.set<int>(aClient, "/data/Display/NutrientCapacity", Nutrisi, asyncCB, "setIntTask");
      Database.set<int>(aClient, "/data/Display/pH+", pHup, asyncCB, "setIntTask");
      Database.set<int>(aClient, "/data/Display/pH-", pHdown, asyncCB, "setIntTask");
      // Set float
      Database.set<number_t>(aClient, "/data/Display/pHTerukur", number_t(pH, 2), asyncCB, "setFloatTask");
      Database.get(aClient, "/data/SetPoint/NutrientLimit", asyncCB, false, "getNutrientL");
      //Database.get(aClient, "/data/SetPoint/pH", asyncCB, false, "getTask2");
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS); // Delay 3 detik sebelum mengulangi
  }
}



void setup() {
  // Initialize Serial Monitor
  pinMode(buzzer, OUTPUT);
  UltrasonicInitialize();
  Serial.begin(115200);
  // Initialize Serial Port for communication
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17 (sesuaikan dengan koneksi ESP32 Anda)
  connect_wifi();
  setupfirebase();

    xTaskCreatePinnedToCore(
    sendfirebase,     // Fungsi yang akan dijalankan oleh task
    "FirebaseTask",  // Nama untuk task
    10000,           // Stacksize untuk task (disesuaikan sesuai kebutuhan)
    NULL,            // Parameter yang akan dikirim ke fungsi task (dalam hal ini tidak ada)
    1,               // Prioritas task (1 adalah prioritas default)
    &FirebaseTask,   // Variabel untuk menyimpan handle task
    1                // Nomor core di mana task akan dijalankan (core 1 pada ESP32 adalah nomor 0)
  );

}  


void loop() {
  app.loop();
  Database.loop();
  baca_kapasitas();
  send_serial();
     if (Nutrisi <= st_nutrisi) {
    digitalWrite(buzzer, HIGH);
  }else{
    digitalWrite(buzzer, LOW);
  }
}



void asyncCB(AsyncResult &aResult)
{
    // WARNING!
    // Do not put your codes inside the callback and printResult.

    printResult(aResult);
}

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }

    if (aResult.available())
    {
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    
    if(String(aResult.uid().c_str()) == "getNutrientL"){
      st_nutrisi = atoi(aResult.c_str());
      Firebase.printf("Data Berhasil di ambil");
      delay(100);
    }
    }
}