// AlashRTC: часы реального времени DS3231 и DS1307 (модули ZS-042, Tiny RTC): дата, время, температура, будильники.
// Без зависимостей, только встроенная Wire. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashRTC
#ifndef ALASH_RTC_H
#define ALASH_RTC_H

#include <Arduino.h>
#include <Wire.h>

class AlashRTC {
 public:
  enum Chip : uint8_t { CHIP_NONE = 0, CHIP_DS3231, CHIP_DS1307 };
  enum Error : uint8_t {
    ERR_OK = 0,
    ERR_NO_DEVICE,    // на шине I2C нет ответа по адресу 0x68
    ERR_I2C,          // ошибка чтения или записи по I2C
    ERR_BAD_TIME,     // такой даты или времени не бывает
    ERR_NOT_SUPPORTED // DS1307 не умеет температуру и будильники
  };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };

  // ---- Запуск --------------------------------------------------------------
  // begin() находит часы по адресу 0x68 и сам определяет DS3231 или DS1307 (можно указать явно).
  bool begin(TwoWire &wire = Wire, bool startWire = true);
  bool begin(Chip chip, TwoWire &wire = Wire, bool startWire = true);
#if defined(ESP32) || defined(ESP8266)
  bool begin(int sdaPin, int sclPin);
#endif

  // ---- Установка времени ----------------------------------------------------
  bool setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second = 0);
  bool setFromCompileTime();                // время компиляции скетча (самый простой способ первой установки)
  bool setUnixTime(uint32_t seconds);       // секунды с 1970-01-01 (время считается местным: часы ничего не знают о поясах)
  bool lostPower();                         // часы теряли питание: время недостоверно, установите его заново
  bool isRunning();

  // ---- Чтение времени (часы опрашиваются сами) --------------------------------------
  uint16_t year();
  uint8_t month();
  uint8_t day();
  uint8_t hour();
  uint8_t minute();
  uint8_t second();
  uint8_t dayOfWeek();                      // 1 = понедельник ... 7 = воскресенье
  uint16_t dayOfYear();                     // 1..366
  uint32_t unixTime();                      // секунды с 1970-01-01
  void format(char *buf, size_t size);      // "2026-11-02 14:05:09" (нужен буфер минимум 20 байт)
  const __FlashStringHelper *dayName();     // "Понедельник" ... на выбранном языке
  static bool isLeapYear(uint16_t y);
  static uint8_t daysInMonth(uint16_t y, uint8_t m);
  static uint8_t weekday(uint16_t y, uint8_t m, uint8_t d);   // 1 = понедельник

  // ---- Только DS3231 --------------------------------------------------------------
  float temperature();                      // °C, шаг 0.25 (встроенный датчик)
  bool setAlarm(uint8_t hour, uint8_t minute, uint8_t second = 0);   // будильник раз в сутки в это время
  bool alarmFired();                        // будильник сработал (true, пока не вызовете clearAlarm)
  void clearAlarm();
  void disableAlarm();
  bool setSquareWave(uint16_t hz);           // 0 = выкл., 1, 1024, 4096, 8192 Гц (DS1307: 1, 4096, 8192, 32768) на выводе SQW

  // ---- Состояние и диагностика --------------------------------------------------------
  Chip chip() const { return _chip; }
  const __FlashStringHelper *chipName() const;
  bool isConnected();
  Error error() const { return _err; }
  void setLanguage(Language lang) { _lang = lang; }
  const __FlashStringHelper *errorText();
  void diagnose(Print &out);
  void update();                            // прочитать часы сейчас (getters делают это сами раз в 100 мс)

 private:
  TwoWire *_wire = &Wire;
  Chip _chip = CHIP_NONE;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;
  uint16_t _y = 2000;
  uint8_t _mo = 1, _d = 1, _h = 0, _mi = 0, _s = 0;
  bool _halted = false, _osf = false, _hasData = false;
  uint32_t _readAt = 0;

  bool probe();
  Chip detect();
  bool readBytes(uint8_t reg, uint8_t *buf, uint8_t len);
  bool writeBytes(uint8_t reg, const uint8_t *buf, uint8_t len);
  bool write8(uint8_t reg, uint8_t v);
  bool read8(uint8_t reg, uint8_t &v);
  void ensureFresh();
  static uint8_t bcd2bin(uint8_t v) { return (v & 0x0F) + 10 * (v >> 4); }
  static uint8_t bin2bcd(uint8_t v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }
  static int32_t daysFromCivil(int y, unsigned m, unsigned d);
};

#endif  // ALASH_RTC_H
