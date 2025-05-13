"""
Weather dataset balancing script using SMOTE (Synthetic Minority Over-sampling Technique) to avoid the predictive model
being excessively influenced by the majority class.

Output: A new balanced CSV dataset, ready for model training.
"""

import pandas as pd
from imblearn.over_sampling import SMOTE
import matplotlib.pyplot as plt
import seaborn as sns

# Load the dataset
INPUT_PATH = "../dataset/cleaned_dataset.csv"
df = pd.read_csv(INPUT_PATH)

# Define the features and target
features = ['Temperature', 'Humidity', 'Pressure']
target = 'Rain Tomorrow'

# Separate the dataset into features (X) and target (y)
X = df[features]
y = df[target]

# Apply SMOTE for rebalancing the entire dataset
smote = SMOTE(random_state=42)
X_res, y_res = smote.fit_resample(X, y)

# Visualize the distribution before and after SMOTE
fig, axes = plt.subplots(1, 2, figsize=(12, 5))

sns.countplot(x=y, ax=axes[0])
axes[0].set_title('Original Distribution')

sns.countplot(x=y_res, ax=axes[1])
axes[1].set_title('Distribution after SMOTE')

plt.tight_layout()
plt.show()

# Show the number of samples before and after SMOTE
print(f"Number of original samples: {y.value_counts()}")
print(f"Number of samples after SMOTE: {y_res.value_counts()}")

# Create a new balanced DataFrame
df_res = pd.DataFrame(X_res, columns=features)
df_res[target] = y_res

# Save the new balanced dataset
OUTPUT_PATH = "../dataset/cleaned_dataset_balanced.csv"
df_res.to_csv(OUTPUT_PATH, index=False)

print(f"[✓] Balanced dataset saved to {OUTPUT_PATH}")
