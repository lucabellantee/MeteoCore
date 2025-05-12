# 🌧️ USA Rainfall Prediction Dataset (2024–2025)

Il dataset utilizzato fornisce dati meteorologici dettagliati raccolti in 20 città principali degli Stati Uniti durante gli anni 2024 e 2025.

## Contenuto del dataset

- Oltre 2 anni di dati giornalieri
- Ogni riga rappresenta un giorno specifico in una delle città monitorate
- Include attributi fondamentali come:
  - Data
  - Luogo
  - Temperatura
  - Umidità
  - Velocità del vento
  - Copertura nuvolosa
  - Pressione atmosferica
  - Precipitazioni

## 🎯 Obiettivo

Il dataset ha come target la colonna **`RainTomorrow`**, un'etichetta binaria:
- `1`: ha piovuto il giorno successivo
- `0`: non ha piovuto il giorno successivo

Questo lo rende particolarmente adatto a problemi di classificazione binaria per il machine learning.

## Note aggiuntive

- Verificare ed eventualmente trattare valori mancanti o anomali
- Bilanciamento delle classi
- Le date sono in formato `YYYY-MM-DD`
- Il file CSV originale non è incluso nel repository (aggiunto a `.gitignore`)

## Fonte

Dataset disponibile su [Kaggle](https://www.kaggle.com/datasets/waqi786/usa-rainfall-prediction-dataset-2024-2025)  
