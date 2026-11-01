// AlashRTC на ESP32 со своими пинами I2C: begin(SDA, SCL).
#include <AlashRTC.h>

AlashRTC rtc;

void setup() {
  Serial.begin(115200);
  if (!rtc.begin(21, 22)) {               // SDA = GPIO21, SCL = GPIO22
    Serial.println(rtc.errorText());
    while (true) delay(1000);
  }
  if (rtc.lostPower()) rtc.setFromCompileTime();
}

void loop() {
  char text[24];
  rtc.format(text, sizeof text);
  Serial.println(text);
  delay(1000);
}
