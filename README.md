# Weather Prediction Embedded System 🌦️

Questo progetto sviluppa un dispositivo embedded per il monitoraggio ambientale e la previsione della pioggia imminente, utilizzando i microcontrollori STM32F446RE e ESP32-WROOM-32E, insieme al sensore BME280 che misura temperatura, pressione e umidità. Il sistema è progettato per raccogliere i dati meteorologici in tempo reale e, utilizzando un modello di Machine Learning pre-addestrato, fare una previsione sulla probabilità di pioggia.

## Integrazione con Zephyr RTOS
Il cuore del sistema è basato su Zephyr RTOS, che viene eseguito sul microcontrollore STM32F446RE. Questo permette una gestione efficiente delle operazioni in tempo reale, come la lettura dei sensori, l'elaborazione dei dati e l'invio dei risultati tramite UART all'ESP32. Il modello di Machine Learning (un albero decisionale) è integrato direttamente nel firmware STM32, il quale elabora le letture meteorologiche per fare una previsione basata su dati storici.

## Comunicazione e Integrazione con ThingSpeak
L'ESP32-WROOM-32E funge da gateway per inviare i dati dal sistema embedded alla piattaforma ThingSpeak per la visualizzazione e l'analisi dei dati in tempo reale. L'ESP32 riceve i dati da STM32 tramite UART, li codifica in formato JSON e li invia via HTTP POST a ThingSpeak, dove possono essere monitorati e analizzati.

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
    thingspeak/thingspeak.ino
    ```
4. Carica il programma sull’**ESP32** tramite Arduino IDE.



### ✅ 2. Compilazione e flash STM32 (Zephyr)

1. Apri il **terminale** nella cartella principale del progetto.
2. Esegui i seguenti comandi per attivare l’ambiente Zephyr:

    ```bash
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

- [ ] Migliorare app
- [ ] Zephyr su esp32

---

## 👨‍💻 Autori



---

## 📝 Licenza

Questo progetto è distribuito sotto licenza **MIT**. Vedi il file `LICENSE` per maggiori dettagli.
