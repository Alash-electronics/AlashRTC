// [ru] AlashRTC: часы не работают или врут? Скетч покажет тип часов, время, температуру и подскажет, что проверить.
// [kk] AlashRTC: сағат жұмыс істемей ме немесе өтірік айта ма? Скетч сағат түрін, уақытты, температураны көрсетеді және нені тексеру керегін айтады.
// [en] AlashRTC: the clock does not work or lies? The sketch shows the clock type, the time, the temperature and tells what to check.
#include <AlashRTC.h>

AlashRTC rtc;

void setup() {
  Serial.begin(115200);
  delay(500);
  rtc.setLanguage(AlashRTC::LANG_RU);     // LANG_KK: қазақша, LANG_EN: English
  rtc.begin();
  rtc.diagnose(Serial);
}

void loop() {}
