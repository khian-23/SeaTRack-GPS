#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// ================= WIFI =================
#define WIFI_SSID "RENTILLOBH2024"
#define WIFI_PASSWORD "IKAWBAHALA3000"

// ================= FIREBASE =================
#define FIREBASE_URL "https://seatrack-e6acd-default-rtdb.asia-southeast1.firebasedatabase.app/gps.json"

// ================= BOAT INFO =================
#define BOAT_NAME "FB Toto Christian12"
#define BOAT_OWNER "Flores Aila Mae"

// ================= GPS =================
SoftwareSerial gpsSerial(D2, D1);
TinyGPSPlus gps;

// Timing
unsigned long lastSend = 0;
const unsigned long interval = 10000; // 🔥 10 seconds

// ================= SEND FUNCTION =================
void sendToFirebase(float lat, float lng, int sats, float speed)
{
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  Serial.println("Connecting to Firebase...");

  if (http.begin(client, FIREBASE_URL))
  {
    http.addHeader("Content-Type", "application/json");

    // 🔥 JSON DATA
    String json = "{";
    json += "\"latitude\":" + String(lat, 6) + ",";
    json += "\"longitude\":" + String(lng, 6) + ",";
    json += "\"satellites\":" + String(sats) + ",";
    json += "\"speed_kmh\":" + String(speed, 2) + ",";
    json += "\"boat_name\":\"" + String(BOAT_NAME) + "\",";
    json += "\"owner\":\"" + String(BOAT_OWNER) + "\"";
    json += "}";

    int httpCode = http.PUT(json);

    if (httpCode > 0)
    {
      Serial.print("HTTP OK: ");
      Serial.println(httpCode);
    }
    else
    {
      Serial.print("HTTP FAILED: ");
      Serial.println(http.errorToString(httpCode));
    }

    http.end();
  }
  else
  {
    Serial.println("HTTP begin FAILED");
  }
}

// ================= SETUP =================
void setup()
{
  Serial.begin(9600);
  gpsSerial.begin(9600);

  Serial.println("\nStarting system...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());
}

// ================= LOOP =================
void loop()
{
  // Always read GPS
  while (gpsSerial.available())
  {
    gps.encode(gpsSerial.read());
  }

  // Debug
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000)
  {
    lastDebug = millis();

    Serial.println("\n--- GPS DEBUG ---");
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());

    Serial.print("Location valid: ");
    Serial.println(gps.location.isValid() ? "YES" : "NO");
  }

  // Send condition
  if (gps.location.isValid() && gps.satellites.value() > 3)
  {
    if (millis() - lastSend > interval)
    {
      lastSend = millis();

      float lat = gps.location.lat();
      float lng = gps.location.lng();
      int sats = gps.satellites.value();
      float speed = gps.speed.kmph(); // 🔥 NEW

      Serial.println("\n===== GPS DATA =====");
      Serial.print("Lat: ");
      Serial.println(lat, 6);
      Serial.print("Lng: ");
      Serial.println(lng, 6);
      Serial.print("Speed (km/h): ");
      Serial.println(speed);
      Serial.print("Satellites: ");
      Serial.println(sats);

      sendToFirebase(lat, lng, sats, speed);

      Serial.println("====================");
    }
  }
}