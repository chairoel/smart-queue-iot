#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include "Adafruit_Thermal.h"

#define WIFI_SSID "WifiNih1"
#define WIFI_PASSWORD "mautauaja"

#define INDICATOR_WIFI 2  // LED BUILD IN
#define BUTTON_ANAK 5
#define BUTTON_UMUM 18
#define BUTTON_GIGI 19

int currentStateAnak = 0;
int currentStateUmum = 0;
int currentStateGigi = 0;


Adafruit_Thermal printer(&Serial2);

void setup() {

  Serial2.begin(9600);
  Serial.begin(115200);
  printer.begin();
  connectWifi();
  // Set time zone
  setTime(0);  // UTC time

  // Add time offset
  adjustTime(7 * 3600);  // Add 7 hours offset
  Serial.println("-----Start Program-----");
  // sendPostRequest();

  // pinMode(BUTTON_ANAK, INPUT_PULLUP);
  // pinMode(BUTTON_UMUM, INPUT_PULLUP);
  // pinMode(BUTTON_GIGI, INPUT_PULLUP);
  pinMode(BUTTON_ANAK, INPUT);
  pinMode(BUTTON_UMUM, INPUT);
  pinMode(BUTTON_GIGI, INPUT);
}

void loop() {

  currentStateAnak = digitalRead(BUTTON_ANAK);
  currentStateUmum = digitalRead(BUTTON_UMUM);
  currentStateGigi = digitalRead(BUTTON_GIGI);

  if (currentStateAnak == LOW) {
    sendPostRequest("ANAK");
    Serial.println("BTN ANAK");
    
  } else if (currentStateUmum == LOW) {
    sendPostRequest("UMUM");
    Serial.println("BTN UMUM");
    
  } else if (currentStateGigi == LOW) {
    sendPostRequest("GIGI");
    Serial.println("BTN GIGI");
  }

}

void sendPostRequest(String caterory) {
  HTTPClient http;
  String url = "https://api.smartqueue.my.id/api/v1/antrian/queue/mk";

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

    // Format insertDate
    String formattedDateTime = formatDateTime(insertDate);
    Serial.printf("Waktu: %s \n", formattedDateTime.c_str());

    printData(status, message, nomorAntrian, kategori, formattedDateTime);
  } else {
    Serial.printf("Error code: %d\n", httpResponseCode);
  }

  http.end();
}

String formatDateTime(const String& originalDateTime) {
  // Parsing original date/time
  int year, month, day, hour, minute, second;
  sscanf(originalDateTime.c_str(), "%d-%d-%dT%d:%d:%d",
         &year, &month, &day, &hour, &minute, &second);

  // Adjust hour
  hour += 7;
  if (hour >= 24) {
    hour -= 24;
    day += 1;
  }

  // Formatting date/time
  char formattedDateTime[20];
  sprintf(formattedDateTime, "%02d-%02d-%04d %02d:%02d:%02d",
          day, month, year, hour, minute, second);

  return String(formattedDateTime);
}

void printData(bool status,
               const String& message,
               const String& nomorAntrian,
               const String& kategori,
               const String& formattedInsertDate) {

  Serial.printf("Status: %s\n", status ? "true" : "false");

  if (status) {
    printer.boldOff();
    printer.justify('C');
    printer.setSize('L');
    printer.setFont('B');

    printer.println("=== SMART QUEUE ===");
    printer.println("");

    printer.setSize('M');
    printer.setCharSpacing(2);
    printer.println("Nomor Antrian Anda:");
    printer.setSize('L');
    printer.println(nomorAntrian.c_str());
    printer.println("");

    printer.setSize('S');
    printer.setCharSpacing(0);
    printer.println(formattedInsertDate.c_str());
    printer.feed(3);

    printer.sleep();
    delay(3000L);
    printer.wake();
    printer.setDefault();
  } else {
    Serial.printf("Message: %s\n", message.c_str());
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
