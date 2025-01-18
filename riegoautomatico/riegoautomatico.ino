// Primer prototipo en maceta

#include <LiquidCrystal_I2C.h>

int motor = 8;

int read, porcentaje;

LiquidCrystal_I2C lcd(0x27, 16, 2); // Serial LCD

void setup() {
  Serial.begin(9600);

  pinMode(motor, OUTPUT);

  lcd.init(); // Inicializa la comunicación con el LCD
  lcd.backlight(); // Activa la retroiluminación
}

void loop() {
  read = analogRead(A3);
  porcentaje = map(read, 1030, 30, 0, 100); // Para el sensor YL-69

  lcd.setCursor(0, 0);
  lcd.print("Humedad: " + String(porcentaje) + "%");

  if(read<1030 && read>=730) // 0-30%
    digitalWrite(motor, LOW);
  else if(read<430 && read>0) // 60-100%
    digitalWrite(motor, HIGH);

  if(digitalRead(motor)==LOW) { // motor encendido
    lcd.setCursor(0, 1);
    lcd.print("MOTOR ON");
  } else { // motor apagado
    lcd.setCursor(0, 1);
    lcd.print("MOTOR OFF");
  }

  delay(500);
}