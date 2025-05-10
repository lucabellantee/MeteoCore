// /**
//  * @file ml_model.c
//  * @brief Implementazione del wrapper per il modello di Machine Learning
//  */

//  #include "ml_model.h"
//  #include <zephyr/logging/log.h>
//  #include <stdlib.h>
//  #include <math.h>
 
//  LOG_MODULE_REGISTER(ml_model, CONFIG_LOG_DEFAULT_LEVEL);
 
//  /* In un'implementazione reale, qui si caricherebbero 
//     i parametri del modello addestrato */
//  void ml_model_init(void)
//  {
//      LOG_INF("Modello ML inizializzato");
//  }
 
//  /* 
//   * Questo è un modello fittizio che simula una previsione di pioggia 
//   * basata su regole semplici.
//   * In un'implementazione reale, qui si utilizzerebbe un modello ML vero e proprio.
//   */
//  float ml_model_predict_rain(const bme280_data_t *data)
//  {
//      float rain_probability = 0.0f;
     
//      /* Modello semplificato:
//       * - Umidità alta aumenta la probabilità di pioggia
//       * - Pressione bassa aumenta la probabilità di pioggia
//       * - Temperatura ha un effetto moderato
//       */
     
//      /* Contributo dell'umidità */
//      if (data->humidity > 85.0f) {
//          rain_probability += 50.0f;
//      } else if (data->humidity > 70.0f) {
//          rain_probability += 30.0f;
//      } else if (data->humidity > 50.0f) {
//          rain_probability += 10.0f;
//      }
     
//      /* Contributo della pressione */
//      if (data->pressure < 990.0f) {
//          rain_probability += 40.0f;
//      } else if (data->pressure < 1000.0f) {
//          rain_probability += 20.0f;
//      } else if (data->pressure < 1010.0f) {
//          rain_probability += 5.0f;
//      }
     
//      /* Contributo della temperatura (effetto minore) */
//      if (data->temperature > 25.0f && data->humidity > 60.0f) {
//          rain_probability += 10.0f; /* Condizioni per temporali estivi */
//      } else if (data->temperature < 5.0f && data->humidity > 80.0f) {
//          rain_probability += 15.0f; /* Condizioni per neve/pioggia invernale */
//      }
     
//      /* Aggiungi un po' di casualità per simulare l'incertezza del modello */
//      rain_probability += ((float)rand() / RAND_MAX) * 10.0f - 5.0f;
     
//      /* Limita la probabilità tra 0% e 100% */
//      if (rain_probability < 0.0f) {
//          rain_probability = 0.0f;
//      } else if (rain_probability > 100.0f) {
//          rain_probability = 100.0f;
//      }
     
//      LOG_DBG("Previsione pioggia: %.1f%%", rain_probability);
     
//      return rain_probability;
//  }