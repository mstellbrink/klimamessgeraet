// Sources:
// https://arduiniana.org/libraries/tinygpsplus/
// https://github.com/mikalhart/TinyGPSPlus/tree/master
// https://shopofthings.ch/blog/2020/04/23/esp32-hardware-serial-neu-zuordnen/

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

#define getall 1

#define GPS_RX 16
#define GPS_TX 17

HardwareSerial SerialGPS(2);
TinyGPSPlus gps;


void setup() {
  Serial.begin(9600);

  // Syntax: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
}

void loop() {
  // So lange Zeichen im seriellen GPS-Puffer vorhanden sind
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // Nur wenn gültige Daten empfangen wurden
  if (gps.location.isUpdated()) {
#ifdef getall
    Serial.print("\nLAT= ");
    Serial.println(gps.location.lat(), 6);  // Latitude in degrees (double)
    Serial.print("LONG= ");
    Serial.println(gps.location.lng(), 6);  // Longitude in degrees (double)
    Serial.print("Raw LAT= ");
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.println(gps.location.rawLat().deg);         // Raw latitude in whole degrees
    Serial.println(gps.location.rawLat().billionths);  // ... and billionths (u16/u32)
    Serial.print("Raw LONG= ");
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.println(gps.location.rawLng().deg);         // Raw longitude in whole degrees
    Serial.println(gps.location.rawLng().billionths);  // ... and billionths (u16/u32)
    Serial.print("DATE= ");
    Serial.println(gps.date.value());  // Raw date in DDMMYY format (u32)
    Serial.println(gps.date.year());   // Year (2000+) (u16)
    Serial.println(gps.date.month());  // Month (1-12) (u8)
    Serial.println(gps.date.day());    // Day (1-31) (u8)
    Serial.print("TIME= ");
    Serial.println(gps.time.value());        // Raw time in HHMMSSCC format (u32)
    Serial.println(gps.time.hour());         // Hour (0-23) (u8)
    Serial.println(gps.time.minute());       // Minute (0-59) (u8)
    Serial.println(gps.time.second());       // Second (0-59) (u8)
    Serial.println(gps.time.centisecond());  // 100ths of a second (0-99) (u8)
    Serial.print("SPEED= ");
    Serial.println(gps.speed.value());  // Raw speed in 100ths of a knot (i32)
    Serial.print("SPEED (knots)= ");
    Serial.println(gps.speed.knots());  // Speed in knots (double)
    Serial.print("SPEED (mph)= ");
    Serial.println(gps.speed.mph());  // Speed in miles per hour (double)
    Serial.print("SPEED (mps)= ");
    Serial.println(gps.speed.mps());  // Speed in meters per second (double)
    Serial.print("SPEED (kmh)= ");
    Serial.println(gps.speed.kmph());  // Speed in kilometers per hour (double)
    Serial.print("COURSE= ");
    Serial.println(gps.course.value());  // Raw course in 100ths of a degree (i32)
    Serial.println(gps.course.deg());    // Course in degrees (double)
    Serial.print("ALTITUDE (cm)= ");
    Serial.println(gps.altitude.value());  // Raw altitude in centimeters (i32)
    Serial.print("ALTITUDE (m)= ");
    Serial.println(gps.altitude.meters());  // Altitude in meters (double)
    Serial.print("ALTITUDE (miles)= ");
    Serial.println(gps.altitude.miles());  // Altitude in miles (double)
    Serial.print("ALTITUDE (km)= ");
    Serial.println(gps.altitude.kilometers());  // Altitude in kilometers (double)
    Serial.print("ALTITUDE (ft)= ");
    Serial.println(gps.altitude.feet());  // Altitude in feet (double)
    Serial.print("SATELLITE COUNT= ");
    Serial.println(gps.satellites.value());  // Number of satellites in use (u32)
    Serial.print("PRECISION= ");
    Serial.println(gps.hdop.value());  // Horizontal Dim. of Precision (100ths-i32)
#else
    Serial.print("\nLAT= ");
    Serial.println(gps.location.lat());
    Serial.print("LONG= ");
    Serial.println(gps.location.lng());
    Serial.print("ALT= ");
    Serial.println(gps.altitude.meters());
#endif
  }
  delay(10000);
}
