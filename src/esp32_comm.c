/**
 * @file esp32_comm.c
 * @brief Implementazione della comunicazione con il modulo ESP32
 */

 #include "esp32_comm.h"
 #include <zephyr/logging/log.h>
 #include <string.h>
 #include <stdio.h>
 
 LOG_MODULE_REGISTER(esp32_comm, CONFIG_LOG_DEFAULT_LEVEL);
 
 /* Dimensione del buffer per la comunicazione */
 #define UART_BUF_SIZE 128
 
 /* Buffer per l'invio dei dati */
 static char tx_buf[UART_BUF_SIZE];
 
 bool esp32_comm_init(const struct device *uart_dev)
 {
     if (!device_is_ready(uart_dev)) {
         LOG_ERR("Dispositivo UART non pronto");
         return false;
     }
     
     /* Configura UART se necessario */
     /* Nota: in Zephyr, la configurazione di base è generalmente gestita
        dalla configurazione del dispositivo nel Device Tree */
     
     LOG_INF("Comunicazione UART con ESP32 inizializzata");
     return true;
 }
 
 bool esp32_send_data(const struct device *uart_dev, 
                     const bme280_data_t *sensor_data, 
                     float rain_probability)
 {
     int len;
     
     /* Formatta i dati in formato JSON per facilità di parsing su ESP32 */
     len = snprintf(tx_buf, sizeof(tx_buf),
                  "{\"temp\":%.2f,\"press\":%.2f,\"hum\":%.2f,\"rain\":%.2f}\n",
                  sensor_data->temperature,
                  sensor_data->pressure,
                  sensor_data->humidity,
                  rain_probability);
     
     if (len < 0 || len >= sizeof(tx_buf)) {
         LOG_ERR("Errore nella formattazione dei dati");
         return false;
     }
     
     /* Invia i dati attraverso UART */
     for (int i = 0; i < len; i++) {
         uart_poll_out(uart_dev, tx_buf[i]);
     }
     
     LOG_DBG("Dati inviati a ESP32: %s", tx_buf);
     return true;
 }