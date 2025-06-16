#include <BME280-SOLDERED.h>

BME280 bme;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  bme.begin();
}

void loop() {
  // Höhe über (angenommenen) Meeresspiegel, basierend auf Luftdruck als float-Wert
  // Aus Wetterdaten kann der tatsächliche Luftdruck angegeben werden
  Serial.print("\nHöhe: ");
  Serial.print(bme.readAltitude());
  Serial.println(" m");

  // Absoluter Luftdruck auf Meereshöhe in hPa als float-Wert
  Serial.print("Luftdruck: ");
  Serial.print(bme.readPressure());
  Serial.println(" hPa");

  // Relative Luftfeuchtigkeit in Prozent als float-Wert
  Serial.print("Luftfeuchtigkeit: ");
  Serial.print(bme.readHumidity());
  Serial.println("%");

  // Temperatur in Grad Celsius als float-Wert
  Serial.print("Temperatur: ");
  Serial.print(bme.readTemperature());
  Serial.println(" Grad Celsius");

  //Serial.println(bme.readAltitude());
  delay(5000);
}
