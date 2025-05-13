"""
Training and saving a Decision Tree model for rain prediction.

This script takes the balanced dataset as input and trains a decision tree classifier to predict whether it will rain the next day ("Rain Tomorrow").

Output:
- File "model_info.txt" containing the tree structure
- File "model.joblib" with the trained model
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

# Create the output directory if it doesn't exist
os.makedirs(OUTPUT_DIR, exist_ok=True)

features = ['Temperature', 'Humidity', 'Pressure']
target = 'Rain Tomorrow'

# Load the dataset
df = pd.read_csv(INPUT_PATH)
X = df[features]
y = df[target]

# TRAINING

# Split into training and test sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train the Decision Tree model
model = DecisionTreeClassifier(max_depth=60)
model.fit(X_train, y_train)

# Save the trained model
joblib.dump(model, MODEL_PATH)
print(f"[✓] Model saved to {MODEL_PATH}")
joblib.dump(model, "model.joblib")

# Export tree rules
tree_rules = export_text(model, feature_names=features)
print(tree_rules)
with open(os.path.join(OUTPUT_DIR, "model_info.txt"), "w") as f:
    f.write(tree_rules)

# EVALUATION

# Classification Report
y_pred = model.predict(X_test)
report = classification_report(y_test, y_pred)
print("[✓] Classification report:")
print(report)
with open(os.path.join(OUTPUT_DIR, "classification_report.txt"), "w") as f:
    f.write(report)

# Confusion Matrix
cm = confusion_matrix(y_test, model.predict(X_test))

# Visualize confusion matrix with heatmap
plt.figure(figsize=(6, 5))
sns.heatmap(cm, annot=True, fmt="d", cmap="Blues", xticklabels=['No Rain', 'Rain'], yticklabels=['No Rain', 'Rain'])
plt.title('Confusion Matrix')
plt.xlabel('Prediction')
plt.ylabel('Actual')
plt.tight_layout()
plt.savefig(os.path.join(OUTPUT_DIR, "confusion_matrix.png"))
plt.show()
