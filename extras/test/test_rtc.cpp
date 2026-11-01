// Тесты AlashRTC без железа (имитация регистров DS3231 / DS1307):
//   g++ -std=c++17 -Wall -I mock -I ../../src test_rtc.cpp ../../src/AlashRTC.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "Wire.h"
#include "AlashRTC.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static bool near(float a, float b, float e) { return fabsf(a - b) <= e; }
static void reset(bool is3231 = true) { g_rtc = FakeRtc(); g_rtc.init(is3231); g_ms += 1000; }

int main() {
  // 1) DS3231: определение, часы стояли (OSF), установка, чтение, регистры в BCD
  reset(); AlashRTC r;
  CHECK(r.begin() && r.chip() == AlashRTC::CHIP_DS3231);
  CHECK(r.lostPower() && !r.isRunning());
  CHECK(r.setTime(2026, 11, 2, 14, 5, 9));
  CHECK(!r.lostPower() && r.isRunning());
  CHECK(g_rtc.reg[0] == 0x09 && g_rtc.reg[1] == 0x05 && g_rtc.reg[2] == 0x14 && g_rtc.reg[3] == 1 && g_rtc.reg[4] == 0x02 && g_rtc.reg[5] == 0x11 && g_rtc.reg[6] == 0x26);
  CHECK(r.year() == 2026 && r.month() == 11 && r.day() == 2 && r.hour() == 14 && r.minute() == 5 && r.second() == 9);
  char buf[24]; r.format(buf, sizeof buf);
  CHECK(strcmp(buf, "2026-11-02 14:05:09") == 0);

  // 2) день недели и день года
  CHECK(AlashRTC::weekday(2026, 11, 2) == 1);
  CHECK(AlashRTC::weekday(2000, 2, 29) == 2);
  CHECK(AlashRTC::weekday(2024, 2, 29) == 4);
  CHECK(AlashRTC::weekday(1970, 1, 1) == 4);
  CHECK(AlashRTC::weekday(2099, 12, 31) == 4);
  CHECK(r.dayOfWeek() == 1 && r.dayOfYear() == 306);
  r.setTime(2024, 3, 1, 0, 0, 0); g_ms += 200;
  CHECK(r.dayOfYear() == 61 && r.dayOfWeek() == 5);
  CHECK(AlashRTC::isLeapYear(2000) && !AlashRTC::isLeapYear(2100) && AlashRTC::isLeapYear(2024) && !AlashRTC::isLeapYear(2023));
  CHECK(AlashRTC::daysInMonth(2024, 2) == 29 && AlashRTC::daysInMonth(2023, 2) == 28 && AlashRTC::daysInMonth(2026, 4) == 30 && AlashRTC::daysInMonth(2026, 13) == 0);

  // 3) unix-время в обе стороны
  r.setTime(2026, 11, 2, 14, 5, 9); g_ms += 200;
  CHECK(r.unixTime() == 1793628309UL);
  CHECK(r.setUnixTime(1793628309UL)); g_ms += 200;
  CHECK(r.year() == 2026 && r.month() == 11 && r.day() == 2 && r.hour() == 14 && r.minute() == 5 && r.second() == 9);
  CHECK(r.setUnixTime(4102444799UL)); g_ms += 200;
  CHECK(r.year() == 2099 && r.month() == 12 && r.day() == 31 && r.hour() == 23 && r.second() == 59);
  CHECK(r.setUnixTime(951782400UL)); g_ms += 200;                        // 2000-02-29 00:00:00
  CHECK(r.year() == 2000 && r.month() == 2 && r.day() == 29);
  CHECK(!r.setUnixTime(86400UL) && r.error() == AlashRTC::ERR_BAD_TIME);  // 1970 раньше 2000

  // 4) неверные даты не записываются и не портят часы
  r.setTime(2026, 11, 2, 14, 5, 9); g_ms += 200;
  CHECK(!r.setTime(2026, 2, 30, 1, 1, 1) && r.error() == AlashRTC::ERR_BAD_TIME);
  CHECK(!r.setTime(2023, 2, 29, 1, 1, 1));
  CHECK(r.setTime(2024, 2, 29, 1, 1, 1));
  CHECK(!r.setTime(2026, 13, 1, 0, 0, 0) && !r.setTime(2026, 1, 1, 24, 0, 0) && !r.setTime(2026, 1, 1, 0, 60, 0) && !r.setTime(1999, 1, 1, 0, 0, 0) && !r.setTime(2100, 1, 1, 0, 0, 0));
  g_ms += 200; CHECK(r.year() == 2024 && r.day() == 29);

  // 5) температура DS3231: 25.25 и отрицательная
  CHECK(near(r.temperature(), 25.25f, 0.001f));
  g_rtc.reg[0x11] = 0xFF; g_rtc.reg[0x12] = 0xC0;                       // -1 + 0.75
  CHECK(near(r.temperature(), -0.25f, 0.001f));
  g_rtc.reg[0x11] = 0xE7; g_rtc.reg[0x12] = 0x00;                       // -25
  CHECK(near(r.temperature(), -25.0f, 0.001f));

  // 6) будильник DS3231
  CHECK(r.setAlarm(7, 30, 0));
  CHECK(g_rtc.reg[7] == 0x00 && g_rtc.reg[8] == 0x30 && g_rtc.reg[9] == 0x07 && g_rtc.reg[10] == 0x80);
  CHECK((g_rtc.reg[0x0E] & 0x05) == 0x05);                                // A1IE и INTCN
  CHECK(!r.alarmFired());
  g_rtc.reg[0x0F] |= 0x01;
  CHECK(r.alarmFired());
  r.clearAlarm(); CHECK(!r.alarmFired());
  CHECK(!r.setAlarm(24, 0) && r.error() == AlashRTC::ERR_BAD_TIME);
  r.disableAlarm(); CHECK((g_rtc.reg[0x0E] & 0x03) == 0);
  CHECK(r.setSquareWave(1024) && (g_rtc.reg[0x0E] & 0x1C) == 0x08);
  CHECK(!r.setSquareWave(77));

  // 7) DS1307: определение, часы стоят (CH), установка снимает CH, температуры и будильников нет
  reset(false); AlashRTC s;
  CHECK(s.begin() && s.chip() == AlashRTC::CHIP_DS1307);
  CHECK(s.lostPower());
  CHECK(s.setTime(2025, 1, 5, 8, 15, 30));
  CHECK(!s.lostPower() && (g_rtc.reg[0] & 0x80) == 0);
  g_ms += 200; CHECK(s.hour() == 8 && s.minute() == 15 && s.second() == 30);
  CHECK(isnan(s.temperature()) && s.error() == AlashRTC::ERR_NOT_SUPPORTED);
  CHECK(!s.setAlarm(1, 1) && !s.alarmFired());
  CHECK(s.setSquareWave(4096) && g_rtc.reg[7] == 0x11);

  // 8) 12-часовой режим: 11 PM = 23, 12 AM = 0, 12 PM = 12, 1 AM = 1
  reset(); AlashRTC h; h.begin(); h.setTime(2026, 1, 1, 0, 0, 0);
  g_rtc.reg[2] = 0x40 | 0x20 | 0x11; g_ms += 200; CHECK(h.hour() == 23);
  g_rtc.reg[2] = 0x40 | 0x12;        g_ms += 200; CHECK(h.hour() == 0);
  g_rtc.reg[2] = 0x40 | 0x20 | 0x12; g_ms += 200; CHECK(h.hour() == 12);
  g_rtc.reg[2] = 0x40 | 0x01;        g_ms += 200; CHECK(h.hour() == 1);

  // 9) время компиляции
  reset(); AlashRTC c; c.begin();
  CHECK(c.setFromCompileTime());
  g_ms += 200;
  char mon[4]; int dd, yy, hh, mm, ss;
  sscanf(__DATE__, "%3s %d %d", mon, &dd, &yy); sscanf(__TIME__, "%d:%d:%d", &hh, &mm, &ss);
  CHECK(c.year() == yy && c.day() == dd && c.hour() == hh && c.minute() == mm && c.second() == ss && c.month() >= 1 && c.month() <= 12);

  // 10) чтение кэшируется на 100 мс, потом обновляется
  reset(); AlashRTC k; k.begin(); k.setTime(2026, 5, 5, 10, 0, 0);
  g_ms += 200; CHECK(k.second() == 0);
  g_rtc.reg[0] = 0x07;
  g_ms += 50;  CHECK(k.second() == 0);
  g_ms += 60;  CHECK(k.second() == 7);

  // 11) нет часов, явный выбор микросхемы, потеря связи
  reset(); g_rtc.present = false; AlashRTC n;
  CHECK(!n.begin() && n.error() == AlashRTC::ERR_NO_DEVICE && !n.isConnected());
  reset(); AlashRTC e; CHECK(e.begin(AlashRTC::CHIP_DS3231) && e.chip() == AlashRTC::CHIP_DS3231);
  g_rtc.present = false; g_ms += 200; e.update(); CHECK(e.error() == AlashRTC::ERR_I2C);

  // 12) названия дней и сообщения на трёх языках
  reset(); AlashRTC l; l.begin(); l.setTime(2026, 11, 2, 1, 1, 1); g_ms += 200;
  l.setLanguage(AlashRTC::LANG_RU); CHECK(strcmp((const char *)l.dayName(), "Понедельник") == 0);
  l.setLanguage(AlashRTC::LANG_EN); CHECK(strcmp((const char *)l.dayName(), "Monday") == 0);
  l.setLanguage(AlashRTC::LANG_KK); CHECK(strcmp((const char *)l.dayName(), "Дүйсенбі") == 0);
  printf("--- diagnose ---\n"); g_rtc.reg[0x0F] = 0x88; l.diagnose(*(new Print));

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
