// [ru] AlashRTC: самый простой пример. Дата и время с часов DS3231 (модуль ZS-042 или Tiny RTC) раз в секунду.
//
//      Подключение: VCC к 3.3 В или 5 В, GND к GND, SDA к A4 (ESP32: GPIO21), SCL к A5 (ESP32: GPIO22).
//      В модуль нужна батарейка CR2032: без неё часы сбрасываются при выключении питания.
// [kk] AlashRTC: ең қарапайым мысал. DS3231 сағатынан (ZS-042 модулі немесе Tiny RTC) күн мен уақыт секунд сайын.
//
//      Қосу: VCC 3.3 В немесе 5 В-қа, GND GND-ға, SDA A4-ке (ESP32: GPIO21), SCL A5-ке (ESP32: GPIO22).
//      Модульге CR2032 батареясы керек: онсыз қуат өшкенде сағат нөлденеді.
// [en] AlashRTC: the simplest example. The date and time from a DS3231 clock (ZS-042 module or Tiny RTC) once a second.
//
//      Wiring: VCC to 3.3 V or 5 V, GND to GND, SDA to A4 (ESP32: GPIO21), SCL to A5 (ESP32: GPIO22).
//      The module needs a CR2032 battery: without it the clock resets when the power is off.
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
