# AlashRTC: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для **часов реального времени DS3231 и DS1307** (модули ZS-042, Tiny RTC): дата, время, температура, будильник. Сама находит часы и определяет тип, ставит время по времени компиляции. Только встроенная `Wire`, **RTClib не нужна**. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashRTC** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

### Подключение
| Модуль часов | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| SQW / INT (необязательно) | D2 | GPIO4 |

Батарейка **CR2032** обязательна, иначе время сбрасывается при выключении. На модуле ZS-042 есть зарядная цепь для аккумулятора LIR2032: с обычной (не перезаряжаемой) CR2032 её нужно отключить (удалить диод или резистор заряда), иначе батарейка может вздуться.

### Быстрый старт
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
- **Четыре микросхемы:** DS3231, DS1307, PCF8563 (I2C) и DS1302 (три провода, `beginDS1302(RST, DAT, CLK)`). Дешёвые модули из наборов подходят.

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin()` | Найти часы и определить тип |
| `setFromCompileTime()` | Поставить время компиляции |
| `setTime(y, m, d, h, mi, s)` | Поставить время (с проверкой даты) |
| `setUnixTime(sec)` | Поставить по секундам с 1970 |
| `lostPower() / isRunning()` | Время недостоверно / часы идут |
| `year() month() day() hour() minute() second()` | Поля даты и времени |
| `dayOfWeek() / dayOfYear() / dayName()` | День недели (1 = пн), день года, название |
| `unixTime() / format(buf, size)` | Секунды с 1970 / строка YYYY-MM-DD hh:mm:ss |
| `temperature()` | Температура, °C (только DS3231) |
| `setAlarm(h, m, s) / alarmFired() / clearAlarm()` | Будильник раз в сутки (DS3231) |
| `setSquareWave(hz)` | Меандр на выводе SQW |
| `diagnose(Serial)` | Самопроверка и подсказки |
| `beginDS1302(ce, io, sclk)` | DS1302: три провода RST / DAT / CLK на любых пинах |

### Если что-то не работает
**Время сбрасывается при выключении питания**

Нет батарейки CR2032 или она села. Замените батарейку и поставьте время: `setFromCompileTime()`.

**Часы не найдены**

Проверьте SDA/SCL, питание и GND. Запустите пример Diagnose: он покажет, что видно на шине. У часов адрес 0x68; тот же адрес у датчика MPU6050, одновременно их подключить нельзя без смены адреса одного из них.

**Время отстаёт или убегает**

DS3231 очень точен (около минуты в год). Если уходит быстро, это DS1307 (кварц хуже) или китайская подделка. Поправка: ставить время раз в несколько месяцев.

**DS1302 не отвечает или время стоит**

Проверьте RST, DAT, CLK (не перепутаны ли) и питание. Батарейка CR2032 обязательна: без неё DS1302 не идёт, а `lostPower()` покажет true. Вызывайте `beginDS1302(RST, DAT, CLK)`, а не `begin()`.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `SetTime`, `Alarm`, `Datalog`, `Diagnose`, `ESP32_CustomPins`, `DS1302`

---

## Қазақша

**DS3231 және DS1307 нақты уақыт сағаттарына** (ZS-042, Tiny RTC модульдері) арналған қарапайым кітапхана: күн, уақыт, температура, оятқыш. Сағатты өзі тауып, түрін анықтайды, уақытты компиляция уақыты бойынша қояды. Тек кірістірілген `Wire`, **RTClib қажет емес**.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashRTC** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

### Қосу
| Сағат модулі | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| SQW / INT (необязательно) | D2 | GPIO4 |

**CR2032** батареясы міндетті, әйтпесе қуат өшкенде уақыт ысырылады. ZS-042 модулінде LIR2032 аккумуляторына зарядтау тізбегі бар: қарапайым CR2032 болса, оны өшіру керек.

### Жылдам бастау
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
- **Төрт микросхема:** DS3231, DS1307, PCF8563 (I2C) және DS1302 (үш сым, `beginDS1302(RST, DAT, CLK)`). Жинақтардағы арзан модульдер жарайды.

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin()` | Сағатты табу және түрін анықтау |
| `setFromCompileTime()` | Компиляция уақытын қою |
| `setTime(y, m, d, h, mi, s)` | Уақытты қою |
| `setUnixTime(sec)` | Секунд бойынша қою |
| `lostPower() / isRunning()` | Уақыт сенімсіз / сағат жүріп жатыр |
| `year() month() day() hour() minute() second()` | Күн мен уақыт өрістері |
| `dayOfWeek() / dayOfYear() / dayName()` | Апта күні, жыл күні, атауы |
| `unixTime() / format(buf, size)` | Секундтар / жол |
| `temperature()` | Температура (DS3231) |
| `setAlarm(h, m, s) / alarmFired() / clearAlarm()` | Тәулігіне бір оятқыш |
| `setSquareWave(hz)` | SQW шығысында меандр |
| `diagnose(Serial)` | Өзін тексеру |
| `beginDS1302(ce, io, sclk)` | DS1302: үш сым RST / DAT / CLK |

### Бірдеңе жұмыс істемесе
**Қуат өшкенде уақыт ысырылады**

CR2032 батареясы жоқ немесе отырған. Батареяны ауыстырып, уақытты қойыңыз.

**Сағат табылмады**

SDA/SCL, қуат және GND тексеріңіз. Diagnose мысалын іске қосыңыз. Сағат мекенжайы 0x68, MPU6050-де де сол.

**Уақыт артта қалады не озады**

DS3231 өте дәл. Тез кетсе, бұл DS1307 немесе жалған.

**DS1302 жауап бермейді не уақыт тұр**

RST, DAT, CLK (шатыспағанын) және қуатты тексеріңіз. CR2032 батареясы міндетті: онсыз DS1302 жүрмейді, `lostPower()` true көрсетеді. `begin()` емес, `beginDS1302(RST, DAT, CLK)` шақырыңыз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `SetTime`, `Alarm`, `Datalog`, `Diagnose`, `ESP32_CustomPins`, `DS1302`

---

## English

A simple library for **DS3231 and DS1307 real-time clocks** (ZS-042, Tiny RTC modules): date, time, temperature, alarm. Finds the clock and its type by itself, sets the time from the compile time. Only the built-in `Wire`, **no RTClib needed**. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashRTC** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

### Wiring
| Clock module | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 3.3 В |
| GND | GND | GND |
| SDA | A4 | GPIO21 |
| SCL | A5 | GPIO22 |
| SQW / INT (необязательно) | D2 | GPIO4 |

A **CR2032** battery is required, otherwise the time resets on power off. The ZS-042 module has a charging circuit meant for a rechargeable LIR2032: with a normal (non-rechargeable) CR2032 it must be disabled (remove the diode or charge resistor).

### Quick start
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
- **Four chips:** DS3231, DS1307, PCF8563 (I2C) and DS1302 (three wires, `beginDS1302(RST, DAT, CLK)`). The cheap kit modules work.

### Method reference
| Method | What it does |
|---|---|
| `begin()` | Find the clock and detect its type |
| `setFromCompileTime()` | Set the compile time |
| `setTime(y, m, d, h, mi, s)` | Set the time (validated) |
| `setUnixTime(sec)` | Set from unix seconds |
| `lostPower() / isRunning()` | Time not reliable / clock running |
| `year() month() day() hour() minute() second()` | Date and time fields |
| `dayOfWeek() / dayOfYear() / dayName()` | Weekday (1 = Mon), day of year, name |
| `unixTime() / format(buf, size)` | Unix seconds / text timestamp |
| `temperature()` | Temperature, °C (DS3231 only) |
| `setAlarm(h, m, s) / alarmFired() / clearAlarm()` | Daily alarm (DS3231) |
| `setSquareWave(hz)` | Square wave on the SQW pin |
| `diagnose(Serial)` | Self-check and tips |
| `beginDS1302(ce, io, sclk)` | DS1302: three wires RST / DAT / CLK on any pins |

### If something does not work
**The time resets when power is off**

No CR2032 battery or it is dead. Replace it and set the time with `setFromCompileTime()`.

**The clock is not found**

Check SDA/SCL, power and GND. Run the Diagnose example. The clock address is 0x68; the MPU6050 uses the same one, so change one of the addresses to use both.

**The time drifts**

The DS3231 is very accurate (about a minute per year). If it drifts fast, it is a DS1307 (weaker crystal) or a counterfeit.

**DS1302 does not answer or the time is frozen**

Check RST, DAT, CLK (not swapped) and power. A CR2032 battery is required: the DS1302 does not run without it and `lostPower()` shows true. Call `beginDS1302(RST, DAT, CLK)`, not `begin()`.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `SetTime`, `Alarm`, `Datalog`, `Diagnose`, `ESP32_CustomPins`, `DS1302`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashRTC
