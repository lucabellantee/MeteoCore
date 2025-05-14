# Weather Prediction Embedded System 🌦️

Questo progetto consiste in un **dispositivo embedded per il monitoraggio ambientale** e la **previsione della pioggia imminente**. È basato su due microcontrollori — uno STM32F446RE e un ESP32-WROOM-32E — e utilizza un sensore ambientale BME280. Il sistema è progettato per operare in modo autonomo e inviare periodicamente dati meteorologici e predizioni a una piattaforma cloud.

---

## ⚙️ Architettura del sistema

### Componenti hardware:

- **STM32F446RE** (con Zephyr RTOS)
- **ESP32-WROOM-32E**
- **Sensore GY-BME280** (Temperatura, Pressione, Umidità)
- Comunicazione:
  - **I2C** tra STM32 e BME280 (STM32 è master, sensore è slave)
  - **UART** tra STM32 e ESP32

### Funzionamento:

1. **Ogni 10 secondi**, l’STM32 legge i valori di temperatura, umidità e pressione dal sensore BME280.
2. I valori vengono **accumulati in buffer** interni.
3. **Ogni 61 secondi**, viene calcolata la media delle letture.
4. Questi dati medi vengono elaborati da un **modello di Machine Learning (albero decisionale)** per prevedere se è probabile che piova ("Rain Tomorrow").
5. STM32 invia i dati via UART all’ESP32.
6. L’ESP32 confeziona i dati in formato JSON e li invia tramite **HTTP POST** alla piattaforma **ThingSpeak** (Matlab API).

---

## 📂 Struttura del repository

## 🛠️ Istruzioni per l'uso

### ✅ 1. Collegamento ESP32

1. Collega l’**ESP32** al PC tramite un cavo USB.
2. **Connettiti alla rete Wi-Fi hotspot** specificata nel file di configurazione del programma ESP32.
3. Apri **Arduino IDE**, carica e apri il file:

    ```
    esp32_weather_station/ESP_32_Configuration.ino
    ```

4. All’interno del file `.ino`, **modifica l’indirizzo IP** con quello del tuo hotspot/router o del server ThingSpeak.
5. Carica il programma sull’**ESP32** tramite Arduino IDE.



### ✅ 2. Compilazione e flash STM32 (Zephyr)

1. Apri il **terminale** in **Visual Studio Code** nella cartella principale del progetto.
2. Esegui i seguenti comandi per attivare l’ambiente Zephyr:

    ```powershell
    C:\zephyrproject\.venv\Scripts\activate.bat
    ```

    ```bash
    zephyr-env.cmd
    ```

3. Compila il progetto per la board **STM32F446RE**:

    ```bash
    west build -b nucleo_f446re stm32_weather_station
    ```

4. Flasha il firmware sulla scheda **STM32**:

    ```bash
    west flash
    ```



### 📡 Integrazione ThingSpeak

1. Assicurati di avere un **account su ThingSpeak** con:
   - **API Key** corretta.
   - **Canale configurato** con i campi per i dati desiderati (ad esempio temperatura, pressione, umidità, previsione).

2. Una volta che l’**ESP32** è configurato e in esecuzione, invierà automaticamente i **dati mediati** e la **previsione di pioggia** ogni 61 secondi alla piattaforma **ThingSpeak**.


## 🤖 Modello di previsione

Il sistema utilizza un **albero decisionale pre-addestrato** (decision tree) per calcolare la probabilità di pioggia basandosi su:

- Temperatura media
- Umidità media
- Pressione media

Il modello è **statico** (codificato nel firmware STM32) e al momento **non prevede aggiornamenti dinamici**. Tuttavia, in futuro potrebbe essere integrato un sistema per aggiornare il modello tramite un server remoto.

---

## ✅ TODO (sviluppi futuri)

- [ ] **Logging locale** dei dati su SD o memoria flash.
- [ ] **Aggiornamento dinamico** del modello ML via rete.
- [ ] **Interfaccia web** per visualizzazione in tempo reale dei dati.
- [ ] **Notifiche push** in caso di pioggia imminente.

---

## 👨‍💻 Autori

Progetto sviluppato nell’ambito del corso di **Sistemi Embedded** – Ingegneria Informatica, Università di [Inserisci nome].

---

## 📝 Licenza

Questo progetto è distribuito sotto licenza **MIT**. Vedi il file `LICENSE` per maggiori dettagli.
