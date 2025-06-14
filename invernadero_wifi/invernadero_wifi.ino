// Sistema completo de invernadero con comunicación WiFi

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// Asignación de pines de la placa:
#define LED D10
#define INA D5 // Motor hélice forward
#define INB D6 // Motor hélice reverse
#define MOTOR D7 // Bomba de agua
#define SENSOR_TEMP D4
#define SENSOR_HUM A0

const char* ssid = "..."; // Nombre de la red WiFi
const char* contrasena = "..."; // Contraseña de la red WiFi
const char* servidor_mqtt = "..."; // IP de la Raspberry con Mosquitto
const int puerto_mqtt = 1883; // Puerto MQTT estándar
const char* topic_temp = "/invernadero/temperatura";
const char* topic_hum_aire = "/invernadero/humedad_aire";
const char* topic_hum_suelo = "/invernadero/humedad_suelo";
const char* topic_segs_riego = "/invernadero/segs_riego";

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD I2C 16x2
DHT dht(SENSOR_TEMP, DHT22);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long ultLecturaT = 0; // Último tiempo en que se leyó el sensor de temperatura
unsigned long ultLecturaH = 0; // Último tiempo en que se leyó el sensor de humedad
unsigned long ultEnvio = 0; // Último tiempo en que se enviaron los datos
unsigned long segs_riego_ini = 0; // Tiempo de inicio del riego

float temp, hum_aire, segs_riego = 0.0;
int hum_suelo, hum_pct;
bool motor_ON = false; // Rastrear si el motor está en marcha

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  digitalWrite(MOTOR, HIGH); // Apagado por defecto

  dht.begin();
  delay(5000); // Estabilización del sensor DHT
  lcd.init(); // Inicializar la comunicación con el LCD
  lcd.backlight(); // Activar la retroiluminación

  WiFi.begin(ssid, contrasena);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    
  Serial.println("\n¡WiFi Conectado!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(servidor_mqtt, puerto_mqtt);
  reconectar();
}

void reconectar() {
  while (!client.connected()) {
    Serial.print("Conectando a MQTT...");
    if (client.connect("WemosD1", "", ""))
      Serial.println("¡Conectado!");
    else {
      Serial.print("Error, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando de nuevo en 5s...");
      delay(5000);
    }
  }
}

void subsTemperatura(float temp) { // Subsistema temperatura
  if (temp <= 25.0) {
    digitalWrite(LED, HIGH); // LED encendido
    digitalWrite(INA, LOW); // Hélice apagada
    digitalWrite(INB, LOW);
  } else {
    digitalWrite(LED, LOW); // LED apagado
    digitalWrite(INA, HIGH); // Hélice encendida
    digitalWrite(INB, LOW);
  }
}

void subsRiego(int hum_pct) { // Subsistema riego
  unsigned long ahora = millis();

  if (hum_pct <= 30 && !motor_ON) {
    digitalWrite(MOTOR, LOW);  // Encender bomba
    segs_riego_ini = ahora;
    motor_ON = true;
  }

  if (motor_ON && (ahora - segs_riego_ini >= 5000)) {
    digitalWrite(MOTOR, HIGH);  // Apagar bomba
    segs_riego += 5.0; // Añadir 5 segundos al acumulado
    motor_ON = false;
  }
}

void sendData() { // Envío de datos a MQTT
  if (motor_ON) {
    unsigned long ahora = millis();
    float segs_riego_actual = (ahora - segs_riego_ini) / 1000.0;
    segs_riego += segs_riego_actual;
    segs_riego_ini = ahora; // Reinicio de la variable para el siguiente intervalo
  }

  // Conversión a string:
  char temp_str[6], hum_air_str[6], hum_suelo_str[6], segs_riego_str[6];
  dtostrf(temp, 4, 1, temp_str);
  dtostrf(hum_aire, 4, 1, hum_air_str);
  itoa(hum_pct, hum_suelo_str, 10);
  dtostrf(segs_riego, 4, 1, segs_riego_str);
  
  client.publish(topic_temp, temp_str);
  client.publish(topic_hum_aire, hum_air_str);
  client.publish(topic_hum_suelo, hum_suelo_str);
  client.publish(topic_segs_riego, segs_riego_str);
  
  segs_riego = 0.0; // Reinicio del contador de riego
}

void loop() {
  unsigned long ahora = millis();

  if (ahora - ultLecturaH >= 1000) {  // Leer sensor de humedad cada 1s
    ultLecturaH = ahora;
    hum_suelo = analogRead(SENSOR_HUM);
    hum_pct = map(hum_suelo, 1024, 0, 0, 100); // Conversión a %

    // Mostrar los datos en el LCD:
    lcd.setCursor(0, 1);
    lcd.print("Humedad: ");
    lcd.print(hum_pct);
    lcd.print("%");

    subsRiego(hum_pct);
  }

  if (ahora - ultLecturaT >= 5000) {  // Leer sensor de temperatura cada 5s
    ultLecturaT = ahora;
    float t = dht.readTemperature(); // Temperatura en ºC
    float h = dht.readHumidity(); // Humedad del aire en %
    if (!isnan(t) && !isnan(h)) {
      temp = t;
      hum_aire = h;
    }
    
    // Mostrar los datos en el LCD:
    lcd.setCursor(0, 0);
    lcd.print("Temp.: ");
    lcd.print(temp);
    lcd.print("C");

    subsTemperatura(temp);
  }

  if (ahora - ultEnvio >= 60000) { // Enviar datos cada 60 segundos (1 minuto)
    if (!client.connected())
      reconectar();

    client.loop();

    sendData();
    ultEnvio = ahora;
  }
}