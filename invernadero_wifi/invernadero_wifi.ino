// Sistema completo en invernadero con WiFi

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>

#define LED D10
#define INA D5 // Motor hélice forward
#define INB D6 // Motor hélice reverse
#define MOTOR D8 // Bomba de agua
#define SENSOR_TEMP D4
#define SENSOR_HUM A0

const char* ssid = "..."; // Nombre de la red WiFi
const char* password = "..."; // Contraseña de la red WiFi
const char* mqtt_server = "..."; // IP de la Raspberry con Mosquitto
const int mqtt_port = 1883; // Puerto MQTT estándar
const char* topic_temp = "/invernadero/temperatura";
const char* topic_hum_aire = "/invernadero/humedad_aire";
const char* topic_hum_suelo = "/invernadero/humedad_suelo";

LiquidCrystal_I2C lcd(0x27, 16, 2); // Serial LCD
DHT dht(SENSOR_TEMP, DHT22);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReadTime = 0; // Guarda el último tiempo en que se leyeron los sensores
unsigned long lastSendTime = 0; // Guarda el último tiempo en que se enviaron los datos

float temp, hum_aire;
int hum_suelo, porcentaje;

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(MOTOR, OUTPUT);

  dht.begin();
  delay(5000); // Tiempo prudencial para que se estabilice el sensor de temperatura tras inicializarlo
  lcd.init(); // Inicializa la comunicación con el LCD
  lcd.backlight(); // Activa la retroiluminación

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    
  Serial.println("\n¡WiFi Conectado!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  reconnect();
}

void reconnect() {
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
  if(temp <= 24) { // frío
    digitalWrite(LED, HIGH); // LED encendido
    digitalWrite(INA, LOW); // Hélice apagada
    digitalWrite(INB, LOW);
  } else { // calor
    digitalWrite(LED, LOW); // LED apagado
    digitalWrite(INA, HIGH); // Hélice encendida
    digitalWrite(INB, LOW);
  }
}

void subsRiego(int porcentaje) { // Subsistema riego
  if(porcentaje >= 0 && porcentaje <= 30) // 0-30%
    digitalWrite(MOTOR, LOW); // Motor encendido
  else if(porcentaje >= 60 && porcentaje <= 100) // 60-100%
    digitalWrite(MOTOR, HIGH); // Motor apagado
}

void sendData(float temp, float hum_aire, int porcentaje, PubSubClient client) { // Envío de datos a MQTT
  char temp_str[6], hum_air_str[6], hum_suelo_str[6];
  dtostrf(temp, 4, 1,temp_str); // Conversión de float a string
  dtostrf(hum_aire, 4, 1,hum_air_str); // Conversión de float a string
  itoa(porcentaje, hum_suelo_str, 10); // Conversión de int a string
  
  client.publish(topic_temp, temp_str);
  client.publish(topic_hum_aire, hum_air_str);
  client.publish(topic_hum_suelo, hum_suelo_str);

  // Debug:
  /*Serial.println("Datos enviados a MQTT:");
  Serial.print("Temperatura: "); Serial.println(temp_str);
  Serial.print("Humedad Aire: "); Serial.println(hum_air_str);
  Serial.print("Humedad Suelo: "); Serial.println(hum_suelo_str);*/
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastReadTime >= 5000) {  // Leer sensores cada 5s
    lastReadTime = currentMillis;
    temp = dht.readTemperature(); // Temperatura en ºC
    hum_aire = dht.readHumidity(); // Humedad del aire en %
    hum_suelo = analogRead(SENSOR_HUM);
    porcentaje = map(hum_suelo, 1020, 30, 0, 100); // Conversión de la humedad del suelo a %

    // Debug:
    /*Serial.print("Humedad Aire: ");
    Serial.print(hum_aire);
    Serial.print(" %\tTemperatura: ");
    Serial.print(temp);
    Serial.print(" *C\tHumedad Suelo: ");
    Serial.print(porcentaje);
    Serial.println(" %");*/
    
    // Muestra de datos en LCD:
    lcd.setCursor(0, 0);
    lcd.print("Temp.: " + String(temp) + "C");
    lcd.setCursor(0, 1);
    lcd.print("Humedad: " + String(porcentaje) + "%");

    subsTemperatura(temp);
    subsRiego(porcentaje);
  }

  if (currentMillis - lastSendTime >= 60000) { // Enviar datos cada 60 segundos (1 minuto)
    lastSendTime = currentMillis;
    if (!client.connected()) {
      reconnect();
    }

    client.loop();
    sendData(temp, hum_aire, porcentaje, client);
  }
}