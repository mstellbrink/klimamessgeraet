/*--------------- Libraries ---------------*/
#include <BME280-SOLDERED.h>   // Temperatur
#include <TinyGPSPlus.h>       // GPS
#include <HardwareSerial.h>    // GPS
#include <DS1307.h>            // RTC
#include <GUVA-S12SD.h>        // UV (manuell installiert)
#include <Adafruit_SSD1306.h>  // OLED-Display
#include <Adafruit_GFX.h>      // OLED-Display
#include <Wire.h>              // OLED-Display
#include <SPI.h>               // SD-Card
#include <SD.h>                // SD-Card
#include <WiFi.h>              // WiFi

/*----------------- Makros ----------------*/
#define GPS_RX 16            // Receiver-Pin für GPS-UART-Kommunikation
#define GPS_TX 17            // Tranceiver-Pin für GPS-UART-Kommunikation
#define UV_PIN 36            // Pin für UV-Sensor
#define WORKING_VOLTAGE 3.3  // Spannung des UV-Sensors
#define SAMPLING_COUNT 1000  // Zählintervall für UV-Sensor
#define SCREEN_WIDTH 128     // Pixelbreite des OLED-Displays
#define SCREEN_HEIGHT 64     // Pixelhoehe des OLED-Displays
#define SCREEN_ADDRESS 0x3D  // I2C-Adresse des OLED-Displays
#define OLED_RESET -1        // Reset-Pin, -1 weil über den ESP32 gesteuert
//#define resetRTC                     // Nur setzen wenn die Zeit eingestellt werden muss!!!
#define MESSINTERVALL 1000             // Intervall zwischen Messungen in ms
#define TEXTSIZE 1                     // Größe des Texts auf dem Display (1 = 100%), Minimum: 1
#define SD_PIN 5                       // Pin zum Erkennen der SD-Card
#define FOLDER "/klima"                // Ordnername auf SD-Karte
#define FILENAME "/klima/messung.csv"  // Dateiname (mit Verzeichnisangabe!) auf SD-Karte
#define WLAN_SWITCH_PIN 13             // PIN für den Schalter zum WiFi an- und ausschalten

/*--------------- Strukturen --------------*/
typedef struct messwerte {
  float altitude;             // Höhe (BME280)
  float pressure;             // Luftdruck
  float humidity;             // Luftfeuchtigkeit
  float temperature;          // Temperatur
  double latitude;            // Längengrad
  double longitude;           // Breitengrad
  double speed;               // Geschwindigkeit
  double gps_altitude;        // Höhe (Air530 GPS)
  unsigned int satellites;    // Anzahl Satelliten für GPS
  unsigned short int hour;    // Stunde
  unsigned short int minute;  // Minute
  unsigned short int second;  // Sekunde
  unsigned short int year;    // Jahr
  unsigned short int month;   // Monat
  unsigned short int day;     // Tag
  float uv_index;             // UV-Index

} Messung;
Messung messung;  // Strukturvariable für Messwerte

/*----------- Globale Variablen -----------*/
BME280 bme;                                                                // BME280-Temperatur-/Feuchtigkeitssensor
DS1307 rtc;                                                                // DS1307 Real-Time-Clock
HardwareSerial SerialGPS(2);                                               // GPS-Schnittstelle (2 = UART2 auf Pin 16 + 17)
TinyGPSPlus gps;                                                           // GPS-Objekt zur Signalauswertung
GUVAS12SD uv(UV_PIN, WORKING_VOLTAGE, SAMPLING_COUNT);                     // GUVA-S12SD UV-Sensor
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // SSD1306 OLED-Display
File file;                                                                 // Datei für Messwerte

/*------- WiFi Server initialisieren ------*/
WiFiServer server(80);                 // WiFi Server auf Port 80 erstellen
const char *ssid = "ESP32AP";          // SSID für WiFi Zugangspunkt
const char *password = "Password123";  // Passwort für WiFi Zugangspunkt
IPAddress myIP;                        // Variable für IP-Adresse
String header;                         // Variable für HTTP-Requests
unsigned long currentTime = millis();  // Zeit-Variablen für Timeout
unsigned long previousTime = 0;
const long timeoutTime = 2000;
bool wifiAvailable;

/*--------- HTML-Text für Webpage ---------*/
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <title>ESP32 CSV Download</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      padding: 30px;
    }
    .error {
      color: red;
      font-weight: bold;
      margin-top: 20px;
    }
    button {
      font-size: 16px;
      padding: 10px 20px;
      margin: 10px;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <h1>ESP32 Webserver</h1>
  <p>CSV-Datei herunterladen:</p>
  <button onclick="window.location.href='/download'">CSV herunterladen</button>
  {{ERROR_MESSAGE}}
</body>
</html>
)=====";

String page = MAIN_page;

void setup() {
  /* Schnittstellen und Geräte initialisieren */
  Serial.begin(9600);
  bme.begin();
  rtc.begin();
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  pinMode(WLAN_SWITCH_PIN, INPUT_PULLUP);

  /* Display aktivieren */
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  display.setTextSize(TEXTSIZE);
  display.setTextColor(SSD1306_WHITE);

  /* SD-Karte einrichten und Datei für Messwerte öffnen */
  file = setupSDCard();

  /* Optional: Zeit auf RTC einstellen */
#ifdef resetRTC
  setTimeAndDate();
#endif
}

void loop() {
  /* Interrupt wenn WLAN-Schalter aktiviert wird */
  if (digitalRead(WLAN_SWITCH_PIN) == HIGH) {
    page = MAIN_page;
    WiFi.mode(WIFI_AP);
    wifiAvailable = WiFi.softAP(ssid, password);

    // Fehler statt Download-Button auf Webpage anzeigen, wenn die SD-Karte nicht verfügbar ist
    if (!file) {
      page.replace("<button onclick=\"window.location.href='/download'\">CSV herunterladen</button>", "");
      page.replace("{{ERROR_MESSAGE}}", "<p class='error'>⚠️ SD-Karte nicht erkannt!</p>");
    } else {
      page.replace("{{ERROR_MESSAGE}}", "");
    }

    // Access-Point aktivieren und Infos auf Display darstellen
    if (wifiAvailable) {
      myIP = WiFi.softAPIP();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WiFi aktiv!");
      display.print("SSID: ");
      display.println(ssid);
      display.print("IP: ");
      display.println(myIP);
      display.println("\nWarte auf Verbindung");
      display.println("...");
      display.display();

      server.begin();

      // Endlosschleife solange der Schalter aktiv ist
      while (digitalRead(WLAN_SWITCH_PIN) == HIGH) {
        // Warte auf eingehende Client-Verbindung
        WiFiClient client = server.available();

        // Sende Webpage bei eigehender Verbindung
        if (client) {
          currentTime = millis();
          previousTime = currentTime;
          String currentLine = "";
          header = "";

          // Timeout beachten
          while (client.connected() && currentTime - previousTime <= timeoutTime) {
            currentTime = millis();
            if (client.available()) {
              char c = client.read();
              header += c;

              if (c == '\n') {
                if (currentLine.length() == 0) {

                  // Download-Seite über Button aufgerufen
                  if (header.indexOf("GET /download") >= 0) {
                    if (!file) {
                      client.println("HTTP/1.1 503 Service Unavailable");
                      client.println("Content-Type: text/plain");
                      client.println("Connection: close");
                      client.println();
                      client.println("SD-Karte nicht verfügbar.");
                    } else {
                      // Sicherstellen, dass alles geschrieben ist
                      file.flush();
                      file.close();

                      File download = SD.open(FILENAME, FILE_READ);
                      if (download) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/csv");
                        client.print("Content-Disposition: attachment; filename=\"");
                        client.print(FILENAME);
                        client.println("\"");
                        client.println("Connection: close");
                        client.println();

                        // Datei blockweise übertragen
                        uint8_t buf[64];
                        size_t len;
                        while ((len = download.read(buf, sizeof(buf))) > 0) {
                          client.write(buf, len);
                        }

                        download.close();

                        // Datei wieder für Logging im Append-Modus öffnen
                        file = SD.open(FILENAME, FILE_APPEND);
                      } else {
                        client.println("HTTP/1.1 404 Not Found");
                        client.println("Content-Type: text/plain");
                        client.println("Connection: close");
                        client.println();
                        client.println("CSV-Datei nicht gefunden.");
                      }
                    }
                  }

                  // normale HTML-Seite (mit Button zum Download)
                  else {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println("Connection: close");
                    client.println();
                    client.print(page);
                  }

                  client.println();  // HTTP-Antwort beenden
                  client.flush();    // sicherstellen, dass alles gesendet ist
                  delay(20);         // minimale Pause für saubere Übertragung
                  client.stop();
                  Serial.println("Client getrennt");
                  break;
                } else {
                  currentLine = "";
                }
              } else if (c != '\r') {
                currentLine += c;
              }
            }
          }
        }

        delay(100);  // entlastet CPU
      }
      // AP trennen, wenn Schalter umgelegt wird
      WiFi.softAPdisconnect(true);  
      server.end();
      Serial.println("WiFi deaktiviert.");
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("WiFi konnte nicht aktiviert werden!");
      display.display();
      while (digitalRead(WLAN_SWITCH_PIN) == HIGH) {
        delay(500);
      }
    }
  }

  /* BME280 -------------------------------*/
  messung.altitude = bme.readAltitude();
  messung.pressure = bme.readPressure();
  messung.humidity = bme.readHumidity();
  messung.temperature = bme.readTemperature();

  /* Air530 -------------------------------*/
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());  // Daten aus seriellem Buffer lesen
  }
  if (gps.location.isUpdated()) {
    messung.latitude = gps.location.lat();
    messung.longitude = gps.location.lng();
    messung.speed = gps.speed.kmph();
    messung.gps_altitude = gps.altitude.meters();
    messung.satellites = gps.satellites.value();
  }

  /* DS1307 -------------------------------*/
  rtc.getTime();
  messung.hour = rtc.hour;
  messung.minute = rtc.minute;
  messung.second = rtc.second;
  messung.year = rtc.year;
  messung.month = rtc.month;
  messung.day = rtc.dayOfMonth;

  /* GUVA-S12SD ---------------------------*/
  float mV = uv.read();
  messung.uv_index = uv.index(mV);

  /* SSD1306 OLED -------------------------*/
  display.clearDisplay();
  display.setCursor(0, 0);

  if (messung.day < 10) { display.print(0); }
  display.print(messung.day);
  display.print(".");
  if (messung.month < 10) { display.print(0); }
  display.print(messung.month);
  display.print(".");
  display.print(messung.year + 2000);
  display.print("   ");

  if (messung.hour < 10) { display.print(0); }
  display.print(messung.hour);
  display.print(":");
  if (messung.minute < 10) { display.print(0); }
  display.print(messung.minute);
  display.print(":");
  if (messung.second < 10) { display.print(0); }
  display.println(messung.second);
  display.println();

  display.print("GPS(");
  display.print(messung.satellites);
  display.print("): ");
  display.print(messung.latitude);
  display.print(" / ");
  display.println(messung.longitude);

  // Ohne Nachkommastellen
  display.print("Hoehe: ");
  display.print((int)messung.altitude);
  display.print(" / ");
  display.print((int)messung.gps_altitude);
  display.println("m");

  display.print("Geschw.: ");
  display.print(messung.speed);
  display.println("km/h");
  display.println();

  display.print("T: ");
  display.print(messung.temperature);
  display.print("C ");

  display.print(" H: ");
  display.print(messung.humidity);
  display.println("%");

  display.print("P: ");
  display.print(messung.pressure);
  display.print("hPa");

  display.print(" UV: ");
  // Wert aufrunden
  if (messung.uv_index > 0) {
    display.println((int)messung.uv_index + 1);
  } else {
    display.println(0);
  }

  /* Display-Anzeige aktualisieren */
  display.display();

  /* HW-125 SD ----------------------------*/
  if (file) {
    messungSpeichern();  // Messdaten auf SD-Karte speichern
  }

  /* Delay --------------------------------*/
  delay(MESSINTERVALL);
}

// Funktion zum setzen der Uhrzeit + Datum auf der RTC
void setTimeAndDate() {
  rtc.fillByYMD(2025, 7, 18);  // 18. Juli 2025
  rtc.fillByHMS(15, 34, 1);    // 15:34:01 Uhr
  rtc.fillDayOfWeek(5);        // Freitag
  rtc.setTime();               // Zeit auf RTC Chip schreiben
}

/* Funktion zum Einrichten der SD-Karte */
/* Gibt entweder eine geöffnete Datei oder ein invalides File zurück */
File setupSDCard() {
  /* Prüfen ob SD-Karte vorhanden ist */
  if (!SD.begin(SD_PIN)) {
    display.clearDisplay();
    display.println("No SD-Card found!");
    display.display();
    delay(10000);
    return File();
  }

  /* Prüfen, ob Ornder bereits vorhanden ist, wenn nicht wird er erstellt */
  if (!SD.exists(FOLDER)) {
    if (!SD.mkdir(FOLDER)) {
      display.clearDisplay();
      display.println("Directory Error!");
      display.display();
      delay(10000);
      return File();
    }
  }

  /* Prüfen, ob Messdatei bereits vorhanden ist, wenn nicht wird sie erstellt */
  if (!SD.exists(FILENAME)) {
    /* Datei erstellen und öffnen, Write Mode (überschreiben) weil Datei noch nicht existiert */
    File temp_file = SD.open(FILENAME, FILE_WRITE);
    if (temp_file) {
      temp_file.print("Datum;");
      temp_file.print("Uhrzeit;");
      temp_file.print("Satelliten;");
      temp_file.print("Latitude;");
      temp_file.print("Longitude;");
      temp_file.print("Hoehe (BME);");
      temp_file.print("Hoehe (GPS);");
      temp_file.print("Geschwindigkeit;");
      temp_file.print("Temperatur;");
      temp_file.print("Luftfeuchtigkeit;");
      temp_file.print("Luftdruck;");
      temp_file.println("UV-Index");
      temp_file.flush();  // Änderungen zwischenspeichern
      return temp_file;
    } else {
      display.clearDisplay();
      display.println("File Error!");
      display.display();
      delay(10000);
      return File();
    }
  } else {
    /* Datei öffnen, Append Mode weil Datei bereits existiert */
    /* Append-Mode laut offizieller Doku nicht unterstützt, geht aber trotzdem hier -> beobachten! */
    File temp_file = SD.open(FILENAME, FILE_APPEND);
    if (temp_file) {
      return temp_file;
    } else {
      display.clearDisplay();
      display.println("File Error!");
      display.display();
      delay(10000);
      return File();
    }
  }
}

/* Funktion zum Speichern der Messwerte */
/* Schreibt Messwerte nacheinander in das geöffnete File im CSV-Format */
void messungSpeichern() {
  file.print(messung.year + 2000);
  if (messung.month < 10) { file.print(0); }
  file.print(messung.month);
  if (messung.day < 10) { file.print(0); }
  file.print(messung.day);
  file.print(";");
  if (messung.hour < 10) { file.print(0); }
  file.print(messung.hour);
  if (messung.minute < 10) { file.print(0); }
  file.print(messung.minute);
  if (messung.second < 10) { file.print(0); }
  file.print(messung.second);
  file.print(";");
  file.print(messung.satellites);
  file.print(";");
  file.print(messung.latitude, 6); // Mit 6 Nachkommastellen speichern, wichtig!
  file.print(";");
  file.print(messung.longitude, 6); // Mit 6 Nachkommastellen speichern, wichtig!
  file.print(";");
  file.print(messung.altitude);
  file.print(";");
  file.print(messung.gps_altitude);
  file.print(";");
  file.print(messung.speed);
  file.print(";");
  file.print(messung.temperature);
  file.print(";");
  file.print(messung.humidity);
  file.print(";");
  file.print(messung.pressure);
  file.print(";");
  file.println(messung.uv_index, 6);
  file.flush();  // Änderungen zwischenspeichern
}