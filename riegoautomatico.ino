#include <LiquidCrystal.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

void setup() {
  Serial.begin(9600);
  pinMode(8, OUTPUT);
  lcd.begin(16, 2);
}

void loop() {
  int read = analogRead(A5);
  int porcentaje = map(read, 1030, 30, 0, 100);

  lcd.setCursor(0, 0);
  lcd.print("Humedad: ");
  lcd.setCursor(9, 0);
  lcd.print(porcentaje);
  lcd.setCursor(12, 0);
  lcd.print("%");

  if(read<1030 && read>=730) // 0-30%
    digitalWrite(8, LOW);
  else if(read<430 && read>0) // 60-100%
    digitalWrite(8, HIGH);

  if(digitalRead(8)==LOW) { // motor encendido
    lcd.setCursor(0, 1);
    lcd.print("MOTOR ON");
  } else { // motor apagado
    lcd.setCursor(0, 1);
    lcd.print("MOTOR OFF");
  }

  delay(500);
  lcd.clear();
}