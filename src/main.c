
/**
 * @file main.c
 * @brief Applicazione principale per il sistema di previsione pioggia
 */

 #include <zephyr/kernel.h>
 #include <zephyr/device.h>
 #include <zephyr/drivers/i2c.h>
 #include <zephyr/drivers/uart.h>
 #include <zephyr/logging/log.h>
 
 #include "bme280.h"
 #include "ml_model.h"
 #include "esp32_comm.h"
 
 LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);
 
 /* Intervallo di lettura dei dati e previsione (in millisecondi) */
 #define SAMPLING_INTERVAL_MS 10000  /* 10 secondi */
 
 /* Thread stack size */
 #define STACK_SIZE 1024
 
 /* Thread priority */
 #define THREAD_PRIORITY 7
 
 /* Buffer per il thread */
 K_THREAD_STACK_DEFINE(thread_stack, STACK_SIZE);
 static struct k_thread thread_data;
 
 /* Semaforo per sincronizzare l'avvio del thread */
 K_SEM_DEFINE(init_sem, 0, 1);
 
 /* Dispositivi */
 static const struct device *i2c_dev;
 static const struct device *uart_dev;
 
 /**
  * @brief Funzione principale del thread
  */
 void weather_thread(void *arg1, void *arg2, void *arg3)
 {
     ARG_UNUSED(arg1);
     ARG_UNUSED(arg2);
     ARG_UNUSED(arg3);
     
     bme280_data_t sensor_data;
     float rain_probability;
     
     /* Attendi che l'inizializzazione sia completata */
     k_sem_take(&init_sem, K_FOREVER);
     
     LOG_INF("Thread di monitoraggio avviato");
     
     while (1) {
         /* Leggi i dati dal sensore BME280 */
         if (bme280_read_data(i2c_dev, &sensor_data)) {
             LOG_INF("Temperatura: %.2f°C, Pressione: %.2f hPa, Umidità: %.2f%%",
                   sensor_data.temperature, sensor_data.pressure, sensor_data.humidity);
             
             /* Calcola la probabilità di pioggia */
             rain_probability = ml_model_predict_rain(&sensor_data);
             LOG_INF("Probabilità di pioggia: %.1f%%", rain_probability);
             
             /* Invia i dati all'ESP32 */
             if (!esp32_send_data(uart_dev, &sensor_data, rain_probability)) {
                 LOG_ERR("Errore nell'invio dei dati all'ESP32");
             }
         } else {
             LOG_ERR("Errore nella lettura dei dati dal sensore");
         }
         
         /* Attendi prima del prossimo campionamento */
         k_sleep(K_MSEC(SAMPLING_INTERVAL_MS));
     }
 }
 
 void main(void)
 {
     LOG_INF("Inizializzazione del sistema di previsione pioggia...");
     
     /* Ottieni il dispositivo I2C */
     i2c_dev = DEVICE_DT_GET(DT_ALIAS(i2c0));
     if (!device_is_ready(i2c_dev)) {
         LOG_ERR("Dispositivo I2C non disponibile");
         return;
     }
     
     /* Ottieni il dispositivo UART */
     uart_dev = DEVICE_DT_GET(DT_ALIAS(uart1));
     if (!device_is_ready(uart_dev)) {
         LOG_ERR("Dispositivo UART non disponibile");
         return;
     }
     
     /* Inizializza il sensore BME280 */
     if (!bme280_init(i2c_dev)) {
         LOG_ERR("Impossibile inizializzare il sensore BME280");
         return;
     }
     
     /* Inizializza il modello ML */
     ml_model_init();
     
     /* Inizializza la comunicazione con ESP32 */
     if (!esp32_comm_init(uart_dev)) {
         LOG_ERR("Impossibile inizializzare la comunicazione con ESP32");
         return;
     }
     
     /* Crea il thread per il monitoraggio */
     k_tid_t tid = k_thread_create(&thread_data, thread_stack, STACK_SIZE,
                                 weather_thread, NULL, NULL, NULL,
                                 THREAD_PRIORITY, 0, K_NO_WAIT);
     k_thread_name_set(tid, "weather_thread");
     
     /* Segnala al thread che l'inizializzazione è completata */
     k_sem_give(&init_sem);
     
     LOG_INF("Sistema inizializzato e pronto");
 }








// CODICE DI PROVA PER SIMULARE IL SENSORE

// /**
//  * @file main.c
//  * @brief Applicazione principale per il sistema di previsione pioggia (versione di test)
//  */

//  #include <zephyr/kernel.h>
//  #include <zephyr/device.h>
//  #include <zephyr/drivers/i2c.h>
//  #include <zephyr/drivers/uart.h>
//  #include <zephyr/logging/log.h>
//  #include <stdlib.h>  // Per rand()
 
//  #include "bme280.h"
//  #include "ml_model.h"
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
//  static const struct device *uart_dev;
 
//  /**
//   * @brief Genera dati fittizi per il test
//   */
//  void generate_test_data(bme280_data_t *data) {
//      // Genera valori casuali ma realistici
//      data->temperature = 20.0f + ((float)rand() / RAND_MAX) * 15.0f;  // 20-35°C
//      data->pressure = 980.0f + ((float)rand() / RAND_MAX) * 40.0f;   // 980-1020 hPa
//      data->humidity = 40.0f + ((float)rand() / RAND_MAX) * 50.0f;    // 40-90%
     
//      LOG_INF("Dati fittizi generati - T: %f°C, P: %f hPa, H: %f \n",
//              data->temperature, data->pressure, data->humidity);
//             //LOG_INF("Dati fittizi generati - T: %d.%02d°C, P: %d.%02d hPa, H: %d.%02d%%",
//               //  (int)data->temperature, (int)(data->temperature * 100) % 100,
//                 //(int)data->pressure, (int)(data->pressure * 100) % 100,
//                 //(int)data->humidity, (int)(data->humidity * 100) % 100);
//  }
 
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
     
//      LOG_INF("Thread di monitoraggio avviato (MODALITÀ TEST - dati fittizi)");
     
//      while (1) {
//          /* Genera dati fittizi per il test */
//          generate_test_data(&sensor_data);
         
//          /* Calcola la probabilità di pioggia */
//          rain_probability = ml_model_predict_rain(&sensor_data);
//          LOG_INF("Probabilità di pioggia: %f", rain_probability);
         
//          /* Invia i dati all'ESP32 */
//          if (!esp32_send_data(uart_dev, &sensor_data, rain_probability)) {
//              LOG_ERR("Errore nell'invio dei dati all'ESP32");
//          } else {
//              LOG_INF("Dati inviati all'ESP32 con successo");
//          }
         
//          /* Attendi prima del prossimo campionamento */
//          k_sleep(K_MSEC(SAMPLING_INTERVAL_MS));
//      }
//  }
 
//  void main(void)
//  {
//      //LOG_INF("Inizializzazione del sistema di previsione pioggia (MODALITÀ TEST)...");
     
//      /* Ottieni il dispositivo UART */
//      uart_dev = DEVICE_DT_GET(DT_ALIAS(uart1));
//      if (!device_is_ready(uart_dev)) {
//          LOG_ERR("Dispositivo UART non disponibile");
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
     
//      LOG_INF("Sistema inizializzato e pronto (MODALITÀ TEST)");
//  }