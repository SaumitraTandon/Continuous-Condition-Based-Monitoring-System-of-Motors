from flask import Flask, request, jsonify
import pickle
import numpy as np

app = Flask(__name__)

# Load the model and scaler from pickle files
with open('classifier.pkl', 'rb') as model_file:
    classifier = pickle.load(model_file)

with open('scaler.pkl', 'rb') as scaler_file:
    scaler = pickle.load(scaler_file)

@app.route('/predict', methods=['POST'])
def predict():
    try:
        # Get JSON data from POST request
        data = request.json
        features = np.array(data['features']).reshape(1, -1)

        # Scale the features
        scaled_features = scaler.transform(features)

        # Make prediction
        prediction = classifier.predict(scaled_features)
        return jsonify({'prediction': int(prediction[0])})

    except Exception as e:
        # Handle exceptions
        return jsonify({'error': str(e)}), 400

if __name__ == '__main__':
    app.run(debug=True)
