// Sources:
// https://funduino.de/nr-28-das-sd-karten-modul
// https://docs.arduino.cc/libraries/sd/

#include <SPI.h>
#include <SD.h>

// Pin zur Erkennung der SD-Karte, sollte bei ESP32 explizit gesetzt werden
#define SD_CS 5
#define FILENAME "/sensor/log/log.csv"

int i = 0;
File log_file;

void setup() {
  Serial.begin(9600);
  delay(1000);

  // SD-Karte initialisieren, bei ESP32 explizit mit Angabe der Pin-Nummer!
  if (!SD.begin(SD_CS)) {
    Serial.println("SD-Initialisierung fehlgeschlagen!");
    while (true)
      ;
  }

  // Verschachtelte Verzeichnisse nacheinander, nicht gleichzeitig erstellen
  if (!SD.exists("/sensor")) {
    if (SD.mkdir("/sensor")) {
      Serial.println("Verzeichnis /sensor erstellt.");
    } else {
      Serial.println("Erstellung von /sensor fehlgeschlagen.");
    }
  } else {
    Serial.println("/sensor existiert bereits.");
  }

  if (!SD.exists("/sensor/log")) {
    if (SD.mkdir("/sensor/log")) {
      Serial.println("Verzeichnis /sensor/log erstellt.");
    } else {
      Serial.println("Erstellung von /sensor/log fehlgeschlagen.");
    }
  } else {
    Serial.println("/sensor/log existiert bereits.");
  }

  if (!SD.exists(FILENAME)) {
    // Write Mode wenn Datei noch nicht existiert
    log_file = SD.open(FILENAME, FILE_WRITE);

    if (log_file) {
      Serial.println("Log-Datei erstellt");
      // Beispiel: Header für CSV-Schreiben
      log_file.print("Uhrzeit;");
      log_file.print("Temperatur;");
      // Zusätzlich Carriage Return (\r) und Newline (\n)
      log_file.println("Luftfeuchtigkeit;");
    } else {
      Serial.println("Log-Datei konnte nicht erstellt werden!");
    }
  } else {
    // Append Mode wenn Datei bereits existiert
    // Laut offizieller Doku nicht unterstützt, geht aber trotzdem hier -> beobachten!!!
    log_file = SD.open(FILENAME, FILE_APPEND);
    if (log_file) {
      Serial.println("Log-Datei geöffnet!");
    } else {
      Serial.println("Log-Datei konnte nicht geöffnet werden!");
    }
  }

  Serial.print("/sensor/log existiert: ");
  Serial.println(SD.exists("/sensor/log"));

  Serial.print("Dateiname: ");
  Serial.println(log_file.name());
}

void loop() {
  // Werte in die CSV schreiben
  log_file.print(i);
  log_file.print(";");
  log_file.print(i * 2);
  log_file.print(";");
  log_file.print(i * 3);
  log_file.println(";");

  // Änderungen speichern! Beim Schließen automatisch ausgeführt
  log_file.flush();

  Serial.println(i);
  i++;
  delay(100);

  if (i == 100) {
    // Datei sollte geschlossen werden, hier der Vollständigkeit halber aufgeführt
    log_file.close();
    Serial.println("File closed...");
    while (1)
      ;
  }
}
