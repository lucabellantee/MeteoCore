/**
 * ESP32 Weather Station - Integration with ThingSpeak
 * 
 * Receives weather data from STM32 via UART, decodes it as JSON
 * and sends it to ThingSpeak via HTTP GET.
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// WiFi Configuration
const char* ssid = WIFI_ID;           
const char* password = WIFI_PASSWORD;   

// ThingSpeak Configuration
const char* thingSpeakApiKey = THING_SPEAK_API;  

// UART
#define RXD2 16
#define TXD2 17

String receivedData = "";
bool dataReady = false;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Weather Station");

  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP Address: " + WiFi.localIP().toString());
}

void loop() {
  while (Serial2.available()) {
    char c = Serial2.read();
    receivedData += c;
    if (c == '\n') {
      dataReady = true;
      break;
    }
  }

  if (dataReady) {
    Serial.println("Received data: " + receivedData);

    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, receivedData);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    } else {
      float temperature = doc["temp"];
      float pressure = doc["press"];
      float humidity = doc["hum"];
      float rainProb = doc["rain"];

      sendToThingSpeak(temperature, pressure, humidity, rainProb);
    }

    receivedData = "";
    dataReady = false;
    
    //delay(16000);
  }

  //delay(10);
}

void sendToThingSpeak(float temp, float press, float hum, float rain) {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    //delay(5000);
    if (WiFi.status() != WL_CONNECTED) {
      return;
    }
  }

  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=" + String(thingSpeakApiKey) +
               "&field1=" + String(temp) +
               "&field2=" + String(press) +
               "&field3=" + String(hum) +
               "&field4=" + String((int)rain);  // Cast to int for safety

  http.begin(url);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
  } else {
    Serial.println("ThingSpeak error: " + String(httpResponseCode));
  }

  http.end();
}
