import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.preprocessing import LabelEncoder
from imblearn.over_sampling import SMOTE
from collections import Counter

# Carica il dataset
INPUT_PATH = "../dataset/cleaned_dataset.csv"
df = pd.read_csv(INPUT_PATH)

# Statistiche descrittive del dataset
print("[✓] Statistiche descrittive del dataset:")
print(df.describe())

# Analizza la distribuzione delle classi nel target
target = 'Rain Tomorrow'
class_distribution = df[target].value_counts()
print(f"\n[✓] Distribuzione delle classi nel target '{target}':")
print(class_distribution)

# Visualizza la distribuzione delle classi con un grafico a barre
plt.figure(figsize=(6, 4))
sns.countplot(x=target, data=df, palette="Set2")
plt.title(f'Distribuzione delle classi per "{target}"')
plt.xlabel(target)
plt.ylabel('Conteggio')
plt.show()

# Esplora la correlazione tra le variabili
correlation_matrix = df.corr()
plt.figure(figsize=(10, 8))
sns.heatmap(correlation_matrix, annot=True, cmap='coolwarm', fmt='.2f')
plt.title("Matrice di correlazione tra le variabili")
plt.show()




