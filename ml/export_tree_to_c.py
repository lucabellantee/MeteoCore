"""
Questo script genera automaticamente i file C `rain_model.c` e `rain_model.h` contenenti
una rappresentazione del modello Decision Tree addestrato in Python, convertito in codice C
compatibile con ambienti embedded come Zephyr RTOS su STM32.

- Il file `rain_model.c` implementa la funzione `predict_rain` che prende in input una struttura `bme280_data_t`
  contenente temperature, umidità e pressione, e restituisce 0 o 1 in base alla previsione di pioggia.
- Il file `rain_model.h` contiene i prototipi di funzione e include il file `bme280.h`.
"""

import os
from sklearn.tree import _tree
import joblib

features = ['Temperature', 'Humidity', 'Pressure']

# Carica il modello già addestrato
model = joblib.load("model.joblib")

# Funzione per convertire l'albero decisionale in codice C, integrato con il wrapper richiesto
def tree_to_c(tree, feature_names):
    tree_ = tree.tree_
    feature_name = [
        feature_names[i] if i != _tree.TREE_UNDEFINED else "undefined!"
        for i in tree_.feature
    ]

    def recurse(node, depth):
        indent = "    " * depth
        if tree_.feature[node] != _tree.TREE_UNDEFINED:
            name = feature_name[node]
            threshold = tree_.threshold[node]
            return (
                f"{indent}if (features[{features.index(name)}] <= {threshold:.2f}) {{\n"
                + recurse(tree_.children_left[node], depth + 1)
                + f"{indent}}} else {{\n"
                + recurse(tree_.children_right[node], depth + 1)
                + f"{indent}}}\n"
            )
        else:
            pred = tree_.value[node][0].argmax()
            return f"{indent}return {pred};\n"

    return recurse(0, 2)  # indent starting from 2 to match surrounding code

# Genera corpo della funzione predict_rain
predict_function_body = tree_to_c(model, features)

# Contenuto completo del file rain_model.c
c_code = f"""#include <zephyr/logging/log.h>
#include "rain_model.h"

LOG_MODULE_REGISTER(rain_model, CONFIG_LOG_DEFAULT_LEVEL);

void ml_model_init(void) {{
    LOG_INF("Modello ML inizializzato");
}}

int predict_rain(const bme280_data_t *data) {{
    double features[3];
    features[0] = (double)data->temperature;
    features[1] = (double)data->humidity;
    features[2] = (double)data->pressure;
{predict_function_body}}}
"""

# Contenuto del file rain_model.h
header_code = """#ifndef RAIN_MODEL_H
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
"""

# Percorso di output
output_dir = "../src"
os.makedirs(output_dir, exist_ok=True)

# Salvataggio dei file
with open(os.path.join(output_dir, "rain_model.c"), "w") as f:
    f.write(c_code)

with open(os.path.join(output_dir, "rain_model.h"), "w") as f:
    f.write(header_code)

print("[✓] Codice C completo generato e salvato in 'src/'")