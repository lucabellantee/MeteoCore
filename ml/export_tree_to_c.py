from sklearn.tree import _tree
import pandas as pd
import joblib

features = ['Temperature', 'Humidity', 'Pressure']

# Carica modello già addestrato da train_model.py
model = joblib.load("model.joblib")

# funzione che converte l'albero decisionale in una rappresentazione in linguaggio C. Genera il codice C che esegue le stesse decisioni che l'albero decisionale farebbe,
# ma su una piattaforma embedded (come STM32), dove non è possibile utilizzare una libreria Python come sklearn.
def tree_to_c(tree, feature_names):

    # inizializzazione albero e caratteristiche
    tree_ = tree.tree_
    feature_name = [
        feature_names[i] if i != _tree.TREE_UNDEFINED else "undefined!"
        for i in tree_.feature
    ]
    
    # funzione ricorsiva per attraversare l'albero
    def recurse(node, depth):
        indent = "    " * depth
        if tree_.feature[node] != _tree.TREE_UNDEFINED:
            name = feature_name[node]
            threshold = tree_.threshold[node]
            return (
                f"{indent}if (features[{features.index(name)}] <= {threshold:.2f}) {{\n"
                + recurse(tree_.children_left[node], depth + 1)
                + f"{indent}}} else {{\n"
                + recurse(tree_.children_right[node], depth + 1)
                + f"{indent}}}\n"
            )
        else:
            pred = tree_.value[node][0].argmax()
            return f"{indent}return {pred};\n"

    return "int predict_rain(float features[3]) {\n" + recurse(0, 1) + "}"

# generazione codice in c
c_code = tree_to_c(model, features)

# Salva il codice C generato per il modello nella cartella 'ml'
with open("../ml/rain_model.c", "w") as f:
    f.write(c_code)

# Salva l'intestazione (header) C nella cartella 'ml'
with open("../ml/rain_model.h", "w") as f:
    f.write("int predict_rain(float features[3]);\n")

print("[✓] Codice C generato per STM32 e salvato in ml")