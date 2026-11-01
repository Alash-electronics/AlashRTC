# AlashRTC

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **часов реального времени DS3231 и DS1307** (модули ZS-042, Tiny RTC): дата, время, температура, будильник. Сама находит часы и определяет тип, ставит время по времени компиляции. Только встроенная `Wire`, **RTClib не нужна**. Сделана для уроков и наборов Alash Electronics.

```cpp
#include <AlashRTC.h>
AlashRTC rtc;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  if (rtc.lostPower()) rtc.setFromCompileTime();   // батарейка села: ставим время
}
void loop() {
  char t[24]; rtc.format(t, sizeof t);
  Serial.println(t);                               // 2026-11-02 14:05:09
  delay(1000);
}
```

### Что умеет
- **Сама находит часы:** адрес 0x68 и тип (DS3231 или DS1307).
- **Первая установка одной строкой:** `setFromCompileTime()`. `lostPower()` скажет, что батарейка села и время надо поставить заново.
- **Установка с проверкой:** `setTime(2026, 11, 2, 14, 5, 9)` не примет 30 февраля, час 24 или 29 февраля не високосного года. Есть `setUnixTime()`.
- **Чтение:** `year()`, `month()`, `day()`, `hour()`, `minute()`, `second()`, `dayOfWeek()` (1 = понедельник), `dayOfYear()`, `unixTime()`, `format()` строкой, `dayName()` на трёх языках.
- **DS3231:** `temperature()` (шаг 0.25), будильник `setAlarm(7, 30)` и `alarmFired()`, меандр `setSquareWave()`.
- **Не спрашивает часы лишний раз:** быстрые повторные вызовы читают по I2C раз в 100 мс.
- **Понятные подсказки** на русском, казахском и английском: `errorText()` и `diagnose(Serial)`.
- Работает на **Arduino AVR, ESP32, ESP8266**; `begin(SDA, SCL)` для своих пинов.

### Подключение
| Модуль часов | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| SQW / INT (необязательно) | D2 | GPIO4 |

Батарейка **CR2032** обязательна, иначе время сбрасывается при выключении. На модуле ZS-042 есть зарядная цепь для аккумулятора LIR2032: с обычной (не перезаряжаемой) CR2032 её нужно отключить (удалить диод или резистор заряда), иначе батарейка может вздуться.

### Чем отличается от других библиотек
Мы посмотрели RTClib (Adafruit), DS3231 (Rodan), uRTCLib и DS3232RTC:
- RTClib требует Adafruit BusIO и разные классы `RTC_DS3231`, `RTC_DS1307`; у нас один класс и автоопределение.
- Нет подсказки, что делать, если время сбилось; у нас `lostPower()` и `diagnose()`.
- Дни недели и русские названия приходится писать самим: у нас `dayName()` на трёх языках.
- Предупреждение про зарядку CR2032 на модулях ZS-042 есть только у нас.

### Примеры
`Basic`, `SetTime`, `Alarm`, `Datalog`, `Diagnose`, `ESP32_CustomPins`

### Тесты
BCD-регистры, определение микросхемы, дни недели и года (включая високосные), unix-время в обе стороны (границы 2000 и 2099), неверные даты, 12-часовой режим, температура с минусом, будильник, DS1307 проверены без железа на имитации регистров: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

**DS3231 және DS1307 нақты уақыт сағаттарына** (ZS-042, Tiny RTC модульдері) арналған қарапайым кітапхана: күн, уақыт, температура, оятқыш. Сағатты өзі тауып, түрін анықтайды, уақытты компиляция уақыты бойынша қояды. Тек кірістірілген `Wire`, **RTClib қажет емес**.

```cpp
#include <AlashRTC.h>
AlashRTC rtc;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  if (rtc.lostPower()) rtc.setFromCompileTime();   // батарейка села: ставим время
}
void loop() {
  char t[24]; rtc.format(t, sizeof t);
  Serial.println(t);                               // 2026-11-02 14:05:09
  delay(1000);
}
```

### Мүмкіндіктері
- **Сағатты өзі табады:** 0x68 мекенжайы және түрі (DS3231 не DS1307).
- **Бірінші орнату бір жолмен:** `setFromCompileTime()`. `lostPower()` батарея отырғанын айтады.
- **Тексерумен орнату:** `setTime(2026, 11, 2, 14, 5, 9)` 30 ақпанды, 24 сағатты қабылдамайды. `setUnixTime()` бар.
- **Оқу:** `year()`, `month()`, `day()`, `hour()`, `minute()`, `second()`, `dayOfWeek()`, `dayOfYear()`, `unixTime()`, `format()`, `dayName()`.
- **DS3231:** `temperature()`, оятқыш `setAlarm(7, 30)` және `alarmFired()`.
- **Артық сұрамайды:** жиі шақыру 100 мс сайын оқиды.
- **Түсінікті кеңестер** орысша, қазақша, ағылшынша: `errorText()`, `diagnose(Serial)`.
- **Arduino AVR, ESP32, ESP8266**; `begin(SDA, SCL)`.

### Қосу
| Сағат модулі | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| SQW / INT (необязательно) | D2 | GPIO4 |

**CR2032** батареясы міндетті, әйтпесе қуат өшкенде уақыт ысырылады. ZS-042 модулінде LIR2032 аккумуляторына зарядтау тізбегі бар: қарапайым CR2032 болса, оны өшіру керек.

### Басқа кітапханалардан айырмашылығы
RTClib (Adafruit), DS3231 (Rodan), uRTCLib және DS3232RTC қарадық:
- RTClib-ке Adafruit BusIO керек және әр түрлі кластар; бізде бір класс.
- Уақыт ысырылса кеңес жоқ; бізде `lostPower()` және `diagnose()`.
- Апта күндерінің атаулары үш тілде: `dayName()`.

### Мысалдар
`Basic`, `SetTime`, `Alarm`, `Datalog`, `Diagnose`, `ESP32_CustomPins`

### Тесттер
BCD регистрлері, микросхеманы анықтау, апта күндері, unix-уақыт, қате күндер, 12 сағаттық режим, температура, оятқыш тексерілген: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for **DS3231 and DS1307 real-time clocks** (ZS-042, Tiny RTC modules): date, time, temperature, alarm. Finds the clock and its type by itself, sets the time from the compile time. Only the built-in `Wire`, **no RTClib needed**. Made for classes and Alash Electronics kits.

```cpp
#include <AlashRTC.h>
AlashRTC rtc;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  if (rtc.lostPower()) rtc.setFromCompileTime();   // батарейка села: ставим время
}
void loop() {
  char t[24]; rtc.format(t, sizeof t);
  Serial.println(t);                               // 2026-11-02 14:05:09
  delay(1000);
}
```

### Features
- **Finds the clock itself:** address 0x68 and the type (DS3231 or DS1307).
- **First setup in one line:** `setFromCompileTime()`. `lostPower()` tells when the battery died and the time must be set again.
- **Checked setting:** `setTime(2026, 11, 2, 14, 5, 9)` rejects Feb 30, hour 24 or Feb 29 of a non-leap year. There is also `setUnixTime()`.
- **Reading:** `year()`, `month()`, `day()`, `hour()`, `minute()`, `second()`, `dayOfWeek()` (1 = Monday), `dayOfYear()`, `unixTime()`, `format()` as text, `dayName()` in three languages.
- **DS3231:** `temperature()` (0.25 step), alarm `setAlarm(7, 30)` and `alarmFired()`, square wave `setSquareWave()`.
- **Does not poll needlessly:** fast repeated calls read over I2C once per 100 ms.
- **Clear tips** in Russian, Kazakh and English: `errorText()` and `diagnose(Serial)`.
- Works on **Arduino AVR, ESP32, ESP8266**; `begin(SDA, SCL)` for custom pins.

### Wiring
| Clock module | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| SQW / INT (необязательно) | D2 | GPIO4 |

A **CR2032** battery is required, otherwise the time resets on power off. The ZS-042 module has a charging circuit meant for a rechargeable LIR2032: with a normal (non-rechargeable) CR2032 it must be disabled (remove the diode or charge resistor).

### How it differs from other libraries
We looked at RTClib (Adafruit), DS3231 (Rodan), uRTCLib and DS3232RTC:
- RTClib needs Adafruit BusIO and separate `RTC_DS3231` / `RTC_DS1307` classes; we have one class with auto-detection.
- No tips when the time is lost; we have `lostPower()` and `diagnose()`.
- Day names in three languages with `dayName()`.

### Examples
`Basic`, `SetTime`, `Alarm`, `Datalog`, `Diagnose`, `ESP32_CustomPins`

### Tests
BCD registers, chip detection, weekdays and day of year (leap years), unix time both ways (2000 and 2099 bounds), invalid dates, 12-hour mode, negative temperature, alarm and DS1307 are checked without hardware: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
