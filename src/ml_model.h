// /**
//  * @file ml_model.h
//  * @brief Wrapper per il modello di Machine Learning
//  */

//  #ifndef ML_MODEL_H
//  #define ML_MODEL_H
 
//  #include "bme280.h"
 
//  /**
//   * @brief Inizializza il modello di ML
//   */
//  void ml_model_init(void);
 
//  /**
//   * @brief Predice la probabilità di pioggia usando i dati del sensore
//   *
//   * @param data Dati del sensore BME280
//   * @return Probabilità di pioggia in percentuale (0-100)
//   */
//  float ml_model_predict_rain(const bme280_data_t *data);
 
//  #endif /* ML_MODEL_H */