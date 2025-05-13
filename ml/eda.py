"""
This script performs an exploratory data analysis (EDA) on the weather dataset
with the aim of better understanding the distribution of classes and the correlations between variables.
It includes:
- Descriptive statistics of the dataset
- Analysis of the class distribution in the target ("Rain Tomorrow")
- Visualization of class distribution using a bar plot
- Analysis of the correlation between variables using a heatmap
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.preprocessing import LabelEncoder
from imblearn.over_sampling import SMOTE
from collections import Counter

# Load the dataset
INPUT_PATH = "../dataset/cleaned_dataset.csv"
df = pd.read_csv(INPUT_PATH)

# Descriptive statistics of the dataset
print("[✓] Descriptive statistics of the dataset:")
print(df.describe())

# Analyze the class distribution in the target
target = 'Rain Tomorrow'
class_distribution = df[target].value_counts()
print(f"\n[✓] Class distribution in the target '{target}':")
print(class_distribution)

# Visualize the class distribution with a bar plot
plt.figure(figsize=(6, 4))
sns.countplot(x=target, data=df, palette="Set2")
plt.title(f'Class Distribution for "{target}"')
plt.xlabel(target)
plt.ylabel('Count')
plt.show()

# Explore the correlation between variables
correlation_matrix = df.corr()
plt.figure(figsize=(10, 8))
sns.heatmap(correlation_matrix, annot=True, cmap='coolwarm', fmt='.2f')
plt.title("Correlation Matrix Between Variables")
plt.show()
