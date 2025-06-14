import serial
from influxdb import InfluxDBClient
import time

# Configura el puerto serie donde está conectado tu Arduino
SERIAL_PORT = "/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A5069RR4-if00-port0"
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
        temp, hum_aire, hum_suelo, segs_riego = map(float, line.split(","))

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
