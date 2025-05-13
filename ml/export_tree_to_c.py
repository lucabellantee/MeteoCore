"""
This script automatically generates the C files `rain_model.c` and `rain_model.h` containing
a representation of the trained Decision Tree model in Python, converted into C code
compatible with embedded environments like Zephyr RTOS on STM32.

- The file `rain_model.c` implements the function `predict_rain` that takes as input a structure `bme280_data_t`
  containing temperature, humidity, and pressure, and returns 0 or 1 based on the rain prediction.
- The file `rain_model.h` contains function prototypes and includes the `bme280.h` file.
"""

import os
from sklearn.tree import _tree
import joblib

features = ['Temperature', 'Humidity', 'Pressure']

# Load the pre-trained model
model = joblib.load("model.joblib")

# Function to convert the decision tree into C code, integrated with the required wrapper
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

# Generate the body of the predict_rain function
predict_function_body = tree_to_c(model, features)

# Complete content of the rain_model.c file
c_code = f"""#include <zephyr/logging/log.h>
#include "rain_model.h"

LOG_MODULE_REGISTER(rain_model, CONFIG_LOG_DEFAULT_LEVEL);

void ml_model_init(void) {{
    LOG_INF("ML model initialized");
}}

int predict_rain(const bme280_data_t *data) {{
    double features[3];
    features[0] = (double)data->temperature;
    features[1] = (double)data->humidity;
    features[2] = (double)data->pressure;
{predict_function_body}}}
"""

# Content of the rain_model.h file
header_code = """#ifndef RAIN_MODEL_H
#define RAIN_MODEL_H

#include "bme280.h"

/**
 * @brief Initializes the ML model
 */
void ml_model_init(void);

/**
 * @brief Predicts the probability of rain using sensor data
 *
 * @param data BME280 sensor data
 * @return Rain probability as a percentage (0-100)
 */
int predict_rain(const bme280_data_t *data);

#endif /* RAIN_MODEL_H */
"""

# Output path
output_dir = "../src"
os.makedirs(output_dir, exist_ok=True)

# Save the files
with open(os.path.join(output_dir, "rain_model.c"), "w") as f:
    f.write(c_code)

with open(os.path.join(output_dir, "rain_model.h"), "w") as f:
    f.write(header_code)

print("[✓] Complete C code generated and saved in 'src/'")
