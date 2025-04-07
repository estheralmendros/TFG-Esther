import influxdb
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LinearRegression
from sklearn.metrics import mean_absolute_error, r2_score
import joblib

# Conectar con InfluxDB
client = influxdb.InfluxDBClient(host='...', port=8086, database='invernadero')

# Obtener datos históricos
query = """
SELECT temperatura, humedad_aire, humedad_suelo, segundos_riego 
FROM riego_data 
WHERE time > now() - 90d
"""
result = client.query(query)
data = list(result.get_points())

# Convertir a DataFrame
df = pd.DataFrame(data)

# Eliminar valores nulos
df.dropna(inplace=True)

# Definir variables de entrada (X) y salida (y)
X = df[['temperatura', 'humedad_aire', 'humedad_suelo']]
y = df['segundos_riego']

# Dividir datos en entrenamiento y prueba (80%-20%)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Crear y entrenar modelo de regresión lineal
model = LinearRegression()
model.fit(X_train, y_train)

# Guardar el modelo
joblib.dump(model, "model.pkl")

# Evaluar el modelo
y_pred = model.predict(X_test)
mae = mean_absolute_error(y_test, y_pred)
r2 = r2_score(y_test, y_pred)

print(f"Error absoluto medio (MAE): {mae:.2f} segundos")
print(f"Coeficiente de determinación (R²): {r2:.2f}")

# Realizar una predicción para nuevas condiciones
nueva_condicion = np.array([[25.0, 60.0, 40.0]])  # Ejemplo: 25°C, 60% humedad aire, 40% humedad suelo
prediccion = model.predict(nueva_condicion)[0]

print(f"Predicción: Se debe regar {prediccion:.2f} segundos")

# Guardar la predicción en InfluxDB
json_body = [
    {
        "measurement": "predicciones_riego",
        "fields": {
            "segundos_riego_predicho": float(prediccion)
        }
    }
]
client.write_points(json_body)

client.close() # Cerrar conexión