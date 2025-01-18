// Subsistema Temperatura

#include <DHT.h>
#include <LiquidCrystal_I2C.h>

int INA = 5; // Motor hélice forward
int INB = 6; // Motor hélice reverse
int LED = 8;
int sensorTemp = 9;

int temp, hum;

LiquidCrystal_I2C lcd(0x27, 16, 2); // Serial LCD
DHT dht(sensorTemp, DHT22);
 
void setup() {
  Serial.begin(9600);

  pinMode(INA, OUTPUT); 
  pinMode(INB, OUTPUT);
  pinMode(LED, OUTPUT);

  dht.begin();
  lcd.init(); // Inicializa la comunicación con el LCD
  lcd.backlight(); // Activa la retroiluminación
}

void loop() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();

  lcd.setCursor(0, 0);
  lcd.print("Temperatura: " + String(temp) + "C");
  lcd.setCursor(0, 1);
  lcd.print("Humedad: " + String(hum) + "%");

  if(temp<=22) {
    digitalWrite(INA, LOW); // Hélice apagada
    digitalWrite(INB,LOW);
    digitalWrite(LED, HIGH); // LED encendido
  } else {
    analogWrite(INA, 255); // Hélice encendida
    digitalWrite(INB,LOW);
    digitalWrite(LED, LOW); // LED apagado
  }

  delay(500);
}