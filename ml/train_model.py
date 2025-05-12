"""
Script di addestramento e salvataggio di un modello Decision Tree per la previsione della pioggia.

Questo script prende in input il dataset bilanciato e addestra un classificatore ad albero decisionale (Decision Tree) per prevedere se il giorno
successivo pioverà oppure no ("Rain Tomorrow").

Output:
- File "model_info.txt" contenente la struttura dell'albero
- File "model.joblib" con il modello addestrato
"""

import pandas as pd
from sklearn.tree import DecisionTreeClassifier, export_text
from sklearn.metrics import classification_report, confusion_matrix 
import joblib
import seaborn as sns
import matplotlib.pyplot as plt
import os
from sklearn.model_selection import train_test_split

INPUT_PATH = "../dataset/cleaned_dataset_balanced.csv"
OUTPUT_DIR = "outputs"
MODEL_PATH = "model.joblib"

# Crea la directory di output se non esiste
os.makedirs(OUTPUT_DIR, exist_ok=True)

features = ['Temperature', 'Humidity', 'Pressure']
target = 'Rain Tomorrow'

# caricamento del dataset
df = pd.read_csv(INPUT_PATH)
X = df[features]
y = df[target]

# TRAINING

# Suddivisione in training e test set
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Addestramento del Decision Tree 
model = DecisionTreeClassifier(max_depth=60)
model.fit(X_train, y_train)

# Salvataggio del modello preaddestrato
joblib.dump(model, MODEL_PATH)
print(f"[✓] Modello salvato in {MODEL_PATH}")
joblib.dump(model, "model.joblib")

# Testo dell'albero
tree_rules = export_text(model, feature_names=features)
print(tree_rules)
with open(os.path.join(OUTPUT_DIR, "model_info.txt"), "w") as f:
    f.write(tree_rules)

# VALUTAZIONE

# Classification Report
y_pred = model.predict(X_test)
report = classification_report(y_test, y_pred)
print("[✓] Report di classificazione:")
print(report)
with open(os.path.join(OUTPUT_DIR, "classification_report.txt"), "w") as f:
    f.write(report)

# Matrice di confusione
cm = confusion_matrix(y_test, model.predict(X_test))

# Visualizzazione matrice di confusione con heatmap
plt.figure(figsize=(6, 5))
sns.heatmap(cm, annot=True, fmt="d", cmap="Blues", xticklabels=['No Rain', 'Rain'], yticklabels=['No Rain', 'Rain'])
plt.title('Matrice di Confusione')
plt.xlabel('Predizione')
plt.ylabel('Reale')
plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, "confusion_matrix.png"))
plt.show()