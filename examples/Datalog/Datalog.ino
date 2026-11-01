// AlashRTC: журнал температуры. Каждые 10 секунд печатается строка "дата время;температура" (формат CSV для таблицы).
// Температура берётся из самих часов DS3231 (точность около 0.25 градуса).
#include <AlashRTC.h>

AlashRTC rtc;
uint32_t last = 0;

void setup() {
  Serial.begin(115200);
  if (!rtc.begin()) {
    Serial.println(rtc.errorText());
    while (true) delay(1000);
  }
  if (rtc.lostPower()) rtc.setFromCompileTime();
  Serial.println(F("время;температура"));
}

void loop() {
  if (millis() - last >= 10000UL || last == 0) {
    last = millis();
    char text[24];
    rtc.format(text, sizeof text);
    Serial.print(text);
    Serial.print(';');
    Serial.println(rtc.temperature(), 2);
  }
}
