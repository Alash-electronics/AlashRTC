// [ru] AlashRTC: журнал температуры. Каждые 10 секунд печатается строка "дата время;температура" (формат CSV для таблицы).
//      Температура берётся из самих часов DS3231 (точность около 0.25 градуса).
// [kk] AlashRTC: температура журналы. Әр 10 секунд сайын "күні уақыты;температура" жолы басылады (кестеге арналған CSV пішімі).
//      Температура DS3231 сағатының өзінен алынады (дәлдігі шамамен 0.25 градус).
// [en] AlashRTC: a temperature log. Every 10 seconds a line "date time;temperature" is printed (CSV format for a spreadsheet).
//      The temperature comes from the DS3231 clock itself (accuracy about 0.25 degrees).
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
