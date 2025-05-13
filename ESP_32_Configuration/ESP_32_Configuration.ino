/**
 * ESP32 Weather Station - Receives data from STM32 and sends it to the database
 * 
 * This sketch receives weather data via UART from STM32, parses it as JSON, prints it to the serial monitor, 
 * and sends it via HTTP POST to a remote server (database) over WiFi.
 * 
 */

 #include <WiFi.h>
 #include <HTTPClient.h>
 #include <ArduinoJson.h>
 #include <secrets.h>
 
 // WiFi Configuration
 const char* ssid = WIFI_ID;      
 const char* password = WIFI_PASSWORD;   
 
 // Server Configuration
 const char* serverUrl = SERVER_URL;  
 
 // Configuration of the physical pins that the ESP32 will use to communicate via UART (serial) with the STM32 board.
 #define RXD2 16  // ESP32 RX pin connected to STM32 TX pin
 #define TXD2 17  // ESP32 TX pin connected to STM32 RX pin
 
 // Buffer for received data
 String receivedData = "";
 bool dataReady = false;
 
 void setup() {
   // Initialize Serial for debugging
   Serial.begin(115200);
   Serial.println("ESP32 Weather Station");
   
   // Initialize Serial 2 for communication with STM32
   Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
   
   // Connects the ESP32 to the WiFi network using the specified SSID and password.
   WiFi.begin(ssid, password); 
   Serial.print("WiFi connection");
   
   // Wait for connection
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   
   Serial.println("");
   Serial.println("WiFi connected");
   Serial.println("IP address: " + WiFi.localIP().toString());
 } 
 
 void loop() {
   // Read data from the serial port
   Serial.println(Serial2.available());
   while (Serial2.available()) {
     char c = Serial2.read();
     receivedData += c;
     if (c == '\n') {
       dataReady = true;
       break;
     }
   }

   // Process the received data
   if (dataReady) {
     Serial.println("Received data: " + receivedData);
     
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
       
       Serial.println("Temperature: " + String(temperature) + "°C");
       Serial.println("Pressure: " + String(pressure) + " hPa");
       Serial.println("Humidity: " + String(humidity) + "%");
       Serial.println("Rain Probability: " + String(rainProb) + "%");
       
       // Send data to server
       sendToDatabase(temperature, pressure, humidity, rainProb);
     }
     
     // Reset 
     receivedData = "";
     dataReady = false;
   }
   
   //delay();
 }
 
 /**
  * Send data to database through HTTP POST
  */
 void sendToDatabase(float temp, float press, float hum, float rain) {
   // Verify wifi connection
   if (WiFi.status() != WL_CONNECTED) {
     Serial.println("// WiFi disconnected. Reconnecting...");
     WiFi.reconnect();
     delay(5000); 
     
     if (WiFi.status() != WL_CONNECTED) {
       Serial.println("// Unable to reconnect to WiFi.");
       return;
     }
   }
   
   // Prepare json data
   DynamicJsonDocument jsonDoc(256);
   jsonDoc["temperature"] = temp;
   jsonDoc["pressure"] = press;
   jsonDoc["humidity"] = hum;
   jsonDoc["rain_probability"] = rain;
   jsonDoc["timestamp"] = millis(); 
   
   String jsonString;
   serializeJson(jsonDoc, jsonString);
   
   // Create HTTP connection
   HTTPClient http;
   http.begin(serverUrl);
   http.addHeader("Content-Type", "application/json");
   
   // Send POST request
   int httpResponseCode = http.POST(jsonString);
   
   if (httpResponseCode > 0) {
     String response = http.getString();
     Serial.println("HTTP Response code: " + String(httpResponseCode));
     Serial.println("Response: " + response);
   } else {
     Serial.println("HTTP request error: " + String(httpResponseCode));
   }
   
   // Close connection
   http.end();
 }