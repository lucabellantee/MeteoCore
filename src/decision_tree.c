// /**
//  * @file ml_model_decisiontree.c
//  * @brief Implementazione del modello ML basato su Decision Tree
//  */

//  #include "ml_model.h"
//  #include "rain_model.h"  // include del tuo modello decision tree
//  #include <zephyr/logging/log.h>
 
//  LOG_MODULE_REGISTER(ml_model_dt, CONFIG_LOG_DEFAULT_LEVEL);
 
//  /**
//   * @brief Inizializza il modello Decision Tree (stub)
//   */
//  void ml_model_init(void)
//  {
//      LOG_INF("Modello Decision Tree inizializzato");
//  }
 
//  /**
//   * @brief Usa il modello Decision Tree per prevedere la probabilità di pioggia
//   *
//   * @param data Puntatore alla struttura con i dati sensore
//   * @return Probabilità stimata di pioggia (float da 0.0 a 100.0)
//   */
//  float ml_model_predict_rain(const bme280_data_t *data)
//  {
//      float features[3] = {
//          data->humidity,
//          data->temperature,
//          data->pressure
//      };
 
//      int prediction = predict_rain(features);  // 0 o 1
//      LOG_DBG("Previsione pioggia (output modello DT): %d", prediction);
 
//      // Traduci la classificazione binaria in una probabilità stimata
//      return prediction ? 85.0f : 15.0f;
//  }
 