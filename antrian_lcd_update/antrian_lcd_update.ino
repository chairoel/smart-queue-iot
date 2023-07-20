#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID "WifiNih1"
#define WIFI_PASSWORD "mautauaja"

// #define WIFI_SSID "DEDY"
// #define WIFI_PASSWORD "sumantri"

#define INDICATOR_WIFI 2  // LED BUILD IN
#define BUTTON_ANAK_A 5
#define BUTTON_UMUM_A 18
#define BUTTON_GIGI_A 19
#define SEND_TO_MP3 4

int currentStateAnak = 0;
int currentStateUmum = 0;
int currentStateGigi = 0;

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void setup() {

  Serial.begin(115200);
  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();
  connectWifi();

  Serial.println("-----Start Program-----");
  lcd.setCursor(4, 0);
  lcd.print("Welcome");
  // sendPostRequest("ANAK");

  // pinMode(BUTTON_ANAK_A, INPUT_PULLUP);
  // pinMode(BUTTON_UMUM_A, INPUT_PULLUP);
  // pinMode(BUTTON_GIGI_A, INPUT_PULLUP);
  pinMode(BUTTON_ANAK_A, INPUT);
  pinMode(BUTTON_UMUM_A, INPUT);
  pinMode(BUTTON_GIGI_A, INPUT);

  pinMode(SEND_TO_MP3, OUTPUT);
  digitalWrite(SEND_TO_MP3, HIGH);
}

void loop() {

  currentStateAnak = digitalRead(BUTTON_ANAK_A);
  currentStateUmum = digitalRead(BUTTON_UMUM_A);
  currentStateGigi = digitalRead(BUTTON_GIGI_A);

  if (currentStateAnak == LOW) {
    sendPostRequest("ANAK");
    Serial.println("BTN ANAK");
    delay(2000);
  } else if (currentStateUmum == LOW) {
    sendPostRequest("UMUM");
    Serial.println("BTN UMUM");
    delay(2000);
  } else if (currentStateGigi == LOW) {
    sendPostRequest("GIGI");
    Serial.println("BTN GIGI");
    delay(2000);
  }
}

void sendPostRequest(String caterory) {
  HTTPClient http;
  String url = "https://api.smartqueue.my.id/api/v1/antrian/queue/next";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Create a JSON payload
  DynamicJsonDocument payload(200);
  payload["kategori"] = caterory;

  String jsonString;
  serializeJson(payload, jsonString);

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);

    bool status = doc["status"];
    String message = doc["message"].as<String>();

    JsonObject data = doc["data"];
    String nomorAntrian = data["nomorAntrian"].as<String>();
    String kategori = data["kategori"].as<String>();
    String insertDate = data["insertDate"].as<String>();


    // Serial.printf("Nomor Antrian: %s\n", nomorAntrian.c_str());
    // Serial.printf("Kategori: %s\n", kategori.c_str());
    // Serial.printf("Insert Date: %s\n", insertDate.c_str());

    printData(status, message, nomorAntrian, kategori);

  } else {
    Serial.printf("Error code: %d\n", httpResponseCode);
  }

  http.end();
}

void printData(bool status,
               const String& message,
               const String& nomorAntrian,
               const String& kategori) {

  Serial.printf("Status: %s\n", status ? "true" : "false");
  Serial.printf("Kategori: %s\n", kategori.c_str());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nomor Antrian: ");


  if (status) {
    lcd.setCursor(6, 1);
    lcd.print(nomorAntrian.c_str());
    digitalWrite(SEND_TO_MP3, LOW);
    delay(500);
    digitalWrite(SEND_TO_MP3, HIGH);
  } else {
    lcd.setCursor(1, 1);
    lcd.print(message.c_str());
  }
}


void connectWifi() {
  Serial.println("Connecting To Wifi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("Wifi Connected");
  Serial.println(WiFi.SSID());
  pinMode(INDICATOR_WIFI, OUTPUT);
  digitalWrite(INDICATOR_WIFI, HIGH);
}
