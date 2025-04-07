/**
 * @file esp32_comm.h
 * @brief Comunicazione con il modulo ESP32
 */

 #ifndef ESP32_COMM_H
 #define ESP32_COMM_H
 
 #include <zephyr/device.h>
 #include <zephyr/drivers/uart.h>
 #include "bme280.h"
 
 /**
  * @brief Inizializza la comunicazione UART con ESP32
  *
  * @param uart_dev Puntatore al dispositivo UART
  * @return true se l'inizializzazione è riuscita, false altrimenti
  */
 bool esp32_comm_init(const struct device *uart_dev);
 
 /**
  * @brief Invia i dati all'ESP32
  *
  * @param uart_dev Puntatore al dispositivo UART
  * @param sensor_data Dati del sensore BME280
  * @param rain_probability Probabilità di pioggia
  * @return true se l'invio è riuscito, false altrimenti
  */
 bool esp32_send_data(const struct device *uart_dev, 
                     const bme280_data_t *sensor_data, 
                     float rain_probability);
 
 #endif /* ESP32_COMM_H */