/**
 * ESP32 Weather Station - Ricevitore dati da STM32 e invio al database
 * 
 * Questo sketch riceve dati meteorologici via UART dall'STM32, li parsa come JSON, li stampa in seriale e 
 * e li invia tramite HTTP POST  a un server remoto (database) tramite la connessione WiFi.
 * 
 */

 #include <WiFi.h>
 #include <HTTPClient.h>
 #include <ArduinoJson.h>
 #include <secrets.h>
 
 // Configurazione WiFi
 const char* ssid = WIFI_ID;      
 const char* password = WIFI_PASSWORD;   
 
 // Configurazione del server
 const char* serverUrl = SERVER_URL;  
 
 // Configurazione dei pin fisici che l'ESP32 userà per comunicare via UART (seriale) con la scheda STM32.
 #define RXD2 16  // Pin RX dell'ESP32 connesso al TX dell'STM32
 #define TXD2 17  // Pin TX dell'ESP32 connesso al RX dell'STM32
 
 // Buffer per i dati ricevuti
 String receivedData = "";
 bool dataReady = false;
 
 void setup() {
   // Inizializza Serial per il debug
   Serial.begin(115200);
   Serial.println("ESP32 Weather Station");
   
   // Inizializza Serial2 per la comunicazione con STM32
   Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
   
   // Connette l’ESP32 alla rete WiFi usando SSID e password specificati.
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
 } 
 
 void loop() {
   // Leggi i dati dalla porta seriale
   Serial.println(Serial2.available());
   while (Serial2.available()) {
     char c = Serial2.read();
     //Serial.println(c);  // Dovresti aggiungere una riga per stampare i dati ricevuti
     receivedData += c;
     //Serial.println("Carattere/i:   " + c); // Messaggio per fare DEBUG
     // Se troviamo un newline, i dati sono pronti per essere elaborati
     if (c == '\n') {
       dataReady = true;
       break;
     }
   }

   // Elabora i dati ricevuti
   if (dataReady) {
     Serial.println("Dati ricevuti: " + receivedData);
     
     // Parse JSON
     DynamicJsonDocument doc(256);
     DeserializationError error = deserializeJson(doc, receivedData);
     
     if (error) {
       Serial.print("deserializeJson() failed: ");
       Serial.println(error.c_str());
     } else {
       // Estrai valori
       float temperature = doc["temp"];
       float pressure = doc["press"];
       float humidity = doc["hum"];
       float rainProb = doc["rain"];
       
       Serial.println("Temperatura: " + String(temperature) + "°C");
       Serial.println("Pressione: " + String(pressure) + " hPa");
       Serial.println("Umidità: " + String(humidity) + "%");
       Serial.println("Probabilità pioggia: " + String(rainProb) + "%");
       
       // Invia i dati al server
       sendToDatabase(temperature, pressure, humidity, rainProb);
     }
     
     // Reset per il prossimo messaggio
     receivedData = "";
     dataReady = false;
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