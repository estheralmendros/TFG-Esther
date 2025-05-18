import serial
from influxdb import InfluxDBClient
import time

# Configura el puerto serie donde está conectado tu Arduino
SERIAL_PORT = "/dev/ttyUSB0"   # O prueba con /dev/ttyACM0 si no funciona
BAUD_RATE = 9600

# Configuración de la base de datos InfluxDB 1.x
INFLUX_HOST = "localhost"
INFLUX_PORT = 8086
INFLUX_DB = "invernadero"

# Inicializar cliente InfluxDB
client = InfluxDBClient(host=INFLUX_HOST, port=INFLUX_PORT)
client.switch_database(INFLUX_DB)

# Abrir el puerto serie
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # Esperar a que el puerto se estabilice

print("Esperando datos desde Arduino...")

while True:
    try:
        line = ser.readline().decode('utf-8', errors="ignore").strip()
        if not line or "nan" in line:
            continue

        print("Recibido:", line)
        parts = line.split(',')
        if len(parts) != 4:
            print("Formato inválido")
            continue

        temp = float(parts[0])
        hum_aire = float(parts[1])
        hum_suelo = float(parts[2])
        segs_riego = float(parts[3])

        json_body = [
            {
                "measurement": "temperatura",
                "fields": {"valor": temp}
            },
            {
                "measurement": "humedad_aire",
                "fields": {"valor": hum_aire}
            },
            {
                "measurement": "humedad_suelo",
                "fields": {"valor": hum_suelo}
            },
            {
                "measurement": "segs_riego",
                "fields": {"valor": segs_riego}
            }
        ]

        client.write_points(json_body)
        print("Datos escritos en InfluxDB.")

    except Exception as e:
        print("Error:", e)