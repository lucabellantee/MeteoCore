"""
Script di bilanciamento del dataset meteorologico tramite SMOTE (Synthetic Minority Over-sampling Technique), per evitare che il modello predittivo
sia eccessivamente influenzato dalla classe maggioritaria.

Output: un nuovo dataset CSV bilanciato, pronto per essere utilizzato nel training del modello.
"""

import pandas as pd
from imblearn.over_sampling import SMOTE
import matplotlib.pyplot as plt
import seaborn as sns

# Carica il dataset
INPUT_PATH = "../dataset/cleaned_dataset.csv"
df = pd.read_csv(INPUT_PATH)

# Definisci le features e il target
features = ['Temperature', 'Humidity', 'Pressure']
target = 'Rain Tomorrow'

# Separa il dataset in features (X) e target (y)
X = df[features]
y = df[target]

# Applica SMOTE per il ribilanciamento sull'intero dataset
smote = SMOTE(random_state=42)
X_res, y_res = smote.fit_resample(X, y)

# Visualizza la distribuzione prima e dopo SMOTE
fig, axes = plt.subplots(1, 2, figsize=(12, 5))

sns.countplot(x=y, ax=axes[0])
axes[0].set_title('Distribuzione originale')

sns.countplot(x=y_res, ax=axes[1])
axes[1].set_title('Distribuzione dopo SMOTE')

plt.tight_layout()
plt.show()

# Mostra il numero di campioni prima e dopo SMOTE
print(f"Numero di campioni originali: {y.value_counts()}")
print(f"Numero di campioni dopo SMOTE: {y_res.value_counts()}")

# Crea un nuovo DataFrame bilanciato
df_res = pd.DataFrame(X_res, columns=features)
df_res[target] = y_res

# Salva il nuovo dataset bilanciato
OUTPUT_PATH = "../dataset/cleaned_dataset_balanced.csv"
df_res.to_csv(OUTPUT_PATH, index=False)

print(f"[✓] Dataset bilanciato salvato in {OUTPUT_PATH}")