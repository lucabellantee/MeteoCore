"""
Script di preprocessing per il dataset meteorologico. Effettua pulizia e preparazione del dataset contenente focalizzandosi solo sulle colonne che
possono essere replicate tramite i sensori: temperatura, umidità e pressione. L'obiettivo è creare un dataset coerente e privo di valori mancanti,
pronto per essere utilizzato dal modello predittivo.

Operazioni eseguite:
- Caricamento del dataset da file CSV
- Selezione delle colonne di interesse (features e target)
- Rimozione dei valori nulli e conversione dei dati in formato numerico
- Verifica che il target sia binario (0 o 1)
- Salvataggio del dataset pulito su file

Output: un nuovo file CSV con dati puliti e pronti per l’analisi o il training del modello.
"""

import pandas as pd

INPUT_PATH = "../dataset/usa_rain_prediction_dataset_2024_2025.csv"
OUTPUT_PATH = "../dataset/cleaned_dataset.csv"

# usiamo come features le colonne corrispondenti ai dati che avremo disponibili tramite i sensori
features = ['Temperature', 'Humidity', 'Pressure']
target = 'Rain Tomorrow'

# Caricamento e selezione colonne
try:
    df = pd.read_csv(INPUT_PATH)
except Exception as e:
    raise RuntimeError(f"Errore nel caricamento del dataset: {e}")

# Seleziona solo le colonne necessarie
required_columns = features + [target]
df = df[required_columns]

# Rimozione valori nulli o NaN
df = df.dropna()

# Forziamo la conversione in tipo numerico dei dati
for col in features:
    df[col] = pd.to_numeric(df[col], errors='coerce')

# Filtra righe corrotte dopo conversione
df = df.dropna()

# Ci assicuriamo che il target sia binario e intero
df[target] = df[target].astype(int)

# Salvataggio dataset pulito
try:
    df.to_csv(OUTPUT_PATH, index=False)
    print(f"[✓] Dataset preprocessato e salvato in {OUTPUT_PATH}")
except Exception as e:
    raise RuntimeError(f"Errore nel salvataggio del dataset: {e}")