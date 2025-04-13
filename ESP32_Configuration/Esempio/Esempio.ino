#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configurazione WiFi
const char* ssid = "luca";       // Sostituisci con il nome della tua rete WiFi
const char* password = "luca2303";   // Sostituisci con la password della tua rete WiFi

// Configurazione del server
const char* serverUrl = "http://192.168.5.5:3000/api/data";  // Sostituisci con l'URL del tuo server

// Configurazione UART
#define RXD2 16  // Pin RX dell'ESP32 connesso al TX dell'STM32
#define TXD2 17  // Pin TX dell'ESP32 connesso al RX dell'STM32

// Variabili di tempo per inviare i dati ogni 3 secondi
unsigned long previousMillis = 0;
const long interval = 3000;  // 3 secondi

void setup() {
  // Inizializza Serial per il debug
  Serial.begin(115200);
  Serial.println("ESP32 Weather Station");

  // Connessione WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connessione al WiFi");

  // Attendi la connessione
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connesso");
  Serial.println("Indirizzo IP: " + WiFi.localIP().toString());

  // Inizializza Serial2 per la comunicazione con STM32
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  // Calcola il tempo trascorso
  unsigned long currentMillis = millis();

  // Se sono passati 3 secondi, invia i dati
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Simulazione dei dati ricevuti (in un'applicazione reale, questi valori dovrebbero provenire dall'STM32 via UART)
    float temperature = 22.5;  // Simulato
    float pressure = 1013.25;  // Simulato
    float humidity = 60.0;     // Simulato
    float rainProb = 30.0;     // Simulato

    Serial.println("Dati simulati ricevuti:");
    Serial.println("Temperatura: " + String(temperature) + "°C");
    Serial.println("Pressione: " + String(pressure) + " hPa");
    Serial.println("Umidità: " + String(humidity) + "%");
    Serial.println("Probabilità pioggia: " + String(rainProb) + "%");

    // Invia i dati al server
    sendToDatabase(temperature, pressure, humidity, rainProb);
  }

  delay(10);  // Piccolo delay per evitare un uso eccessivo della CPU
}

/**
 * Invia i dati al database tramite HTTP POST
 */
void sendToDatabase(float temp, float press, float hum, float rain) {
  // Verifica che la connessione WiFi sia attiva
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnesso. Riconnessione...");
    WiFi.reconnect();
    delay(5000); // Attendi la riconnessione

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Impossibile riconnettersi al WiFi.");
      return;
    }
  }

  // Prepara i dati JSON per l'invio
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["temperature"] = temp;
  jsonDoc["pressure"] = press;
  jsonDoc["humidity"] = hum;
  jsonDoc["rain_probability"] = rain;
  jsonDoc["timestamp"] = millis(); // In un'applicazione reale, usare un timestamp reale

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Crea una connessione HTTP
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  // Invia la richiesta POST
  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    Serial.println("Response: " + response);
  } else {
    Serial.println("Errore nella richiesta HTTP: " + String(httpResponseCode));
  }

  // Chiudi la connessione
  http.end();
}
