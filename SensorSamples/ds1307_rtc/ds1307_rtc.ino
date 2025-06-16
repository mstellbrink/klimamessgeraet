// Sources:
// https://github.com/Seeed-Studio/RTC_DS1307
// https://wiki.seeedstudio.com/Grove-RTC/

#include <DS1307.h>

// Nur definieren wenn die Zeit einmalig gesetzt werden soll!
// #define setTimeAndDate

DS1307 rtc;

void setup() {
  Serial.begin(9600);

  // Muss vor setzen der Zeit aktiviert sein
  rtc.begin();

  // Einmaliges Setzen der Zeit, alle Werte müssen angegeben werden!
#ifdef setTimeAndDate
  rtc.fillByYMD(2025, 5, 18);  // 18. Mai 2025
  rtc.fillByHMS(18, 19, 1);    // 18:19:01 Uhr
  rtc.fillDayOfWeek(7);        // Sonntag
  rtc.setTime();               // Zeit auf RTC Chip schreiben
#endif
}

void loop() {
  printTime();
  delay(1000);
}

/*Function: Display time on the serial monitor*/
void printTime() {
  rtc.getTime();
  Serial.print(rtc.hour, DEC);
  Serial.print(":");
  Serial.print(rtc.minute, DEC);
  Serial.print(":");
  Serial.print(rtc.second, DEC);
  Serial.print("  ");
  Serial.print(rtc.month, DEC);
  Serial.print("/");
  Serial.print(rtc.dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(rtc.year + 2000, DEC);
  Serial.print(" ");
  Serial.print(rtc.dayOfMonth);
  Serial.print("*");
  switch (rtc.dayOfWeek)  // Friendly printout the weekday
  {
    case MON:
      Serial.print("MON");
      break;
    case TUE:
      Serial.print("TUE");
      break;
    case WED:
      Serial.print("WED");
      break;
    case THU:
      Serial.print("THU");
      break;
    case FRI:
      Serial.print("FRI");
      break;
    case SAT:
      Serial.print("SAT");
      break;
    case SUN:
      Serial.print("SUN");
      break;
  }
  Serial.println(" ");
}