// Sources:
// https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi
// https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html
// https://randomnerdtutorials.com/esp32-web-server-arduino-ide/

#include <WiFi.h>
//#include <NetworkClient.h>
//#include <WiFiAP.h>

// Wifi Zugangsdaten
const char *ssid = "ESP32AP";
const char *password = "Password123";

// Server auf Port 80 erstellen
WiFiServer server(80);
// Variable für IP-Adresse
IPAddress myIP;

// Variable für HTTP-Requests
String header;

// Zeit-Variablen für Timeout
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

// HTML-Text für Webpage
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
  <p>CSV-Datei herunterladen oder Seite neu laden:</p>
  <button onclick="window.location.href='/download'">CSV herunterladen</button>
  <button onclick="window.location.href='/'">Zur Startseite</button>
  {{ERROR_MESSAGE}}
</body>
</html>
)=====";

String page = MAIN_page;
bool sdAvailable = false;  // später von SD.begin() gesetzt

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.println("Access Point wird konfiguriert...");

  // Access Point mit Zugangsdaten erstellen
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Access Point konnte nicht erstellt werden!");
    while (1)
      ;
  }

  // IP-Adresse zum späteren Aufrufen der Webpage
  myIP = WiFi.softAPIP();
  Serial.print("AP IP-Adresse: ");
  Serial.println(myIP);

  // Server starten
  server.begin();
  Serial.println("Server gestartet!");
}

void loop() {
  // Server hört Port 80 auf Client-Verbindungen ab
  WiFiClient client = server.available();

  // Wenn sich ein Client verbindet
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("Client verbunden...");

    // String für eingehende Daten
    String currentLine = "";

    // Fehlermeldung einblenden durch zu ersetzende Parameter
    if (!sdAvailable) {
      page.replace("{{ERROR_MESSAGE}}", "<p class='error'>⚠️ SD-Karte nicht erkannt!</p>");
    } else {
      page.replace("{{ERROR_MESSAGE}}", "");
    }

    // Loop solange der Client verbunden ist
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      // Wenn eingehende Daten verfügbar sind
      if (client.available()) {
        // Byte lesen und im Serial Monitor ausgeben
        char c = client.read();
        Serial.write(c);
        header += c;

        // Newline Character erkannt
        if (c == '\n') {
          // Wenn die aktuelle Zeile leer ist (= 2 Newlines in Folge), ist der HTTP-Request fertig
          if (currentLine.length() == 0) {
            // HTTP header starten immer mit einem Response Code (z.B. HTTP/1.1 200 OK)
            client.println("HTTP/1.1 200 OK");
            // und einem content-type damit der Client weiß was kommt, hier HTML-Text
            client.println("Content-type:text/html");
            client.println("Connection: close");
            // Dann noch eine leere Zeile
            client.println();

            // HTML-Webpage darstellen, auf einmal senden
            client.print(page);

            // Die HTTP-Antwort endet mit einem weiteren Newline-Character
            client.println();

            break;
          } else {
            // Wenn es die erste Newline ist, setze Nachrichtenpuffer zurück
            currentLine = "";
          }
        } else if (c != '\r') {
          // Alle anderen Zeichen in den Nachrichtenpuffer
          currentLine += c;
        }
      }
    }
  }
}
