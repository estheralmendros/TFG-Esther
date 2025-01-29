// Primer prototipo de sistema completo en invernadero

#include <DHT.h>
#include <LiquidCrystal_I2C.h>

int LED = 10;
int INA = 5; // Motor hélice forward
int INB = 6; // Motor hélice reverse
int motor = 8; // Bomba de agua
int sensorTemp = 9;

int temp, hum, porcentaje;

LiquidCrystal_I2C lcd(0x27, 16, 2); // Serial LCD
DHT dht(sensorTemp, DHT22);

void setup() {
  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(motor, OUTPUT);

  dht.begin();
  lcd.init(); // Inicializa la comunicación con el LCD
  lcd.backlight(); // Activa la retroiluminación
}

void loop() {
  temp = dht.readTemperature();
  hum = analogRead(A3); // Sensor de humedad del suelo
  porcentaje = map(hum, 1020, 30, 0, 100);

  lcd.setCursor(0, 0);
  lcd.print("Temperatura: " + String(temp) + "C");
  lcd.setCursor(0, 1);
  lcd.print("Humedad: " + String(porcentaje));
  lcd.setCursor(12, 1);
  lcd.print("%");

  // Subsistema temperatura:
  if(temp<=22) { // frío
    digitalWrite(LED, HIGH); // LED encendido
    digitalWrite(INA, LOW); // Hélice apagada
    digitalWrite(INB, LOW);
  } else { // calor
    digitalWrite(LED, LOW); // LED apagado
    analogWrite(INA, 255); // Hélice encendida
    digitalWrite(INB, LOW);
  }

  // Subsistema riego:
  if(porcentaje>=0 && porcentaje<=30) // 0-30%
    digitalWrite(motor, LOW); // Motor encendido
  else if(porcentaje>=60 && porcentaje<=100) // 60-100%
    digitalWrite(motor, HIGH); // Motor apagado

  delay(500);
}