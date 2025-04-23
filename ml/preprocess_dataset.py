import pandas as pd

INPUT_PATH = "../dataset/usa_rain_prediction_dataset_2024_2025.csv"
OUTPUT_PATH = "../dataset/cleaned_dataset.csv"

# Caricamento dataset con le colonne previste
features = ['Temperature', 'Humidity', 'Pressure'] # usiamo come features le colonne corrispondenti ai dati che avremo disponibili tramite i sensori
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