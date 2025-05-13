
// /**
//  * @file main.c
//  * @brief Applicazione principale per il sistema di previsione pioggia
//  */

//  #include <zephyr/kernel.h>
//  #include <zephyr/device.h>
//  #include <zephyr/drivers/i2c.h>
//  #include <zephyr/drivers/uart.h>
//  #include <zephyr/logging/log.h>
//  #include "bme280.h"
//  #include "rain_model.h"
//  #include "esp32_comm.h"
 
//  LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);
 
//  /* Intervallo di lettura dei dati e previsione (in millisecondi) */
//  #define SAMPLING_INTERVAL_MS 10000  /* 10 secondi */
 
//  /* Thread stack size */
//  #define STACK_SIZE 1024
 
//  /* Thread priority */
//  #define THREAD_PRIORITY 7
 
//  /* Buffer per il thread */
//  K_THREAD_STACK_DEFINE(thread_stack, STACK_SIZE);
//  static struct k_thread thread_data;
 
//  /* Semaforo per sincronizzare l'avvio del thread */
//  K_SEM_DEFINE(init_sem, 0, 1);
 
//  /* Dispositivi */
//  static const struct device *i2c_dev;
//  static const struct device *uart_dev;
 
//  /**
//   * @brief Funzione principale del thread
//   */
//  void weather_thread(void *arg1, void *arg2, void *arg3)
//  {
//      ARG_UNUSED(arg1);
//      ARG_UNUSED(arg2);
//      ARG_UNUSED(arg3);
     
//      bme280_data_t sensor_data;
//      float rain_probability;
     
//      /* Attendi che l'inizializzazione sia completata */
//      k_sem_take(&init_sem, K_FOREVER);
     
//      LOG_INF("Thread di monitoraggio avviato");
     
//      while (1) {
//          /* Leggi i dati dal sensore BME280 */
//          if (bme280_read_data(i2c_dev, &sensor_data)) {
//              LOG_INF("Temperatura: %.2f°C, Pressione: %.2f hPa, Umidità: %.2f%%",
//                    sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
             
//              /* Calcola la probabilità di pioggia */
//              rain_probability = (float)predict_rain(&sensor_data);
//              LOG_INF("Probabilità di pioggia: %.1f%%", rain_probability);
             
//              /* Invia i dati all'ESP32 */
//              if (!esp32_send_data(uart_dev, &sensor_data, rain_probability)) {
//                  LOG_ERR("Errore nell'invio dei dati all'ESP32");
//              }
//          } else {
//              LOG_ERR("Errore nella lettura dei dati dal sensore");
//          }
         
//          /* Attendi prima del prossimo campionamento */
//          k_sleep(K_MSEC(SAMPLING_INTERVAL_MS));
//      }
//  }
 
//  void main(void)
//  {
//      LOG_INF("Inizializzazione del sistema di previsione pioggia...");
     
     
//      /* Ottieni il dispositivo I2C */
//      i2c_dev = DEVICE_DT_GET(DT_ALIAS(i2c1));
//      if (!device_is_ready(i2c_dev)) {
//          LOG_ERR("Dispositivo I2C non disponibile");
//          return;
//      }
     
//      /* Ottieni il dispositivo UART */
//      uart_dev = DEVICE_DT_GET(DT_ALIAS(uart1));
//      if (!device_is_ready(uart_dev)) {
//          LOG_ERR("Dispositivo UART non disponibile");
//          return;
//      }
     
//      /* Inizializza il sensore BME280 */
//      if (!bme280_init(i2c_dev)) {
//          LOG_ERR("Impossibile inizializzare il sensore BME280");
//          return;
//      }
     
//      /* Inizializza il modello ML */
//      ml_model_init();
     
//      /* Inizializza la comunicazione con ESP32 */
//      if (!esp32_comm_init(uart_dev)) {
//          LOG_ERR("Impossibile inizializzare la comunicazione con ESP32");
//          return;
//      }
     
//      /* Crea il thread per il monitoraggio */
//      k_tid_t tid = k_thread_create(&thread_data, thread_stack, STACK_SIZE,
//                                  weather_thread, NULL, NULL, NULL,
//                                  THREAD_PRIORITY, 0, K_NO_WAIT);
//      k_thread_name_set(tid, "weather_thread");
     
//      /* Segnala al thread che l'inizializzazione è completata */
//      k_sem_give(&init_sem);
     
//      LOG_INF("Sistema inizializzato e pronto");
//  }














// // //CODICE DI PROVA PER SIMULARE IL SENSORE

// // /**
// //  * @file main.c
// //  * @brief Applicazione principale per il sistema di previsione pioggia (versione di test)
// //  */

// //  #include <zephyr/kernel.h>
// //  #include <zephyr/device.h>
// //  #include <zephyr/drivers/i2c.h>
// //  #include <zephyr/drivers/uart.h>
// //  #include <zephyr/logging/log.h>
// //  #include <stdlib.h>  // Per rand()
 
// //  #include "bme280.h"
// //  #include "rain_model.h"
// //  #include "esp32_comm.h"
 
// //  LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);
 
// //  /* Intervallo di lettura dei dati e previsione (in millisecondi) */
// //  #define SAMPLING_INTERVAL_MS 10000  /* 10 secondi */
 
// //  /* Thread stack size */
// //  #define STACK_SIZE 1024
 
// //  /* Thread priority */
// //  #define THREAD_PRIORITY 7
 
// //  /* Buffer per il thread */
// //  K_THREAD_STACK_DEFINE(thread_stack, STACK_SIZE);
// //  static struct k_thread thread_data;
 
// //  /* Semaforo per sincronizzare l'avvio del thread */
// //  K_SEM_DEFINE(init_sem, 0, 1);
 
// //  /* Dispositivi */
// //  static const struct device *uart_dev;
 
// //  /**
// //   * @brief Genera dati fittizi per il test
// //   */
// //  void generate_test_data(bme280_data_t *data) {
// //      // Genera valori casuali ma realistici
// //      data->temperature = 20.0f + ((float)rand() / RAND_MAX) * 15.0f;  // 20-35°C
// //      data->pressure = 980.0f + ((float)rand() / RAND_MAX) * 40.0f;   // 980-1020 hPa
// //      data->humidity = 40.0f + ((float)rand() / RAND_MAX) * 50.0f;    // 40-90%
     
// //      LOG_INF("Dati fittizi generati - T: %f°C, P: %f hPa, H: %f \n",
// //              data->temperature, data->pressure, data->humidity);
// //             //LOG_INF("Dati fittizi generati - T: %d.%02d°C, P: %d.%02d hPa, H: %d.%02d%%",
// //               //  (int)data->temperature, (int)(data->temperature * 100) % 100,
// //                 //(int)data->pressure, (int)(data->pressure * 100) % 100,
// //                 //(int)data->humidity, (int)(data->humidity * 100) % 100);
// //  }
 
// //  /**
// //   * @brief Funzione principale del thread
// //   */
// //  void weather_thread(void *arg1, void *arg2, void *arg3)
// //  {
// //      ARG_UNUSED(arg1);
// //      ARG_UNUSED(arg2);
// //      ARG_UNUSED(arg3);
     
// //      bme280_data_t sensor_data;
// //      float rain_probability;
     
// //      /* Attendi che l'inizializzazione sia completata */
// //      k_sem_take(&init_sem, K_FOREVER);
     
// //      LOG_INF("Thread di monitoraggio avviato (MODALITÀ TEST - dati fittizi)");
     
// //      while (1) {
// //          /* Genera dati fittizi per il test */
// //          generate_test_data(&sensor_data);
         
// //          /* Calcola la probabilità di pioggia */
// //          rain_probability = (float)predict_rain(&sensor_data);
// //          LOG_INF("Probabilità di pioggia: %f", rain_probability);
         
// //          /* Invia i dati all'ESP32 */
// //          if (!esp32_send_data(uart_dev, &sensor_data, rain_probability)) {
// //              LOG_ERR("Errore nell'invio dei dati all'ESP32");
// //          } else {
// //              LOG_INF("Dati inviati all'ESP32 con successo");
// //          }
         
// //          /* Attendi prima del prossimo campionamento */
// //          k_sleep(K_MSEC(SAMPLING_INTERVAL_MS));
// //      }
// //  }
 
// //  void main(void)
// //  {
// //      //LOG_INF("Inizializzazione del sistema di previsione pioggia (MODALITÀ TEST)...");
     
// //      /* Ottieni il dispositivo UART */
// //      uart_dev = DEVICE_DT_GET(DT_ALIAS(uart1));
// //      if (!device_is_ready(uart_dev)) {
// //          LOG_ERR("Dispositivo UART non disponibile");
// //          return;
// //      }
     
// //      /* Inizializza il modello ML */
// //      ml_model_init();
     
// //      /* Inizializza la comunicazione con ESP32 */
// //      if (!esp32_comm_init(uart_dev)) {
// //          LOG_ERR("Impossibile inizializzare la comunicazione con ESP32");
// //          return;
// //      }
     
// //      /* Crea il thread per il monitoraggio */
// //      k_tid_t tid = k_thread_create(&thread_data, thread_stack, STACK_SIZE,
// //                                  weather_thread, NULL, NULL, NULL,
// //                                  THREAD_PRIORITY, 0, K_NO_WAIT);
// //      k_thread_name_set(tid, "weather_thread");
     
// //      /* Segnala al thread che l'inizializzazione è completata */
// //      k_sem_give(&init_sem);
     
// //      LOG_INF("Sistema inizializzato e pronto (MODALITÀ TEST)");
// //  }

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <stdio.h>
#include "bme280.h"
#include "rain_model.h"
#include "esp32_comm.h"

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

#define STACK_SIZE 2048
#define THREAD_PRIORITY 5
#define SAMPLING_INTERVAL_MS 10000 // 10 SECONDS
#define PREDICTION_INTERVAL_MS 65000 // 1 MINUTE AND 5 SECONDS
#define MAX_SAMPLES 30  // 5 minutes if SAMPLING_INTERVAL_MS = 10s

// I2C sensor - uses DT_ALIAS as in the original code
static const struct device *i2c_dev = DEVICE_DT_GET(DT_ALIAS(i2c1));

// UART ESP32 - keep uart1 as in the original if you're using the same device
static const struct device *uart_dev = DEVICE_DT_GET(DT_ALIAS(uart1));

// Semaphore for synchronization
K_SEM_DEFINE(init_sem, 0, 2);

// Structure buffer
typedef struct {
    float temperature[MAX_SAMPLES];
    float pressure[MAX_SAMPLES];
    float humidity[MAX_SAMPLES];
    int count;
} sensor_buffer_t;

static sensor_buffer_t sensor_buffer;
K_MUTEX_DEFINE(buffer_mutex);

// Thread stack & data
K_THREAD_STACK_DEFINE(thread_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(prediction_thread_stack, STACK_SIZE);
static struct k_thread thread_data;
static struct k_thread prediction_thread_data;

/**
 * @brief Data acquisition thread
 *
 * This thread reads data from the BME280 sensor at a regular interval defined by 
 * SAMPLING_INTERVAL_MS (10 seconds) and stores the values (temperature, pressure, humidity) 
 * in a buffer. The buffer holds a maximum of MAX_SAMPLES (30 samples). 
 * It ensures thread synchronization using a mutex to avoid race conditions.
 */
void data_acquisition_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);
    
    bme280_data_t sensor_data;

    k_sem_take(&init_sem, K_FOREVER);
    LOG_INF("Reading thread started");

    while (1) {
        if (bme280_read_data(i2c_dev, &sensor_data)) {
            LOG_INF("T: %.2fC, P: %.2f hPa, H: %.2f%%",
                    sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);

            k_mutex_lock(&buffer_mutex, K_FOREVER);
            if (sensor_buffer.count < MAX_SAMPLES) {
                int i = sensor_buffer.count;
                sensor_buffer.temperature[i] = sensor_data.temperature;
                sensor_buffer.pressure[i] = sensor_data.pressure;
                sensor_buffer.humidity[i] = sensor_data.humidity;
                sensor_buffer.count++;
            }
            k_mutex_unlock(&buffer_mutex);
        } else {
            LOG_ERR("Sensor read error");
        }

        k_sleep(K_MSEC(SAMPLING_INTERVAL_MS));
    }
}

/**
 * @brief Prediction thread
 *
 * This thread is responsible for calculating the average values of the sensor data 
 * stored in the buffer and performing the rain prediction using a machine learning model. 
 * After calculating the average values for temperature, pressure, and humidity, it uses 
 * the `predict_rain` function to predict the probability of rain. 
 * The result is then sent to the ESP32 via UART. 
 * The prediction occurs at an interval defined by PREDICTION_INTERVAL_MS (1 minute and 5 seconds).
 */
void prediction_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);
    
    bme280_data_t avg_data;
    float rain_prob;

    k_sem_take(&init_sem, K_FOREVER);
    LOG_INF("Prediction thread started");

    while (1) {
        k_mutex_lock(&buffer_mutex, K_FOREVER);
        if (sensor_buffer.count > 0) {
            float sum_t = 0, sum_p = 0, sum_h = 0;
            for (int i = 0; i < sensor_buffer.count; i++) {
                sum_t += sensor_buffer.temperature[i];
                sum_p += sensor_buffer.pressure[i];
                sum_h += sensor_buffer.humidity[i];
            }
            avg_data.temperature = sum_t / sensor_buffer.count;
            avg_data.pressure = sum_p / sensor_buffer.count;
            avg_data.humidity = sum_h / sensor_buffer.count;
            sensor_buffer.count = 0;  // reset
        } else {
            k_mutex_unlock(&buffer_mutex);
            k_sleep(K_MSEC(PREDICTION_INTERVAL_MS));
            continue;
        }
        k_mutex_unlock(&buffer_mutex);

        rain_prob = (float)predict_rain(&avg_data);
        LOG_INF("Average -> T: %.2fC, P: %.2f hPa, H: %.2f%%", avg_data.temperature, avg_data.pressure, avg_data.humidity);
        LOG_INF("Rain prediction: %.1f%%", rain_prob);

        if (!esp32_send_data(uart_dev, &avg_data, rain_prob)) {
            LOG_ERR("Error sending data to ESP32");
        }

        k_sleep(K_MSEC(PREDICTION_INTERVAL_MS));
    }
}

/**
 * @brief Main function
 *
 * Initializes the I2C, UART, and sensor devices. It also initializes the ML model 
 * for rain prediction and starts the data acquisition and prediction threads.
 * It ensures that all devices and threads are properly initialized and ready.
 */
void main(void)
{
    if (!device_is_ready(i2c_dev)) {
        LOG_ERR("I2C not ready");
        return;
    }

    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART not ready");
        return;
    }

    if (!bme280_init(i2c_dev)) {
        LOG_ERR("BME280 initialization error");
        return;
    }
    
    // Initialize ML model
    ml_model_init();
    
    // Initialize communication with ESP32
    if (!esp32_comm_init(uart_dev)) {
        LOG_ERR("Unable to initialize communication with ESP32");
        return;
    }

    LOG_INF("Devices initialized");
    k_sem_give(&init_sem);
    k_sem_give(&init_sem);

    // Create data acquisition thread
    k_thread_create(&thread_data, thread_stack, STACK_SIZE,
                    data_acquisition_thread, NULL, NULL, NULL,
                    THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&thread_data, "data_acquisition_thread");

    // Create prediction thread
    k_thread_create(&prediction_thread_data, prediction_thread_stack, STACK_SIZE,
                    prediction_thread, NULL, NULL, NULL,
                    THREAD_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&prediction_thread_data, "prediction_thread");
}
