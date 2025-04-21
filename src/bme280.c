/**
 * @file bme280.c
 * @brief Implementazione del driver per il sensore BME280
 */

 #include "bme280.h"
 #include <zephyr/logging/log.h>
 
 LOG_MODULE_REGISTER(bme280, CONFIG_LOG_DEFAULT_LEVEL);
 
 /* Registri del BME280 */
 #define BME280_REG_ID           0xD0
 #define BME280_REG_RESET        0xE0
 #define BME280_REG_CTRL_HUM     0xF2
 #define BME280_REG_STATUS       0xF3
 #define BME280_REG_CTRL_MEAS    0xF4
 #define BME280_REG_CONFIG       0xF5
 #define BME280_REG_PRESS_MSB    0xF7
 #define BME280_REG_PRESS_LSB    0xF8
 #define BME280_REG_PRESS_XLSB   0xF9
 #define BME280_REG_TEMP_MSB     0xFA
 #define BME280_REG_TEMP_LSB     0xFB
 #define BME280_REG_TEMP_XLSB    0xFC
 #define BME280_REG_HUM_MSB      0xFD
 #define BME280_REG_HUM_LSB      0xFE
 
 /* Valori del chip ID */
 #define BME280_CHIP_ID          0x60  // Alcuni moduli potrebbero usare 0x60 o 0x61
 
 /* Parametri di calibrazione */
 struct bme280_calib_param {
     /* Temperatura */
     uint16_t dig_t1;
     int16_t  dig_t2;
     int16_t  dig_t3;
     
     /* Pressione */
     uint16_t dig_p1;
     int16_t  dig_p2;
     int16_t  dig_p3;
     int16_t  dig_p4;
     int16_t  dig_p5;
     int16_t  dig_p6;
     int16_t  dig_p7;
     int16_t  dig_p8;
     int16_t  dig_p9;
     
     /* Umidità */
     uint8_t  dig_h1;
     int16_t  dig_h2;
     uint8_t  dig_h3;
     int16_t  dig_h4;
     int16_t  dig_h5;
     int8_t   dig_h6;
     
     /* Variabile per temperature fine */
     int32_t t_fine;
 };
 
 static struct bme280_calib_param calib_data;
 
 /**
  * @brief Legge un byte dal registro specificato
  */
 static int bme280_read_reg(const struct device *i2c_dev, uint8_t reg, uint8_t *data)
 {
     int ret = i2c_reg_read_byte(i2c_dev, BME280_I2C_ADDR, reg, data);
     if (ret != 0) {
         LOG_ERR("Errore di lettura, registro 0x%02x, ret: %d", reg, ret);
     }
     return ret;
 }
 
 /**
  * @brief Scrive un byte nel registro specificato
  */
 static int bme280_write_reg(const struct device *i2c_dev, uint8_t reg, uint8_t value)
 {
     int ret = i2c_reg_write_byte(i2c_dev, BME280_I2C_ADDR, reg, value);
     if (ret != 0) {
         LOG_ERR("Errore di scrittura, registro 0x%02x, valore 0x%02x, ret: %d", reg, value, ret);
     }
     return ret;
 }
 
 /**
  * @brief Legge i parametri di calibrazione
  */
 static bool read_calibration_data(const struct device *i2c_dev)
 {
     uint8_t buf[24];
     
     /* Legge i primi parametri di calibrazione (0x88-0x9F) */
     if (i2c_burst_read(i2c_dev, BME280_I2C_ADDR, 0x88, buf, 24) != 0) {
         LOG_ERR("Impossibile leggere i parametri di calibrazione");
         return false;
     }
     
     calib_data.dig_t1 = (uint16_t)(buf[1] << 8) | buf[0];
     calib_data.dig_t2 = (int16_t)((buf[3] << 8) | buf[2]);
     calib_data.dig_t3 = (int16_t)((buf[5] << 8) | buf[4]);
     
     calib_data.dig_p1 = (uint16_t)(buf[7] << 8) | buf[6];
     calib_data.dig_p2 = (int16_t)((buf[9] << 8) | buf[8]);
     calib_data.dig_p3 = (int16_t)((buf[11] << 8) | buf[10]);
     calib_data.dig_p4 = (int16_t)((buf[13] << 8) | buf[12]);
     calib_data.dig_p5 = (int16_t)((buf[15] << 8) | buf[14]);
     calib_data.dig_p6 = (int16_t)((buf[17] << 8) | buf[16]);
     calib_data.dig_p7 = (int16_t)((buf[19] << 8) | buf[18]);
     calib_data.dig_p8 = (int16_t)((buf[21] << 8) | buf[20]);
     calib_data.dig_p9 = (int16_t)((buf[23] << 8) | buf[22]);
     
     /* Legge il parametro dig_h1 */
     if (bme280_read_reg(i2c_dev, 0xA1, &calib_data.dig_h1) != 0) {
         LOG_ERR("Impossibile leggere dig_h1");
         return false;
     }
     
     /* Legge i parametri di calibrazione dell'umidità */
     if (i2c_burst_read(i2c_dev, BME280_I2C_ADDR, 0xE1, buf, 7) != 0) {
         LOG_ERR("Impossibile leggere i parametri di calibrazione dell'umidità");
         return false;
     }
     
     calib_data.dig_h2 = (int16_t)((buf[1] << 8) | buf[0]);
     calib_data.dig_h3 = buf[2];
     calib_data.dig_h4 = (int16_t)((buf[3] << 4) | (buf[4] & 0x0F));
     calib_data.dig_h5 = (int16_t)((buf[5] << 4) | (buf[4] >> 4));
     calib_data.dig_h6 = (int8_t)buf[6];
     
     return true;
 }
 
 /**
  * @brief Calcola la temperatura compensata
  */
 static float compensate_temperature(int32_t adc_temp)
 {
     int32_t var1, var2;
     
     var1 = ((((adc_temp >> 3) - ((int32_t)calib_data.dig_t1 << 1))) * 
             ((int32_t)calib_data.dig_t2)) >> 11;
     var2 = (((((adc_temp >> 4) - ((int32_t)calib_data.dig_t1)) * 
             ((adc_temp >> 4) - ((int32_t)calib_data.dig_t1))) >> 12) * 
             ((int32_t)calib_data.dig_t3)) >> 14;
             
     calib_data.t_fine = var1 + var2;
     
     return ((calib_data.t_fine * 5 + 128) >> 8) / 100.0f;
 }
 
 /**
  * @brief Calcola la pressione compensata
  */
 static float compensate_pressure(int32_t adc_press)
 {
     int64_t var1, var2, p;
     
     var1 = ((int64_t)calib_data.t_fine) - 128000;
     var2 = var1 * var1 * (int64_t)calib_data.dig_p6;
     var2 = var2 + ((var1 * (int64_t)calib_data.dig_p5) << 17);
     var2 = var2 + (((int64_t)calib_data.dig_p4) << 35);
     var1 = ((var1 * var1 * (int64_t)calib_data.dig_p3) >> 8) + 
            ((var1 * (int64_t)calib_data.dig_p2) << 12);
     var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_data.dig_p1) >> 33;
     
     if (var1 == 0) {
         return 0; // per evitare divisione per zero
     }
     
     p = 1048576 - adc_press;
     p = (((p << 31) - var2) * 3125) / var1;
     var1 = (((int64_t)calib_data.dig_p9) * (p >> 13) * (p >> 13)) >> 25;
     var2 = (((int64_t)calib_data.dig_p8) * p) >> 19;
     
     p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data.dig_p7) << 4);
     
     return (float)p / 256.0f / 100.0f; // Pressione in hPa
 }
 
 /**
  * @brief Calcola l'umidità compensata
  */
 static float compensate_humidity(int32_t adc_hum)
 {
     int32_t v_x1_u32r;
     
     v_x1_u32r = calib_data.t_fine - ((int32_t)76800);
     v_x1_u32r = (((((adc_hum << 14) - (((int32_t)calib_data.dig_h4) << 20) - 
                 (((int32_t)calib_data.dig_h5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * 
                 (((((((v_x1_u32r * ((int32_t)calib_data.dig_h6)) >> 10) * 
                 (((v_x1_u32r * ((int32_t)calib_data.dig_h3)) >> 11) + ((int32_t)32768))) >> 10) + 
                 ((int32_t)2097152)) * ((int32_t)calib_data.dig_h2) + 8192) >> 14));
                 
     v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * 
                 ((int32_t)calib_data.dig_h1)) >> 4));
                 
     v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
     v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
     
     return (float)(v_x1_u32r >> 12) / 1024.0f;
 }
 
 bool bme280_init(const struct device *i2c_dev)
 {
     uint8_t chip_id;
     
     LOG_INF("Inizializzazione BME280 all'indirizzo 0x%02x", BME280_I2C_ADDR);
     
     if (!device_is_ready(i2c_dev)) {
         LOG_ERR("Dispositivo I2C non pronto");
         return false;
     }
     
     /* Test semplice I2C */
     LOG_INF("Inizio scansione dispositivi I2C...");
     for (uint8_t addr = 1; addr < 128; addr++) {
         uint8_t dummy = 0;
         int result = i2c_write(i2c_dev, &dummy, 0, addr);
         if (result == 0) {
             LOG_INF("Trovato dispositivo I2C all'indirizzo: 0x%02x", addr);
         }
     }
     LOG_INF("Scansione I2C completata");
     
     /* Verifica l'ID del chip */
     int ret = bme280_read_reg(i2c_dev, BME280_REG_ID, &chip_id);
     if (ret != 0) {
         LOG_ERR("Impossibile leggere l'ID del chip, errore: %d", ret);
         return false;
     }
     
     LOG_INF("Letto chip ID: 0x%02x, aspettato: 0x%02x", chip_id, BME280_CHIP_ID);
     
     /* Accetta sia 0x60 che 0x61 come ID validi */
     if (chip_id != BME280_CHIP_ID && chip_id != 0x61) {
         LOG_ERR("ID del chip non valido: 0x%02x", chip_id);
         return false;
     }
     
     /* Reset del sensore */
     if (bme280_write_reg(i2c_dev, BME280_REG_RESET, 0xB6) != 0) {
         LOG_ERR("Impossibile resettare il sensore");
         return false;
     }
     
     /* Attesa per il completamento del reset */
     k_sleep(K_MSEC(10));
     
     /* Legge i parametri di calibrazione */
     if (!read_calibration_data(i2c_dev)) {
         return false;
     }
     
     /* Configura l'oversampling per umidità */
     if (bme280_write_reg(i2c_dev, BME280_REG_CTRL_HUM, 0x01) != 0) {
         LOG_ERR("Impossibile configurare CTRL_HUM");
         return false;
     }
     
     /* Configura l'oversampling per temperatura e pressione e modalità operativa */
     if (bme280_write_reg(i2c_dev, BME280_REG_CTRL_MEAS, 0x27) != 0) {
         LOG_ERR("Impossibile configurare CTRL_MEAS");
         return false;
     }
     
     /* Configura il rate di standby e filtro */
     if (bme280_write_reg(i2c_dev, BME280_REG_CONFIG, 0x00) != 0) {
         LOG_ERR("Impossibile configurare CONFIG");
         return false;
     }
     
     LOG_INF("Sensore BME280 inizializzato correttamente");
     return true;
 }
 
 bool bme280_read_data(const struct device *i2c_dev, bme280_data_t *data)
 {
     uint8_t buf[8];
     int32_t adc_temp, adc_press, adc_hum;
     
     /* Legge i dati grezzi */
     if (i2c_burst_read(i2c_dev, BME280_I2C_ADDR, BME280_REG_PRESS_MSB, buf, 8) != 0) {
         LOG_ERR("Impossibile leggere i dati del sensore");
         return false;
     }
     
     /* Estrae i valori ADC */
     adc_press = ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) | (buf[2] >> 4);
     adc_temp = ((uint32_t)buf[3] << 12) | ((uint32_t)buf[4] << 4) | (buf[5] >> 4);
     adc_hum = ((uint32_t)buf[6] << 8) | buf[7];
     
     /* Calcola i valori compensati */
     data->temperature = compensate_temperature(adc_temp);
     data->pressure = compensate_pressure(adc_press);
     data->humidity = compensate_humidity(adc_hum);
     
     LOG_INF("T: %.2f°C, P: %.2f hPa, H: %.2f%%", 
             data->temperature, data->pressure, data->humidity);
     
     return true;
 }