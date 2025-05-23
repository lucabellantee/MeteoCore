# MeteoCore
A complete and autonomous embedded system for atmospheric data collection and rain prediction, powered by Zephyr RTOS and an integrated Machine Learning model running on the STM32-F446RE microcontroller.

## Overview

MeteoCore is an embedded device for environmental monitoring and imminent rain prediction. The system collects real-time weather data using temperature, pressure, and humidity sensors, and processes this data through a pre-trained Machine Learning model to predict the likelihood of rainfall.

### Key Features

- **Real-time monitoring** of temperature, pressure, and humidity
- **Weather forecasting** powered by Machine Learning
- **Cloud data transmission** via ThingSpeak
- **Real-time operating system** based on Zephyr RTOS
- **Distributed architecture** with UART and I2C communication
- **Formal verification** using the SPIN model checker to ensure system correctness

## System Architecture

The system consists of two microcontrollers working in synergy:
- **STM32F446RE** (with Zephyr RTOS): The core of the system for data processing and Machine Learning
- **ESP32-WROOM-32E**: Gateway for cloud connectivity
- **Sensore GY-BME280**: Collects environmental data (Temperature, Pressure, Humidity)

### Communication

- **I2C**: STM32 ↔ BME280 (STM32 as master)
- **UART**: STM32 ↔ ESP32
- **HTTP/WiFi**: ESP32 ↔ ThingSpeak

## Operation

1. **Data Collection**: Every 10 seconds, the STM32 reads values from the BME280 sensor
2. **Accumulation**: The readings are stored in internal buffers
3. **Processing**: Every 61 seconds, the average of the readings is calculated
4. **Prediction**: A decision tree processes the data to predict rainfall
5. **Transmission**: The STM32 sends the data via UART to the ESP32
6. **Upload**: The ESP32 formats the data in JSON and sends it to ThingSpeak

## Formal Verification with SPIN Model Checker

MeteoCore uses the **SPIN model checker** for the formal verification of the concurrent system behavior, ensuring the correctness of critical safety and liveness properties.
### Modellazione in Promela

## ⚙️ Architettura del sistema

```bash
MeteoCore/
├── Test main/                  # SPIN Model Checker verification files
│   ├── Dockerfile              # Docker container for SPIN environment
│   ├── spin-commands.md        # SPIN verification commands reference
│   └── test_main.pml           # Main Promela model for system verification
│   
├── thingspeak/                  # ESP32 + Arduino code for cloud connectivity
│   └── thingspeak.ino          # Arduino sketch for ThingSpeak integration
│                  
├── src/                         # STM32 source code (Zephyr RTOS)
│   ├── bme280.c                # BME280 sensor driver implementation
│   ├── bme280.h                # BME280 sensor driver header
│   ├── esp32_comm.c            # UART communication with ESP32
│   ├── esp32_comm.h            # ESP32 communication header
│   ├── main.c                  # Main application entry point
│   ├── rain_model.c            # Machine Learning decision tree implementation
│   └── rain_model.h            # Rain prediction model header
├── ml/                         # Machine Learning pipeline and training
│   ├── outputs/                # Generated models and training results
│   ├── eda.py                  # Exploratory Data Analysis script
│   ├── export_tree_to_c.py     # Export trained model to C code
│   ├── preprocess_dataset.py   # Data preprocessing and cleaning
│   ├── rebalancing.py          # Dataset balancing for training
│   └── train_model.py          # Model training and evaluation
│
├── boards/                     # Board-specific configurations
│   └── nucleo_f446re.overlay   # Device tree overlay for STM32F446RE
│ 
├── Utils/                      # Utility tools and configurations
│   ├── ESP_32_Configuration/   # ESP32 setup and configuration tools
│   │     └── ESP_32_Configuration.ino  # ESP32 initial configuration sketch
│   └── Server/                 # Local development server
│         ├── package-lock.json # Node.js dependencies lock file
│         ├── package.json      # Node.js project configuration
│         └── server.js         # Local server for testing and development
├── LICENSE                     # MIT License file
└── README.md                   # Project documentation (this file)
```

## Instructions for Use

### 1. ESP32 Connection

1. Connect the **ESP32** to your PC using a USB cable.
2. **Connect to the Wi-Fi hotspot** specified in the ESP32 program's configuration file.
3. Open **Arduino IDE**, load and open the file:
   
    ```
    thingspeak/thingspeak.ino
    ```
5. Upload the program to the **ESP32** using the Arduino IDE.



### 2. Compile and Flash STM32 (Zephyr)

1. Open the **terminal** in the main project folder.
2. Run the following commands to activate the Zephyr environment:

    ```bash
    C:\zephyrproject\.venv\Scripts\activate.bat
    ```

    ```bash
    zephyr-env.cmd
    ```
   The path to the `activate.bat` file may vary from one device to another. Additionally, before running the `zephyr-env.cmd` command, you must set the `ZEPHYR_BASE` and `ZEPHYR_SDK_INSTALL_DIR` environment 
   variables with the base path of Zephyr and the path to the SDK, respectively.


3. Build the project for the **STM32F446RE board**:

    ```bash
    west build -b nucleo_f446re stm32_weather_station
    ```

4. Flash the firmware onto the **STM32 board**:

    ```bash
    west flash
    ```



### 📡 ThingSpeak Integration

1. Make sure you have a **ThingSpeak** account with:
   - A valid **API Key**.
   - A **channel configured** with fields for the desired data (e.g., temperature, pressure, humidity, prediction).

2. Once the **ESP32** is configured and running, it will automatically send the averaged data and rain prediction every 61 seconds to the **ThingSpeak** platform.

## Prediction Model

The system uses a **pre-trained decision tree** to calculate the probability of rainfall based on:

- Average temperature
- Average humidity
- Average pressure

The model is **static** (hardcoded in the STM32 firmware) and currently **does not support dynamic updates**. However, a system for updating the model via a remote server may be integrated in the future.

---

## Authors
This project is developed and maintained by the following authors:

- **Zazzarini Micol** - [GitHub Profile](https://github.com/MicolZazzarini)
- **Bellante Luca** - [GitHub Profile](https://github.com/lucabellantee)


---

# License
This project is licensed under the [MIT License](LICENSE) 

