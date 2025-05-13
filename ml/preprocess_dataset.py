"""
Preprocessing script for the weather dataset. It performs cleaning and preparation of the dataset focusing only on the columns that
can be replicated through the sensors: temperature, humidity, and pressure. The goal is to create a consistent dataset with no missing values,
ready to be used by the predictive model.

Operations performed:
- Loading the dataset from a CSV file
- Selecting the columns of interest (features and target)
- Removing null values and converting data to numeric format
- Ensuring the target is binary (0 or 1)
- Saving the cleaned dataset to a file

Output: A new CSV file with cleaned data, ready for analysis or model training.
"""

import pandas as pd

INPUT_PATH = "../dataset/usa_rain_prediction_dataset_2024_2025.csv"
OUTPUT_PATH = "../dataset/cleaned_dataset.csv"

# Use the columns corresponding to the data that will be available from the sensors as features
features = ['Temperature', 'Humidity', 'Pressure']
target = 'Rain Tomorrow'

# Loading and selecting columns
try:
    df = pd.read_csv(INPUT_PATH)
except Exception as e:
    raise RuntimeError(f"Error loading dataset: {e}")

# Select only the necessary columns
required_columns = features + [target]
df = df[required_columns]

# Remove null or NaN values
df = df.dropna()

# Force conversion to numeric type for the data
for col in features:
    df[col] = pd.to_numeric(df[col], errors='coerce')

# Filter out corrupted rows after conversion
df = df.dropna()

# Ensure the target is binary and integer
df[target] = df[target].astype(int)

# Save the cleaned dataset
try:
    df.to_csv(OUTPUT_PATH, index=False)
    print(f"[✓] Preprocessed dataset saved to {OUTPUT_PATH}")
except Exception as e:
    raise RuntimeError(f"Error saving dataset: {e}")
