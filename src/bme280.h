/**
 * @file bme280.h
 * @brief Driver per il sensore BME280
 */

 #ifndef BME280_H
 #define BME280_H
 
 #include <zephyr/device.h>
 #include <zephyr/drivers/i2c.h>
 #include <stdbool.h>
 
 /** Indirizzo I2C predefinito del BME280 */
 #define BME280_I2C_ADDR 0x76  // Indirizzo alternativo: 0x77
 
 /** Struttura per i dati del sensore */
 typedef struct {
     float temperature;   /**< Temperatura in °C */
     float pressure;      /**< Pressione in hPa */
     float humidity;      /**< Umidità relativa in % */
 } bme280_data_t;
 
 /**
  * @brief Inizializza il sensore BME280
  *
  * @param i2c_dev Puntatore al dispositivo I2C
  * @return true se l'inizializzazione è riuscita, false altrimenti
  */
 bool bme280_init(const struct device *i2c_dev);
 
 /**
  * @brief Legge i dati dal sensore BME280
  *
  * @param i2c_dev Puntatore al dispositivo I2C
  * @param data Puntatore alla struttura dove salvare i dati
  * @return true se la lettura è riuscita, false altrimenti
  */
 bool bme280_read_data(const struct device *i2c_dev, bme280_data_t *data);
 
 #endif /* BME280_H */