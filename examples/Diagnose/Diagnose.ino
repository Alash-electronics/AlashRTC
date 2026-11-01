// AlashRTC: часы не работают или врут? Скетч покажет тип часов, время, температуру и подскажет, что проверить.
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
