# Projektbeschreibung
Im Rahmen der Projektarbeit "Lemgoer Stadtklima" wurde ein mobiles Klimamessgerät, basierend auf einem ESP32 zur
Erfassung von Temperatur, Luftfeuchtigkeit und GPS-Daten entwickelt.

Der für diesen ESP32 entwickelte Quellcode wird über dieses Repository bereitgestellt.

# Messgerät
Das finale Messgerät besteht aus den folgenden Komponenten:
- ESP32-WROOM-32E Mikrocontroller
- GY-BME280 Barometrischer Sensor 
- Breakout-Board für MicroSD-Karten 
- Grove DS1307 Echtzeituhr (RTC) 
- Adafruit monochromes 1.3" Display (128x64 OLED-Pixel) 
- GUVA-S12SD UV-Sensor 
- Grove Air530 GPS-Sensor 
- Steckboard („Breadboard“)
- TP4056 Batterieladungsmodul und Schutzschaltung (USB Typ C) 
- Samsung INR18650-30Q Batterie (3,7V; 3000mAh) 
- Batteriehalter für 18650 Zelle mit Anschluss (optional) 
- 3-Pin-Kipp- oder Schiebeschalter (Zwei Stück) 
- Jumper-Kabel (mindestens 36 Stück) 
- Gehäuse (3D-gedruckt)

Eine ausführliche Anleitung zum Aufbau des Messgerätes befindet sich in dem beigenfügten Dokument `Aufbau Klimamessgerät.pdf`.

Auf die genaue Struktur sowie den Aufbau des Gehäuses wird in dieser Dokumentation nicht genauer eingegangen.

# Funktion
Das Messgerät liest, sofern alle Geräte korrekt initialisiert wurden und die SD-Karte erkannt wurde, regelmäßig Daten von sämtlichen Sensoren aus und speichert diese in einer vordefinierten Struktur.

Nach jeder Messung werden die Daten aus dieser Struktur im CSV-Format auf die SD-Karte geschrieben. Die CSV-Datei sowie das Verzeichnis zum Speichern wird, falls noch nicht vorhanden, automatisch erstellt.

Die Struktur zur Messung ist wie folgt aufgebaut:
```c
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
```
Das CSV-Format wird für die Speicherung minimal angepasst. Die Daten werden in folgender Reihenfolge, getrennt durch `;` gespeichert:
- Datum
- Uhrzeit
- Anzahl Satelliten (GPS)
- Längengrad
- Breitengrad
- Höhe (BME280)
- Höhe (GPS)
- Geschwindigkeit
- Temperatur
- Luftfeuchtigkeit
- Luftdruck
- UV-Index 

Über den Schalter für den WiFi-Access-Point aktiviert sich der interne WiFi-Chip des ESP32. Um durch die Frequenzen des Access-Points keine fehlerhaften Messwerte zu verursachen, wird die Messung währenddessen pausiert.

Mit z.B. einem Smartphone kann eine Verbindung zum Access-Point hergestellt werden. Über die auf dem Display des Messgerätes dargestellte IP-Adresse kann eine HTML-Seite auf dem ESP32 aufgerufen werden, über die die CSV-Datei direkt heruntergeladen werden kann. So können die Messwerte ausgelesen werden, ohne jedes Mal die SD-Karte ausbauen zu müssen.

Sämtliche Messwerte sowie der Status des Messgeräts werden über den gesamten aktiven Zeitraum hinweg auf dem OLED-Display dargestellt.

# Benötigte Software / Bibliotheken
## IDE
Für Entwicklung, Debugging und das Kompilieren des Quellcodes wurde die von Arduino bereitgestellte *Arduino IDE* verwendet, da diese neben den Arduino-Mikrocontrollern auch die ESP32-Geräte von Espressif unterstützt.

Die IDE kann direkt von der [Arduino Homepage](https://www.arduino.cc/en/software/) heruntergeladen werden.

## Treiber (Windows)
Damit die serielle UART-Schnittstelle des Mikrocontrollers korrekt verwendet werden kann, muss der `CP210x USB to UART`-Treiber installiert werden. 

Der Treiber kann z.B. von [Silicon Labs](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads) heruntergeladen und anschließend installiert werden. Im Rahmen dieses Projektes wurde für eine Installation auf Windows 11 das Paket `CP210x Windows Drivers` ausgewählt.

Nach der Installation sollte im Geräte-Manager der ESP32 als `Silicon Labs CP210x USB to UART Bridge (COMX)` erkannt werden.

## ESP32 in der IDE einrichten
Zunächst muss in der Arduino IDE die Bibliothek zur Nutzung von ESP32-Mikrocontrollern installiert werden.

Über `Tools` → `Board` → `Boards Manager` oder über ein Symbol in der linken Seitenleiste kann der *Boards-Manager* geöffnet werden. Dort muss nach dem `ESP32` gesucht und das dazugehörige Paket von *Espressif Systems* installiert werden.

Nach der Installation bietet die *Board-Auswahl* eine Vielzahl von ESP32-Boards an. Für dieses Projekt wurde der Typ `ESP32 Dev Module` eingesetzt.

## Bibliotheken
Für die Nutzung der verschiedenen Komponenten müssen diverse Bibliotheken zum korrekten Auslesen der Messwerte installiert werden. Fast alle Bibliotheken lassen sich direkt über die Arduino IDE installieren.

Der `Library Manager` der Arduino IDE kann über die linke Seitenleiste aufgerufen werden, dort müssen die folgenden Bibliotheken gesucht und installiert werden:
- `SD` von Arduino und SparkFun
- `Soldered BME280 and BME680 EasyC Library` von Adafruit
- `Adafruit SSD1306` von Adafruit
- `Adafruit GFX Library` von Adafruit
- `Grove - RTC DS1307` von Seeed Studio
- `TinyGPSPlus` von Mikal Hart

Lediglich die Bibliothek für den *GUVA-S12SD* UV-Sensor muss manuell über das [GitHub Repository](https://github.com/ma2shita/GUVA-S12SD) von [ma2shita](https://github.com/ma2shita) heruntergeladen und anschließend in das lokale Verzeichnis der IDE verschoben werden.

Unter Windows liegt das Verzeichnis für Bibliotheken der Arduino IDE standardmäßig im Benutzerverzeichnis unter `Dokumente` → `Arduino` → `libraries`. Dort befinden sich auch die zuvor in der IDE installierten Bibliotheken.

# Installation & Upload
Der Quellcode aus diesem Repository kann direkt in der Arduino IDE kompiliert werden.

Neben dem finalen Quellcode des Messgeräts im Ordner `klimamessgeraet` befinden sich im Verzeichnis `SensorSamples` Beispiel-Quellcodes für die Nutzung der einzelnen Komponenten und Sensoren des Messgeräts.

Nachdem der korrekte Mikrocontroller und Anschluss in der IDE ausgewählt wurde, kann der Quellcode über die *Upload*-Schaltfläche auf den ESP32 geladen und ausgeführt werden.

# Inbetriebnahme / Nutzung
Nach dem Upload muss das Messgerät vom USB-Anschluss getrennt werden. Erst danach kann es über die Batterie mit Strom versorgt und mobil betrieben werden.

Die Stromversorgung über die Batterie und den USB-Anschluss des ESP32 darf **niemals gleichzeitig aktiv** sein! Dies kann den Controller und die anderen Komponenten beschädigen oder sogar zerstören.

# Autor
Erstellt im Rahmen des Projektes `Lemgoer Statdklima` an der [TH OWL](https://www.th-owl.de/).

**Kontakt:**\
Mika Stellbrink\
mika.stellbrink@stud.th-owl.de