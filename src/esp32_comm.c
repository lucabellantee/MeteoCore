/**
 * @file esp32_comm.c
 * @brief Implementation of communication with the ESP32 module
 */

 #include "esp32_comm.h"
 #include <zephyr/logging/log.h>
 #include <string.h>
 #include <stdio.h>
 
 LOG_MODULE_REGISTER(esp32_comm, CONFIG_LOG_DEFAULT_LEVEL);
 
 /* Buffer size for communication */
 #define UART_BUF_SIZE 128
 
 /* Buffer for sending data */
 static char tx_buf[UART_BUF_SIZE];
 
 bool esp32_comm_init(const struct device *uart_dev)
 {
     if (!device_is_ready(uart_dev)) {
         LOG_ERR("UART device not ready");
         return false;
     }
     
     /* Configures UART if necessary */
     /* Note: in Zephyr, the basic configuration is usually managed
        by the device configuration in the Device Tree */
     
     LOG_INF("UART communication with ESP32 initialized");
     return true;
 }
 
 bool esp32_send_data(const struct device *uart_dev, 
                     const bme280_data_t *sensor_data, 
                     float rain_probability)
 {
     int len;
     
     /* Formats the data in JSON format for easy parsing on ESP32 */
     len = snprintf(tx_buf, sizeof(tx_buf),
                "{\"temp\":%.2f,\"press\":%.2f,\"hum\":%.2f,\"rain\":%.2f}\n",
                (double)sensor_data->temperature,
                (double)sensor_data->pressure,
                (double)sensor_data->humidity,
                (double)rain_probability);
     
     if (len < 0 || len >= sizeof(tx_buf)) {
         LOG_ERR("Error in data formatting");
         return false;
     }
    //LOG_INF("Complete string to send: %s", tx_buf);
    //LOG_INF("Formatted message length: %d", len);
    //LOG_INF("Last character is: 0x%02X", tx_buf[len-1]);

     
    //LOG_INF("Now entering the for loop to send data\n");
     /* Sends data via UART */
     for (int i = 0; i < len; i++) {
         //LOG_INF("Character: %c\n", tx_buf[i]);
         uart_poll_out(uart_dev, tx_buf[i]);
         //uart_poll_out(uart_dev, 'a');
         //LOG_INF("Data sent!\n");
         //k_sleep(K_MSEC(1));  // Adds a 1ms delay between characters
     }
     
     LOG_DBG("Data sent to ESP32: %s", tx_buf);
     return true;
 }
