#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// ================= WIFI =================
#define WIFI_SSID "vivo Y16"
#define WIFI_PASSWORD "12345678"

// ================= FIREBASE (HTTP REST) =================
#define FIREBASE_HOST "http://seatrack-e6acd-default-rtdb.asia-southeast1.firebasedatabase.app"

// ================= GPS =================
SoftwareSerial gpsSerial(D2, D1); // RX, TX
TinyGPSPlus gps;

// Timing
unsigned long lastSend = 0;
const unsigned long interval = 5000;

// ================= SEND TO FIREBASE =================
void sendToFirebase(float lat, float lng, int sats)
{
  WiFiClient client;
  HTTPClient http;

  String url = String(FIREBASE_HOST) + "/gps.json";

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"latitude\":" + String(lat, 6) + ",";
  json += "\"longitude\":" + String(lng, 6) + ",";
  json += "\"satellites\":" + String(sats);
  json += "}";

  int httpResponseCode = http.PUT(json);

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);
    Serial.println("Firebase update SUCCESS ✅");
  }
  else
  {
    Serial.print("HTTP ERROR ❌: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void setup()
{
  Serial.begin(9600);
  gpsSerial.begin(9600);

  Serial.println("\nStarting system...");

  // ========= WIFI =========
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Internet test
  WiFiClient testClient;
  if (testClient.connect("google.com", 80))
  {
    Serial.println("Internet OK ✅");
  }
  else
  {
    Serial.println("No Internet ❌");
  }
}

void loop()
{
  while (gpsSerial.available())
  {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid())
  {
    if (millis() - lastSend > interval)
    {
      lastSend = millis();

      float lat = gps.location.lat();
      float lng = gps.location.lng();
      int sats = gps.satellites.value();

      Serial.println("\n===== GPS DATA =====");
      Serial.print("Lat: ");
      Serial.println(lat, 6);
      Serial.print("Lng: ");
      Serial.println(lng, 6);
      Serial.print("Satellites: ");
      Serial.println(sats);

      // 🔥 SEND USING HTTP (WORKS ON HOTSPOT)
      sendToFirebase(lat, lng, sats);

      Serial.println("====================");
    }
  }
  else
  {
    Serial.println("Waiting for GPS fix...");
    delay(2000);
  }
}