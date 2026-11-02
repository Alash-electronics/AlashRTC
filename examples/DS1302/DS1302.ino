// [ru] AlashRTC с модулем DS1302 (три провода, не I2C): RST (CE), DAT (IO), CLK (SCLK) на любые пины.
//      Подключение: VCC 3.3-5 В, GND, RST к пину 7, DAT к пину 6, CLK к пину 5. Батарейка CR2032 обязательна: без неё DS1302 не идёт.
//      Для PCF8563 (I2C, адрес 0x51) ничего менять не нужно: rtc.begin() найдёт его сам.
// [kk] AlashRTC DS1302 модулімен (үш сым, I2C емес): RST (CE), DAT (IO), CLK (SCLK) кез келген пиндерге.
//      Қосу: VCC 3.3-5 В, GND, RST 7-пинге, DAT 6-пинге, CLK 5-пинге. CR2032 батареясы міндетті: онсыз DS1302 жүрмейді.
//      PCF8563 үшін (I2C, мекенжайы 0x51) ештеңе өзгертпейсіз: rtc.begin() оны өзі табады.
// [en] AlashRTC with a DS1302 module (three wires, not I2C): RST (CE), DAT (IO), CLK (SCLK) on any pins.
//      Wiring: VCC 3.3-5 V, GND, RST to pin 7, DAT to pin 6, CLK to pin 5. A CR2032 battery is required: the DS1302 does not run without it.
//      For a PCF8563 (I2C, address 0x51) change nothing: rtc.begin() finds it by itself.
#include <AlashRTC.h>

#if defined(ESP32)
const int PIN_RST = 27, PIN_DAT = 26, PIN_CLK = 25;
#elif defined(ESP8266)
const int PIN_RST = 14, PIN_DAT = 12, PIN_CLK = 13;
#else
const int PIN_RST = 7, PIN_DAT = 6, PIN_CLK = 5;
#endif

AlashRTC rtc;

void setup() {
  Serial.begin(115200);
  if (!rtc.beginDS1302(PIN_RST, PIN_DAT, PIN_CLK)) {
    Serial.println(rtc.errorText());
    while (true) delay(1000);
  }
  if (rtc.lostPower()) {                  // первый запуск или села батарейка
    Serial.println(F("Time lost, setting the compile time"));
    rtc.setFromCompileTime();
  }
}

void loop() {
  char text[24];
  rtc.format(text, sizeof text);
  Serial.print(text);
  Serial.print(F("  "));
  Serial.println(rtc.dayName());
  delay(1000);
}
