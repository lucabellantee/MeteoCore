/**
 * @file esp32_comm.h
 * @brief Communication with the ESP32 module
 */

 #ifndef ESP32_COMM_H
 #define ESP32_COMM_H
 
 #include <zephyr/device.h>
 #include <zephyr/drivers/uart.h>
 #include "bme280.h"
 
 /**
  * @brief Initializes UART communication with the ESP32
  *
  * @param uart_dev Pointer to the UART device
  * @return true if initialization was successful, false otherwise
  */
 bool esp32_comm_init(const struct device *uart_dev);
 
 /**
  * @brief Sends data to the ESP32
  *
  * @param uart_dev Pointer to the UART device
  * @param sensor_data BME280 sensor data
  * @param rain_probability Rain probability
  * @return true if sending was successful, false otherwise
  */
 bool esp32_send_data(const struct device *uart_dev, 
                     const bme280_data_t *sensor_data, 
                     float rain_probability);
 
 #endif /* ESP32_COMM_H */
