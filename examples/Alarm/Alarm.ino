// AlashRTC: будильник DS3231. Раз в сутки в заданное время часы поднимают флаг, и скетч включает светодиод.
// Только DS3231 (у DS1307 будильника нет). Флаг читается опросом, провод на пин INT/SQW не нужен.
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
