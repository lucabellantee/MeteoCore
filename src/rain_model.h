#ifndef RAIN_MODEL_H
#define RAIN_MODEL_H

#include "bme280.h"

/**
 * @brief Inizializza il modello di ML
 */
void ml_model_init(void);

/**
 * @brief Predice la probabilita di pioggia usando i dati del sensore
 *
 * @param data Dati del sensore BME280
 * @return Probabilita di pioggia in percentuale (0-100)
 */
int predict_rain(const bme280_data_t *data);

#endif /* RAIN_MODEL_H */
