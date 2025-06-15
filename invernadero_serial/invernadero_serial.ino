// Sistema completo de invernadero con comunicación Serial

#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// Asignación de pines de la placa:
#define LED 10
#define INA 5 // Motor hélice forward
#define INB 6 // Motor hélice reverse
#define MOTOR 7 // Bomba de agua
#define SENSOR_TEMP 8
#define SENSOR_HUM A0

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD I2C 16x2
DHT dht(SENSOR_TEMP, DHT22);

unsigned long ultLecturaT = 0; // Último tiempo en que se leyó el sensor de temperatura
unsigned long ultLecturaH = 0; // Último tiempo en que se leyó el sensor de humedad
unsigned long ultEnvio = 0; // Último tiempo en que se enviaron los datos
unsigned long segs_riego_ini = 0; // Tiempo de inicio del riego

float temp, hum_aire, segs_riego = 0.0;
int hum_suelo, hum_pct;
bool motor_ON = false; // Rastrear si el motor está en marcha

void setup() {
  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  digitalWrite(MOTOR, HIGH); // Apagado por defecto

  dht.begin();
  delay(5000); // Estabilización del sensor DHT
  lcd.init(); // Inicializar la comunicación con el LCD
  lcd.backlight(); // Activar la retroiluminación
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
    digitalWrite(MOTOR, LOW); // Encender bomba
    segs_riego_ini = ahora;
    motor_ON = true;
  }

  if (motor_ON && (ahora - segs_riego_ini >= 5000)) {
    digitalWrite(MOTOR, HIGH); // Apagar bomba
    segs_riego += 5.0; // Añadir 5 segundos al acumulado
    motor_ON = false;
  }
}

void sendData() { // Envío de datos por USB Serial
  if (motor_ON) {
    unsigned long ahora = millis();
    float segs_riego_actual = (ahora - segs_riego_ini) / 1000.0;
    segs_riego += segs_riego_actual;
    segs_riego_ini = ahora; // Reinicio de la variable para el siguiente intervalo
  }

  Serial.print(temp, 1);
  Serial.print(",");
  Serial.print(hum_aire, 1);
  Serial.print(",");
  Serial.print(hum_pct);
  Serial.print(",");
  Serial.println(segs_riego, 1);

  segs_riego = 0.0; // Reinicio del contador de riego
}

void loop() {
  unsigned long ahora = millis();

  if (ahora - ultLecturaH >= 1000) { // Leer sensor de humedad cada 1s
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

  if (ahora - ultLecturaT >= 5000) { // Leer sensor de temperatura cada 5s
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
    sendData();
    ultEnvio = ahora;
  }
}