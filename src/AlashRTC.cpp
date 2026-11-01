// MIT License. Alash Electronics.
#include "AlashRTC.h"

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

static const uint8_t ADDR = 0x68;
static const uint8_t REG_TIME = 0x00;         // секунды, минуты, часы, день недели, дата, месяц, год
static const uint8_t REG_ALARM1 = 0x07;       // DS3231: секунды, минуты, часы, день/дата
static const uint8_t REG_CONTROL = 0x0E;      // DS3231
static const uint8_t REG_STATUS = 0x0F;       // DS3231: OSF (бит 7), A2F (бит 1), A1F (бит 0)
static const uint8_t REG_TEMP = 0x11;         // DS3231

// ---- Запуск --------------------------------------------------------------------

bool AlashRTC::begin(TwoWire &wire, bool startWire) {
  _wire = &wire;
  if (startWire) _wire->begin();
  _chip = CHIP_NONE;
  _hasData = false;
  return probe();
}

bool AlashRTC::begin(Chip chip, TwoWire &wire, bool startWire) {
  _wire = &wire;
  if (startWire) _wire->begin();
  _hasData = false;
  _wire->beginTransmission(ADDR);
  if (_wire->endTransmission() != 0) {
    _err = ERR_NO_DEVICE;
    _chip = CHIP_NONE;
    return false;
  }
  _chip = chip;
  _err = ERR_OK;
  update();
  return _err == ERR_OK;
}

#if defined(ESP32) || defined(ESP8266)
bool AlashRTC::begin(int sdaPin, int sclPin) {
  Wire.begin(sdaPin, sclPin);
  return begin(Wire, false);
}
#endif

bool AlashRTC::probe() {
  _wire->beginTransmission(ADDR);
  if (_wire->endTransmission() != 0) {
    _err = ERR_NO_DEVICE;
    return false;
  }
  _chip = detect();
  _err = ERR_OK;
  update();
  return _err == ERR_OK;
}

// У DS3231 по адресу 0x11 лежит температура: старший байт целые градусы, младший только биты 7:6 (0.25 градуса).
// У DS1307 по тем же адресам обычная память, где такого порядка нет.
AlashRTC::Chip AlashRTC::detect() {
  uint8_t t[2], ctl = 0xFF;
  if (!readBytes(REG_TEMP, t, 2) || !read8(REG_CONTROL, ctl)) return CHIP_DS1307;
  bool plausibleTemp = (t[1] & 0x3F) == 0 && (int8_t)t[0] > -40 && (int8_t)t[0] < 85;
  return (plausibleTemp && (ctl & 0x80) == 0) ? CHIP_DS3231 : CHIP_DS1307;
}

bool AlashRTC::isConnected() {
  _wire->beginTransmission(ADDR);
  return _wire->endTransmission() == 0;
}

const __FlashStringHelper *AlashRTC::chipName() const {
  return _chip == CHIP_DS3231 ? F("DS3231") : _chip == CHIP_DS1307 ? F("DS1307") : F("-");
}

// ---- Календарь -------------------------------------------------------------------

bool AlashRTC::isLeapYear(uint16_t y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

uint8_t AlashRTC::daysInMonth(uint16_t y, uint8_t m) {
  static const uint8_t d[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (m < 1 || m > 12) return 0;
  return (m == 2 && isLeapYear(y)) ? 29 : d[m - 1];
}

// Дни от 1970-01-01 (алгоритм Говарда Хиннанта)
int32_t AlashRTC::daysFromCivil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  int era = (y >= 0 ? y : y - 399) / 400;
  unsigned yoe = (unsigned)(y - era * 400);
  unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int32_t)doe - 719468;
}

uint8_t AlashRTC::weekday(uint16_t y, uint8_t m, uint8_t d) {
  int32_t days = daysFromCivil(y, m, d);        // 1970-01-01 был четвергом
  int w = (int)((days % 7 + 7 + 3) % 7);        // 0 = понедельник
  return (uint8_t)(w + 1);
}

// ---- Установка --------------------------------------------------------------------

bool AlashRTC::setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  if (_chip == CHIP_NONE) {
    _err = ERR_NO_DEVICE;
    return false;
  }
  if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > daysInMonth(year, month) || hour > 23 || minute > 59 || second > 59) {
    _err = ERR_BAD_TIME;
    return false;
  }
  uint8_t b[7] = {bin2bcd(second), bin2bcd(minute), bin2bcd(hour), weekday(year, month, day), bin2bcd(day), bin2bcd(month), bin2bcd(year - 2000)};
  if (!writeBytes(REG_TIME, b, 7)) {
    _err = ERR_I2C;
    return false;
  }
  if (_chip == CHIP_DS3231) {
    uint8_t st = 0;
    if (read8(REG_STATUS, st)) write8(REG_STATUS, st & ~0x80);   // сбросить флаг «часы останавливались»
  }
  _err = ERR_OK;
  _hasData = false;
  update();
  return true;
}

bool AlashRTC::setFromCompileTime() {
  // __DATE__ = "Nov  2 2026", __TIME__ = "14:05:09"
  static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char *d = __DATE__;
  uint8_t m = 1;
  for (uint8_t i = 0; i < 12; i++)
    if (strncmp(d, months + 3 * i, 3) == 0) m = i + 1;
  uint8_t day = (d[4] == ' ' ? 0 : (d[4] - '0') * 10) + (d[5] - '0');
  uint16_t y = atoi(d + 7);
  const char *t = __TIME__;
  return setTime(y, m, day, (t[0] - '0') * 10 + (t[1] - '0'), (t[3] - '0') * 10 + (t[4] - '0'), (t[6] - '0') * 10 + (t[7] - '0'));
}

bool AlashRTC::setUnixTime(uint32_t sec) {
  uint32_t days = sec / 86400, rem = sec % 86400;
  // дни -> дата (обратный алгоритм Хиннанта)
  int32_t z = (int32_t)days + 719468;
  int era = (z >= 0 ? z : z - 146096) / 146097;
  unsigned doe = (unsigned)(z - era * 146097);
  unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int y = (int)yoe + era * 400;
  unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  unsigned mp = (5 * doy + 2) / 153;
  unsigned d = doy - (153 * mp + 2) / 5 + 1;
  unsigned m = mp < 10 ? mp + 3 : mp - 9;
  y += m <= 2;
  return setTime(y, m, d, rem / 3600, (rem % 3600) / 60, rem % 60);
}

bool AlashRTC::lostPower() {
  update();
  return _chip == CHIP_DS3231 ? _osf : _halted;
}

bool AlashRTC::isRunning() {
  update();
  return !_halted && !(_chip == CHIP_DS3231 && _osf);
}

// ---- Чтение ------------------------------------------------------------------------

void AlashRTC::update() {
  if (_chip == CHIP_NONE) return;
  uint8_t b[7];
  if (!readBytes(REG_TIME, b, 7)) {
    _err = ERR_I2C;
    return;
  }
  _halted = (_chip == CHIP_DS1307) && (b[0] & 0x80);     // DS1307: бит CH остановил часы
  _s = bcd2bin(b[0] & 0x7F);
  _mi = bcd2bin(b[1] & 0x7F);
  if (b[2] & 0x40) {                                      // 12-часовой режим: бит 5 = PM
    uint8_t h = bcd2bin(b[2] & 0x1F);
    if (h == 12) h = 0;
    _h = h + ((b[2] & 0x20) ? 12 : 0);
  } else {
    _h = bcd2bin(b[2] & 0x3F);
  }
  _d = bcd2bin(b[4] & 0x3F);
  _mo = bcd2bin(b[5] & 0x1F);
  _y = 2000 + bcd2bin(b[6]);
  if (_chip == CHIP_DS3231) {
    uint8_t st = 0;
    if (read8(REG_STATUS, st)) _osf = (st & 0x80) != 0;
  }
  _hasData = true;
  _readAt = millis();
  _err = ERR_OK;
}

void AlashRTC::ensureFresh() {
  if (_hasData && (uint32_t)(millis() - _readAt) < 100) return;
  update();
}

uint16_t AlashRTC::year() { ensureFresh(); return _y; }
uint8_t AlashRTC::month() { ensureFresh(); return _mo; }
uint8_t AlashRTC::day() { ensureFresh(); return _d; }
uint8_t AlashRTC::hour() { ensureFresh(); return _h; }
uint8_t AlashRTC::minute() { ensureFresh(); return _mi; }
uint8_t AlashRTC::second() { ensureFresh(); return _s; }
uint8_t AlashRTC::dayOfWeek() { ensureFresh(); return weekday(_y, _mo, _d); }

uint16_t AlashRTC::dayOfYear() {
  ensureFresh();
  uint16_t n = _d;
  for (uint8_t m = 1; m < _mo; m++) n += daysInMonth(_y, m);
  return n;
}

uint32_t AlashRTC::unixTime() {
  ensureFresh();
  return (uint32_t)daysFromCivil(_y, _mo, _d) * 86400UL + _h * 3600UL + _mi * 60UL + _s;
}

void AlashRTC::format(char *buf, size_t size) {
  ensureFresh();
  snprintf(buf, size, "%04u-%02u-%02u %02u:%02u:%02u", _y, _mo, _d, _h, _mi, _s);
}

const __FlashStringHelper *AlashRTC::dayName() {
  uint8_t w = dayOfWeek();
  switch (w) {
    case 1: return T("Понедельник", "Дүйсенбі", "Monday");
    case 2: return T("Вторник", "Сейсенбі", "Tuesday");
    case 3: return T("Среда", "Сәрсенбі", "Wednesday");
    case 4: return T("Четверг", "Бейсенбі", "Thursday");
    case 5: return T("Пятница", "Жұма", "Friday");
    case 6: return T("Суббота", "Сенбі", "Saturday");
    default: return T("Воскресенье", "Жексенбі", "Sunday");
  }
}

// ---- DS3231: температура и будильник -----------------------------------------------------

float AlashRTC::temperature() {
  if (_chip != CHIP_DS3231) {
    _err = ERR_NOT_SUPPORTED;
    return NAN;
  }
  uint8_t t[2];
  if (!readBytes(REG_TEMP, t, 2)) {
    _err = ERR_I2C;
    return NAN;
  }
  _err = ERR_OK;
  return (int8_t)t[0] + (t[1] >> 6) * 0.25f;
}

bool AlashRTC::setAlarm(uint8_t hour, uint8_t minute, uint8_t second) {
  if (_chip != CHIP_DS3231) {
    _err = ERR_NOT_SUPPORTED;
    return false;
  }
  if (hour > 23 || minute > 59 || second > 59) {
    _err = ERR_BAD_TIME;
    return false;
  }
  // Будильник 1: совпадение часов, минут и секунд (бит A1M4 = 1: день не учитывается)
  uint8_t b[4] = {bin2bcd(second), bin2bcd(minute), bin2bcd(hour), 0x80};
  if (!writeBytes(REG_ALARM1, b, 4)) {
    _err = ERR_I2C;
    return false;
  }
  uint8_t ctl = 0, st = 0;
  read8(REG_CONTROL, ctl);
  write8(REG_CONTROL, ctl | 0x01 | 0x04);              // A1IE и INTCN: будильник выводится на пин INT/SQW
  read8(REG_STATUS, st);
  write8(REG_STATUS, st & ~0x01);                       // сбросить прошлое срабатывание
  _err = ERR_OK;
  return true;
}

bool AlashRTC::alarmFired() {
  if (_chip != CHIP_DS3231) return false;
  uint8_t st = 0;
  return read8(REG_STATUS, st) && (st & 0x01);
}

void AlashRTC::clearAlarm() {
  uint8_t st = 0;
  if (_chip == CHIP_DS3231 && read8(REG_STATUS, st)) write8(REG_STATUS, st & ~0x03);
}

void AlashRTC::disableAlarm() {
  uint8_t ctl = 0;
  if (_chip == CHIP_DS3231 && read8(REG_CONTROL, ctl)) write8(REG_CONTROL, ctl & ~0x03);
  clearAlarm();
}

bool AlashRTC::setSquareWave(uint16_t hz) {
  if (_chip == CHIP_DS3231) {
    uint8_t ctl = 0;
    if (!read8(REG_CONTROL, ctl)) return false;
    ctl &= ~(0x04 | 0x18);                               // INTCN = 0 (меандр), биты RS1 RS2
    if (hz == 0) ctl |= 0x04;
    else if (hz == 1) ctl |= 0x00;
    else if (hz == 1024) ctl |= 0x08;
    else if (hz == 4096) ctl |= 0x10;
    else if (hz == 8192) ctl |= 0x18;
    else return false;
    return write8(REG_CONTROL, ctl);
  }
  if (_chip == CHIP_DS1307) {
    uint8_t v = hz == 0 ? 0x00 : hz == 1 ? 0x10 : hz == 4096 ? 0x11 : hz == 8192 ? 0x12 : hz == 32768U ? 0x13 : 0xFF;
    return v != 0xFF && write8(0x07, v);
  }
  return false;
}

// ---- I2C -----------------------------------------------------------------------------------

bool AlashRTC::readBytes(uint8_t reg, uint8_t *buf, uint8_t len) {
  _wire->beginTransmission(ADDR);
  _wire->write(reg);
  if (_wire->endTransmission(false) != 0) return false;
  if (_wire->requestFrom(ADDR, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = (uint8_t)_wire->read();
  return true;
}

bool AlashRTC::writeBytes(uint8_t reg, const uint8_t *buf, uint8_t len) {
  _wire->beginTransmission(ADDR);
  _wire->write(reg);
  for (uint8_t i = 0; i < len; i++) _wire->write(buf[i]);
  return _wire->endTransmission() == 0;
}

bool AlashRTC::write8(uint8_t reg, uint8_t v) { return writeBytes(reg, &v, 1); }
bool AlashRTC::read8(uint8_t reg, uint8_t &v) { return readBytes(reg, &v, 1); }

// ---- Диагностика ---------------------------------------------------------------------------

const __FlashStringHelper *AlashRTC::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_DEVICE:
      return T("Часы не найдены: проверьте SDA / SCL, питание (3.3-5 В) и GND. Адрес часов 0x68",
               "Сағат табылмады: SDA / SCL, қуатты (3.3-5 В) және GND тексеріңіз. Сағат мекенжайы 0x68",
               "The clock is not found: check SDA / SCL, power (3.3-5 V) and GND. The clock address is 0x68");
    case ERR_I2C:
      return T("Ошибка I2C: проверьте контакты и подтяжку линий", "I2C қатесі: байланысты және тартқыш резисторларды тексеріңіз",
               "I2C error: check connections and pull-up resistors");
    case ERR_BAD_TIME:
      return T("Такой даты или времени не бывает (год 2000-2099, месяц 1-12, день по календарю, часы 0-23)",
               "Мұндай күн не уақыт болмайды (жыл 2000-2099, ай 1-12, күн күнтізбе бойынша, сағат 0-23)",
               "Such a date or time does not exist (year 2000-2099, month 1-12, valid day, hours 0-23)");
    case ERR_NOT_SUPPORTED:
      return T("DS1307 не умеет температуру и будильники: они есть только у DS3231",
               "DS1307 температура мен оятқышты білмейді: олар тек DS3231-де бар",
               "DS1307 has no temperature or alarms: only the DS3231 does");
  }
  return F("");
}

void AlashRTC::diagnose(Print &out) {
  out.println(T("AlashRTC: диагностика", "AlashRTC: диагностика", "AlashRTC: diagnostics"));
  if (_chip == CHIP_NONE) {
    out.println(errorText());
    return;
  }
  out.print(T("Микросхема: ", "Микросхема: ", "Chip: "));
  out.println(chipName());
  update();
  if (_err != ERR_OK) {
    out.println(errorText());
    return;
  }
  char buf[24];
  format(buf, sizeof buf);
  out.print(T("Время: ", "Уақыт: ", "Time: "));
  out.println(buf);
  if (lostPower()) {
    out.println(T("Часы теряли питание или остановились: время недостоверно. Поставьте его: setFromCompileTime() или setTime(...). Если так при каждом включении, замените батарейку CR2032 (у модуля с зарядкой ZS-042 не ставьте обычную CR2032 без доработки схемы заряда).",
                  "Сағат қуатсыз қалды немесе тоқтады: уақыт сенімсіз. Оны қойыңыз: setFromCompileTime() немесе setTime(...). Әр қосқанда осылай болса, CR2032 батареясын ауыстырыңыз.",
                  "The clock lost power or stopped: the time is not reliable. Set it with setFromCompileTime() or setTime(...). If this happens at every power-up, replace the CR2032 battery."));
  }
  if (_chip == CHIP_DS3231) {
    out.print(T("Температура: ", "Температура: ", "Temperature: "));
    out.println(temperature(), 2);
  }
}
