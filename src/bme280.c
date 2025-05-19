/**
 * @file bme280.c
 * @brief Implementation of the driver for the BME280 sensor
 */

 #include "bme280.h"
 #include <zephyr/logging/log.h>
 
 LOG_MODULE_REGISTER(bme280, CONFIG_LOG_DEFAULT_LEVEL);
 
 /* BME280 Registers */
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
 
 /* chip ID values */
 #define BME280_CHIP_ID          0x60
 
 /* Calibration Parameters */
 struct bme280_calib_param {
     /* Temperature */
     uint16_t dig_t1;
     int16_t  dig_t2;
     int16_t  dig_t3;
     
     /* Pressure */
     uint16_t dig_p1;
     int16_t  dig_p2;
     int16_t  dig_p3;
     int16_t  dig_p4;
     int16_t  dig_p5;
     int16_t  dig_p6;
     int16_t  dig_p7;
     int16_t  dig_p8;
     int16_t  dig_p9;
     
     /* Humidity */
     uint8_t  dig_h1;
     int16_t  dig_h2;
     uint8_t  dig_h3;
     int16_t  dig_h4;
     int16_t  dig_h5;
     int8_t   dig_h6;
     
     int32_t t_fine;
 };
 
 static struct bme280_calib_param calib_data;
 static uint8_t bme280_i2c_addr = 0x76;  

 /**
  * @brief Reads a byte from the specified register
  */
 static int bme280_read_reg(const struct device *i2c_dev, uint8_t reg, uint8_t *data)
 {
     return i2c_reg_read_byte(i2c_dev, bme280_i2c_addr, reg, data);
 }
 
 /**
  * @brief Writes a byte to the specified register
  */
 static int bme280_write_reg(const struct device *i2c_dev, uint8_t reg, uint8_t value)
 {
     return i2c_reg_write_byte(i2c_dev, bme280_i2c_addr, reg, value);
 }
 
 /**
  * @brief Reads the calibration parameters
  */
 static bool read_calibration_data(const struct device *i2c_dev)
 {
     uint8_t buf[24];
     
     /* Reads the first calibration parameters (0x88-0x9F) */
     if (i2c_burst_read(i2c_dev, bme280_i2c_addr, 0x88, buf, 24) != 0) {
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
     
     /* Reads parameter dig_h1 */
     if (bme280_read_reg(i2c_dev, 0xA1, &calib_data.dig_h1) != 0) {
         return false;
     }
     
     /* Reads humidity calibration parameters */
     if (i2c_burst_read(i2c_dev, bme280_i2c_addr, 0xE1, buf, 7) != 0) {
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
  * @brief Calculates the compensated temperature
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
  * @brief Calculates the compensated pressure
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
         return 0; 
     }
     
     p = 1048576 - adc_press;
     p = (((p << 31) - var2) * 3125) / var1;
     var1 = (((int64_t)calib_data.dig_p9) * (p >> 13) * (p >> 13)) >> 25;
     var2 = (((int64_t)calib_data.dig_p8) * p) >> 19;
     
     p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data.dig_p7) << 4);
     
     return (float)p / 256.0f / 100.0f; // Pressure in hPa
 }
 
 /**
  * @brief Calculates the compensated humidity
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
    if (!device_is_ready(i2c_dev)) {
        return false;
    }

    k_msleep(200);  


    // Scan on both possible addresses (0x76, 0x77)
    for (uint8_t addr = 0x76; addr <= 0x77; addr++) {
        uint8_t dummy = 0;
        struct i2c_msg ping_msg = {
            .buf = &dummy,
            .len = 0,
            .flags = I2C_MSG_WRITE,
        };

        int ret = i2c_transfer(i2c_dev, &ping_msg, 1, addr);

        if (ret == 0) {
            bme280_i2c_addr = addr;
            break;
        }
    }

    if (bme280_i2c_addr == 0) {
        return false;
    }

    k_msleep(10);

    // Sensor configuration
    bme280_write_reg(i2c_dev, BME280_REG_CTRL_HUM, 0x01);     
    bme280_write_reg(i2c_dev, BME280_REG_CTRL_MEAS, 0x27);    
    bme280_write_reg(i2c_dev, BME280_REG_CONFIG, 0xA0);      

    for (int attempt = 0; attempt < 5; attempt++) {
        // 1. Writing the register address for ID
        uint8_t id_reg = 0xD0;
        struct i2c_msg write_msg = {
            .buf = &id_reg,
            .len = 1,
            .flags = I2C_MSG_WRITE | I2C_MSG_STOP,
        };

        int write_ret = i2c_transfer(i2c_dev, &write_msg, 1, bme280_i2c_addr);

        if (write_ret != 0) {
            k_msleep(10);
            continue;
        }

        k_msleep(10);

        // 2. Reading the ID value
        uint8_t chip_id = 0;
        struct i2c_msg read_msg = {
            .buf = &chip_id,
            .len = 1,
            .flags = I2C_MSG_READ | I2C_MSG_STOP,
        };

        int read_ret = i2c_transfer(i2c_dev, &read_msg, 1, bme280_i2c_addr);

        if (read_ret != 0) {
            k_msleep(10);
            continue;
        }


        if (chip_id == 0x60) {

            // Reading the calibration parameters
            if (!read_calibration_data(i2c_dev)) {
                return false;
            }

            return true;
        } else {
        }

        k_msleep(20);
    }

    return false;
}


 bool bme280_read_data(const struct device *i2c_dev, bme280_data_t *data)
 {
     uint8_t buf[8];
     int32_t adc_temp, adc_press, adc_hum;
     
     /* Reads the raw data */
     if (i2c_burst_read(i2c_dev, bme280_i2c_addr, BME280_REG_PRESS_MSB, buf, 8) != 0) {
         return false;
     }
     
     /* Extracts the ADC values */
     adc_press = ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) | (buf[2] >> 4);
     adc_temp = ((uint32_t)buf[3] << 12) | ((uint32_t)buf[4] << 4) | (buf[5] >> 4);
     adc_hum = ((uint32_t)buf[6] << 8) | buf[7];
     
     /* Calculates the compensated values */
     data->temperature = compensate_temperature(adc_temp);
     data->pressure = compensate_pressure(adc_press);
     data->humidity = compensate_humidity(adc_hum);
     

     
     return true;
 }