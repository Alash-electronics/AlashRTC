// [ru] AlashRTC: установка времени. Три способа: по времени компиляции (самый простой), вручную и по секундам unix.
//      Загрузите скетч сразу после компиляции: время компиляции будет отставать на время загрузки.
// [kk] AlashRTC: уақытты орнату. Үш тәсіл: компиляция уақыты бойынша (ең қарапайым), қолмен және unix секундтары бойынша.
//      Скетчті компиляциядан кейін бірден жүктеңіз: компиляция уақыты жүктеу уақытына кешігеді.
// [en] AlashRTC: setting the time. Three ways: by the compile time (the simplest), manually and by unix seconds.
//      Upload the sketch right after compiling: the compile time lags by the upload time.
#include <AlashRTC.h>

AlashRTC rtc;

void setup() {
  Serial.begin(115200);
  if (!rtc.begin()) {
    Serial.println(rtc.errorText());
    while (true) delay(1000);
  }

  rtc.setFromCompileTime();               // способ 1: время компиляции

  // способ 2: вручную. Год, месяц, день, час, минута, секунда. Неверная дата (например, 30 февраля) вернёт false.
  // if (!rtc.setTime(2026, 11, 2, 14, 30, 0)) Serial.println(rtc.errorText());

  // способ 3: секунды с 1970 года (например, из интернета)
  // rtc.setUnixTime(1793628309);

  Serial.println(F("Время установлено"));
}

void loop() {
  Serial.print(rtc.dayName());            // день недели на выбранном языке
  Serial.print(", ");
  Serial.print(rtc.day());
  Serial.print('.');
  Serial.print(rtc.month());
  Serial.print('.');
  Serial.print(rtc.year());
  Serial.print("  ");
  Serial.print(rtc.hour());
  Serial.print(':');
  Serial.print(rtc.minute());
  Serial.print(':');
  Serial.println(rtc.second());
  delay(1000);
}
