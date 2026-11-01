// AlashRTC: самый простой пример. Дата и время с часов DS3231 (модуль ZS-042 или Tiny RTC) раз в секунду.
//
// Подключение: VCC к 3.3 В или 5 В, GND к GND, SDA к A4 (ESP32: GPIO21), SCL к A5 (ESP32: GPIO22).
// В модуль нужна батарейка CR2032: без неё часы сбрасываются при выключении питания.
#include <AlashRTC.h>

AlashRTC rtc;

void setup() {
  Serial.begin(115200);
  if (!rtc.begin()) {                     // сам находит часы и их тип (DS3231 или DS1307)
    Serial.println(rtc.errorText());      // объяснит, что проверить
    while (true) delay(1000);
  }
  if (rtc.lostPower()) {                  // первый запуск или села батарейка: время недостоверно
    Serial.println(F("Время сбилось, ставлю время компиляции скетча"));
    rtc.setFromCompileTime();
  }
}

void loop() {
  char text[24];
  rtc.format(text, sizeof text);          // "2026-11-02 14:05:09"
  Serial.println(text);
  delay(1000);
}
