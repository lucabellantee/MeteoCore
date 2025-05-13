#ifndef RAIN_MODEL_H
#define RAIN_MODEL_H

#include "bme280.h"

/**
 * @brief Initializes the ML model
 */
void ml_model_init(void);

/**
 * @brief Predicts the probability of rain using the sensor data
 *
 * @param data BME280 sensor data
 * @return Rain probability in percentage (0-100)
 */
int predict_rain(const bme280_data_t *data);

#endif /* RAIN_MODEL_H */
