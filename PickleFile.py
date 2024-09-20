import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.ensemble import RandomForestClassifier
import pickle

# Load dataset
dataset = pd.read_excel('Motor Dataset.xlsx')
X = dataset.iloc[:, :-1].values
y = dataset.iloc[:, -1].values


X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.25, random_state=0)   # Split dataset into training and testing sets


sc = StandardScaler() # Feature scaling
X_train = sc.fit_transform(X_train)
X_test = sc.transform(X_test)


classifier = RandomForestClassifier(n_estimators=10, criterion='entropy', random_state=0)  # Train RandomForestClassifier
classifier.fit(X_train, y_train)

# Save the model and scaler to pickle files
with open('classifier.pkl', 'wb') as model_file:
    pickle.dump(classifier, model_file)

with open('scaler.pkl', 'wb') as scaler_file:
    pickle.dump(sc, scaler_file)

print("Model and scaler saved successfully!")
