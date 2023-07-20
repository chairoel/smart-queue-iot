#include <DMD32.h>
#include "fonts/Arial_black_16.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DFRobotDFPlayerMini.h"

#define WIFI_SSID "WifiNih1"
#define WIFI_PASSWORD "mautauaja"

// #define WIFI_SSID "DEDY"
// #define WIFI_PASSWORD "sumantri"

#define INDICATOR_WIFI 2  // LED BUILD IN
#define GET_DATA 15
#define CHECK_DEVICE_ON 5

#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

hw_timer_t* timer = NULL;

int currentStateGetData = 1;
int currentStateDevice = 0;
DFRobotDFPlayerMini myMP3;

const char* previousQueue = "";

void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}


void setup(void) {
  Serial2.begin(9600);
  Serial.begin(115200);
  connectWifi();
  setupLedP10();
  setupMP3Module();

  Serial.println("-----Start Program-----");
  pinMode(GET_DATA, INPUT);
  pinMode(CHECK_DEVICE_ON, INPUT);
}


void loop(void) {
  currentStateGetData = digitalRead(GET_DATA);
  currentStateDevice = digitalRead(CHECK_DEVICE_ON);
  //5 & 15

  if ((currentStateGetData == LOW) && (currentStateDevice == HIGH)) {
    Serial.printf("Data: %d\n", currentStateGetData);
    sendGetRequest();
  }
}

void sendGetRequest() {
  HTTPClient http;
  String url = "https://api.smartqueue.my.id/api/v1/antrian/queue/mk/terkini";

  http.begin(url);

  int httpResponseCode = http.GET();

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
    // previousQueue = nomorAntrian.c_str();


    printData(status, message, nomorAntrian, kategori, insertDate);
  } else {
    Serial.printf("Error code: %d\n", httpResponseCode);
  }

  http.end();
}

void printData(bool status,
               const String& message,
               const String& nomorAntrian,
               const String& kategori,
               const String& insertDate) {
  Serial.printf("Status: %s\n", status ? "true" : "false");

  int counterAnak = 0;
  int counterGigi = 100;
  int counterUmum = 200;

  if (status) {
    String antrianString = nomorAntrian.c_str();
    int antrianInt = antrianString.substring(1).toInt();
    printLedP10(nomorAntrian.c_str());

    if (kategori == "ANAK") {
      counterAnak = counterAnak + antrianInt;
      Serial.printf("Counter Anak: %d \n", counterAnak);
      myMP3.play(counterAnak);

    } else if (kategori == "GIGI") {
      counterGigi = counterGigi + antrianInt;
      Serial.printf("Counter Gigi: %d \n", counterGigi);
      myMP3.play(counterGigi);

    } else if (kategori == "UMUM") {
      counterUmum = counterUmum + antrianInt;
      Serial.printf("Counter Umum: %d \n", counterUmum);
      myMP3.play(counterUmum);
    }
  } else {
    Serial.printf("Message: %s \n", message.c_str());
  }
  delay(3000);
}

void printLedP10(const char* MSG) {
  dmd.clearScreen(true);
  dmd.selectFont(Arial_Black_16);
  dmd.drawString(0, 0, MSG, strlen(MSG), GRAPHICS_NORMAL);
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

void setupLedP10() {
  uint8_t cpuClock = ESP.getCpuFreqMHz();
  timer = timerBegin(0, cpuClock, true);
  timerAttachInterrupt(timer, &triggerScan, true);
  timerAlarmWrite(timer, 300, true);
  timerAlarmEnable(timer);
  dmd.clearScreen(true);
}

void setupMP3Module() {
  if (!myMP3.begin(Serial2, true)) {
    Serial.println("Gagal");
    while (true)
      ;
  }
  myMP3.volume(30);
}
