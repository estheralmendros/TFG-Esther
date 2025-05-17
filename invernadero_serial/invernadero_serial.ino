// Sistema completo de invernadero con comunicación Serial

#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define LED 10
#define INA 5 // Motor hélice forward
#define INB 6 // Motor hélice reverse
#define MOTOR 7 // Bomba de agua
#define SENSOR_TEMP 4
#define SENSOR_HUM A0

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD I2C 16x2
DHT dht(SENSOR_TEMP, DHT22);

unsigned long lastReadTimeT = 0; // Guarda el último tiempo en que se leyó el sensor de temperatura
unsigned long lastReadTimeH = 0; // Guarda el último tiempo en que se leyó el sensor de humedad
unsigned long lastSendTime = 0; // Guarda el último tiempo en que se enviaron los datos
unsigned long tiempo_inicio_riego = 0;

float temp = 0.0, hum_aire = 0.0, segs_riego = 0.0;
int hum_suelo = 0, porcentaje = 0;
bool motor_ON = false;

void setup() {
  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  digitalWrite(MOTOR, HIGH); // Apagado por defecto

  dht.begin();
  delay(5000); // Tiempo prudencial para que se estabilice el sensor de temperatura tras inicializarlo
  lcd.init(); // Inicializa la comunicación con el LCD
  lcd.backlight(); // Activa la retroiluminación
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
}

void subsTemperatura(float t) { // Subsistema temperatura
  if (t <= 25) { // frío
    digitalWrite(LED, HIGH); // LED encendido
    digitalWrite(INA, LOW); // Hélice apagada
    digitalWrite(INB, LOW);
  } else { // calor
    digitalWrite(LED, LOW); // LED apagado
    digitalWrite(INA, HIGH); // Hélice encendida
    digitalWrite(INB, LOW);
  }
}

void subsRiego(int hum_pct) { // Subsistema riego
  unsigned long ahora = millis();

  if (hum_pct <= 30 && !motor_ON) {
    digitalWrite(MOTOR, LOW); // Encender bomba
    tiempo_inicio_riego = ahora;
    motor_ON = true;
  }

  if (motor_ON && (ahora - tiempo_inicio_riego >= 5000)) {
    digitalWrite(MOTOR, HIGH); // Apagar bomba
    segs_riego += 5.0; // Añadir 5 segundos al acumulado
    motor_ON = false;
  }
}

void sendData() { // Envío de datos por USB Serial
  Serial.print(temp, 1); // Temperatura
  Serial.print(",");
  Serial.print(hum_aire, 1); // Humedad aire
  Serial.print(",");
  Serial.print(porcentaje); // Humedad suelo (%)
  Serial.print(",");
  Serial.println(segs_riego, 1); // Segundos de riego

  segs_riego = 0.0; // Reiniciar contador de riego
}

void loop() {
  unsigned long now = millis();

  if (now - lastReadTimeH >= 1000) { // Leer sensor de humedad cada 1s
    lastReadTimeH = now;
    hum_suelo = analogRead(SENSOR_HUM);
    porcentaje = map(hum_suelo, 1024, 0, 0, 100); // Conversión de la humedad del suelo a %

    lcd.setCursor(0, 1);
    lcd.print("Humedad: ");
    lcd.print(porcentaje);
    lcd.print("%");

    subsRiego(porcentaje);
  }

  if (now - lastReadTimeT >= 5000) { // Leer sensor de temperatura cada 5s
    lastReadTimeT = now;
    float t = dht.readTemperature(); // Temperatura en ºC
    float h = dht.readHumidity(); // Humedad del aire en %
    if (!isnan(t) && !isnan(h)) {
      temp = t;
      hum_aire = h;
    }

    lcd.setCursor(0, 0);
    lcd.print("Temp.: ");
    lcd.print(temp);
    lcd.print("C");

    subsTemperatura(temp);
  }

  if (now - lastSendTime >= 60000) {
    sendData();
    lastSendTime = now;
  }
}