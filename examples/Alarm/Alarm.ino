// [ru] AlashRTC: будильник DS3231. Раз в сутки в заданное время часы поднимают флаг, и скетч включает светодиод.
//      Только DS3231 (у DS1307 будильника нет). Флаг читается опросом, провод на пин INT/SQW не нужен.
// [kk] AlashRTC: DS3231 оятқышы. Тәулігіне бір рет берілген уақытта сағат жалауды көтереді, ал скетч жарықдиодты қосады.
//      Тек DS3231 (DS1307-де оятқыш жоқ). Жалау сұрау арқылы оқылады, INT/SQW пиніне сым керек емес.
// [en] AlashRTC: a DS3231 alarm. Once a day at the set time the clock raises a flag, and the sketch switches an LED on.
//      DS3231 only (the DS1307 has no alarm). The flag is read by polling, no wire to the INT/SQW pin is needed.
#include <AlashRTC.h>

AlashRTC rtc;
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  if (!rtc.begin()) {
    Serial.println(rtc.errorText());
    while (true) delay(1000);
  }
  if (rtc.lostPower()) rtc.setFromCompileTime();
  // Будильник на 1 минуту позже текущего времени, чтобы сразу увидеть, как работает
  uint8_t m = rtc.minute() + 1, h = rtc.hour();
  if (m > 59) { m = 0; h = (h + 1) % 24; }
  if (!rtc.setAlarm(h, m, 0)) {
    Serial.println(rtc.errorText());
    while (true) delay(1000);
  }
  Serial.print(F("Будильник на "));
  Serial.print(h);
  Serial.print(':');
  Serial.println(m);
}

void loop() {
  if (rtc.alarmFired()) {
    Serial.println(F("Подъём!"));
    digitalWrite(LED_BUILTIN, HIGH);
    rtc.clearAlarm();                     // сбросить флаг, иначе он останется
  }
  delay(200);
}
