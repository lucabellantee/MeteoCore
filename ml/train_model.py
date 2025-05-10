import pandas as pd
from sklearn.tree import DecisionTreeClassifier, export_text
from sklearn.metrics import classification_report, confusion_matrix 
import joblib
import seaborn as sns
import matplotlib.pyplot as plt

INPUT_PATH = "../dataset/cleaned_dataset_balanced.csv"
features = ['Temperature', 'Humidity', 'Pressure']
target = 'Rain Tomorrow'

# caricamento del dataset
df = pd.read_csv(INPUT_PATH)
X = df[features]
y = df[target]

# Train test split
from sklearn.model_selection import train_test_split
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Addestramento del Decision Tree 
model = DecisionTreeClassifier(max_depth=60)
model.fit(X_train, y_train)

# Valutazione della performance
print("[✓] Report di classificazione:")
print(classification_report(y_test, model.predict(X_test)))

# Esportazione testo dell'albero
tree_rules = export_text(model, feature_names=features)
print(tree_rules)

# Matrice di confusione
cm = confusion_matrix(y_test, model.predict(X_test))

# Visualizzazione matrice di confusione con heatmap
plt.figure(figsize=(6, 5))
sns.heatmap(cm, annot=True, fmt="d", cmap="Blues", xticklabels=['No Rain', 'Rain'], yticklabels=['No Rain', 'Rain'])
plt.title('Matrice di Confusione')
plt.xlabel('Predizione')
plt.ylabel('Reale')
plt.show()

# Salvataggio delle informazioni sull'albero
with open("model_info.txt", "w") as f:
    f.write(tree_rules)

# si salva su disco il modello preaddestrato in modo da poterlo recuperare in un secondo momento
joblib.dump(model, "model.joblib")