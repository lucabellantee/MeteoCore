/**
 * @file bme280.h
 * @brief Driver for the BME280 sensor
 */

 #ifndef BME280_H
 #define BME280_H
 
 #include <zephyr/device.h>
 #include <zephyr/drivers/i2c.h>
 #include <stdbool.h>
 
 /** Default I2C address of the BME280 */
 // #define BME280_I2C_ADDR 0x76 // Must match the address in the overlay file
 
 /** Structure for sensor data */
 typedef struct {
     float temperature;   /**< Temperature in °C */
     float pressure;      /**< Pressure in hPa */
     float humidity;      /**< Relative humidity in % */
 } bme280_data_t;
 
 /**
  * @brief Initializes the BME280 sensor
  *
  * @param i2c_dev Pointer to the I2C device
  * @return true if initialization was successful, false otherwise
  */
 bool bme280_init(const struct device *i2c_dev);
 
 /**
  * @brief Reads data from the BME280 sensor
  *
  * @param i2c_dev Pointer to the I2C device
  * @param data Pointer to the structure where data will be saved
  * @return true if reading was successful, false otherwise
  */
 bool bme280_read_data(const struct device *i2c_dev, bme280_data_t *data);
 
 #endif /* BME280_H */
