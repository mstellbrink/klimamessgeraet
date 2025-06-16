// Sources:
// https://github.com/ma2shita/GUVA-S12SD

// Library muss manuell heruntergeladen werden!
#include <GUVA-S12SD.h>

// VP Pin (GPIO36) am ESP32
#define UV_PIN 36
#define WORKING_VOLTAGE 3.3
#define SAMPLING_COUNT 1000

GUVAS12SD uv(UV_PIN, WORKING_VOLTAGE, SAMPLING_COUNT);

void setup() {
  Serial.begin(9600);
}

void loop() {
  float mV = uv.read();
  Serial.println(mV);
  float uv_index = uv.index(mV);
  Serial.println(uv_index);
  delay(1000);
}