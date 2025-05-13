/**
 * ESP32 Weather Station - Integrazione con ThingSpeak
 * 
 * Riceve dati meteorologici da STM32 via UART, li decodifica come JSON
 * e li invia a ThingSpeak via HTTP GET.
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Configurazione WiFi
const char* ssid = WIFI_ID;           
const char* password = WIFI_PASSWORD;   

// Configurazione ThingSpeak
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
  Serial.print("Connessione al WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connesso");
  Serial.println("Indirizzo IP: " + WiFi.localIP().toString());
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
    Serial.println("Dati ricevuti: " + receivedData);

    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, receivedData);

    if (error) {
      Serial.print("deserializeJson() fallita: ");
      Serial.println(error.c_str());
    } else {
      float temperature = doc["temp"];
      float pressure = doc["press"];
      float humidity = doc["hum"];
      float rainProb = doc["rain"];

      Serial.println("Temperatura: " + String(temperature) + "°C");
      Serial.println("Pressione: " + String(pressure) + " hPa");
      Serial.println("Umidità: " + String(humidity) + "%");
      Serial.println("Pioggia prevista: " + String((int)rainProb));

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
    Serial.println("WiFi disconnesso. Riconnessione...");
    WiFi.reconnect();
    //delay(5000);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Impossibile riconnettersi al WiFi.");
      return;
    }
  }

  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=" + String(thingSpeakApiKey) +
               "&field1=" + String(temp) +
               "&field2=" + String(press) +
               "&field3=" + String(hum) +
               "&field4=" + String((int)rain);  // Cast a int per sicurezza

  Serial.println("URL: " + url);
  http.begin(url);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("ThingSpeak response: " + response);
  } else {
    Serial.println("Errore ThingSpeak: " + String(httpResponseCode));
  }

  http.end();
}
