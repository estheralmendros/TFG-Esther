# 🌱 Gemelo Digital de un Invernadero para Optimizar el Riego

Este proyecto ha sido desarrollado por **Esther Almendros Pérez** como parte de su Trabajo de Fin de Grado en Ingeniería de Computadores en la Universidad de Málaga, y tutorizado por Javier Troya Castilla y Paula Muñoz Ariza.

## 📋 Descripción

Este sistema consiste en la creación de un gemelo digital para un invernadero automatizado, con el objetivo de optimizar el uso del agua en la agricultura mediante decisiones inteligentes de riego. El sistema combina una placa microcontroladora, una Raspberry Pi, tecnologías IoT, análisis de datos y un modelo predictivo basado en aprendizaje automático.

Se recogen datos en tiempo real de temperatura, humedad del aire y humedad del suelo, los cuales se almacenan en una base de datos de series temporales y se visualizan en *dashboards*. Un modelo predictivo estima la duración óptima del riego a partir de estas condiciones.

## ✨ Características principales

- **Gemelo físico:** Invernadero a escala con sensores y actuadores.
- **Base de datos temporal:** InfluxDB para almacenar las mediciones.
- **Visualización en tiempo real:** Panel de Grafana conectado a la base de datos.
- **Comunicación IoT:** MQTT (Wemos) o USB Serial (Arduino UNO).
- **Predicción de riego:** Modelo de regresión lineal entrenado con datos reales.
- **Sistema modular:** Basado en contenedores Docker y scripts en Python.

## 🚀 Empezando

### 📋 Requisitos

- Raspberry Pi con Raspbian OS.
- Python 3.8 o superior.
- Docker y Docker Compose.
- Git (opcional).

Se asume que se dispone de un invernadero con los siguientes componentes:

- Microcontrolador Arduino UNO (recomendado) o Wemos D1 R1.
- Sensores: DHT22 y sensor de humedad resistivo.
- Actuadores: Bomba de agua, ventilador y pantalla LCD I2C.

### ⚙️ Instalación

#### 1. Clonar el repositorio

```bash
git clone https://github.com/estheralmendros/TFG-Esther.git
cd TFG-Esther
```

#### 2. Preparar el entorno

- Instala los servicios necesarios mediante IoTStack:
    - Para la opción A: Mosquitto, Node-RED, InfluxDB y Grafana.
    - Para la opción B: InfluxDB y Grafana.

```bash
curl -fsSL https://raw.githubusercontent.com/SensorsIot/IOTstack/master/install.sh | bash
sudo shutdown -r now
cd IOTstack/
./menu.sh
```

- Inicia los servicios:

```bash
docker-compose up -d
```

- Instala las dependencias de Python:

```bash
pip install -r requirements.txt
```

#### 3. Verificación de conexión Serial (opción B)

Una vez conectada la placa Arduino a la Raspberry mediante USB, verificar que aparece en:

```bash
ls /dev/serial/by-id/
```

#### 4. Configurar InfluxDB y Grafana

- Accede al contenedor de InfluxDB:

```bash
docker exec -it influxdb influx
```

- Crea una base de datos llamada `invernadero`.
- En Grafana (`http://<IP-RASPBERRY>:3000/`), configura InfluxDB como fuente de datos y crea los dashboards con las métricas: `temperatura`, `humedad_aire`, `humedad_suelo`, `segs_riego`.

## ▶️ Ejecución

**Con Wemos D1 R1 (opción A):**

- Sube el código `invernadero_wifi.ino` al Wemos desde el IDE de Arduino, configurando el WiFi y el broker MQTT.
- Asegúrate de que Node-RED esté suscrito a los *topics* y reenvíe los datos a InfluxDB.

**Con Arduino UNO (opción B) (recomendada):**

- Sube el código `invernadero_serial.ino` a la placa desde el IDE de Arduino.
- Ejecuta el script de lectura por puerto serie (con el nombre del puerto serial detectado en `/dev/serial/by-id/`):

```bash
python arduino_a_influx.py
```

**Ejecutar predicción del gemelo digital**

```bash
python gemelo_digital.py
```

Esto generará una predicción de los segundos de riego óptimos según las condiciones actuales y lo insertará en InfluxDB. El modelo se guardará como `model.pkl`.

## 👤 Autora

**Esther Almendros Pérez**

Tutorizado por:

- Javier Troya Castilla
- Paula Muñoz Ariza

Universidad de Málaga – ETSI Informática<br>
Grado en Ingeniería de Computadores – Curso 2024/2025
