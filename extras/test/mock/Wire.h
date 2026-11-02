// Имитация I2C и часов DS3231 / DS1307 (карта регистров).
#pragma once
#include "Arduino.h"
struct FakeRtc {
  bool present = true; bool ds3231 = true; bool pcf = false; uint8_t addr = 0x68;
  uint8_t reg[64] = {0};
  void initPcf() {                                // PCF8563 по адресу 0x51: после включения VL = 1
    ds3231 = false; pcf = true; addr = 0x51; memset(reg, 0, sizeof reg);
    reg[0x02] = 0x80; reg[0x05] = 0x01; reg[0x07] = 0x01;
  }
  void init(bool is3231) {
    pcf = false; addr = 0x68;
    ds3231 = is3231; memset(reg, 0, sizeof reg);
    if (is3231) { reg[0x0E] = 0x1C; reg[0x0F] = 0x88; reg[0x11] = 25; reg[0x12] = 0x40; }   // OSF=1 после включения; 25.25 °C
    else { reg[0x00] = 0x80; for (int i = 0x08; i < 0x40; i++) reg[i] = 0xFF; }              // CH=1: часы стоят; память не очищена
  }
};
inline FakeRtc g_rtc;
class TwoWire {
 public:
  void begin() {}
  void begin(int, int) {}
  void beginTransmission(uint8_t a) { _a = a; _n = 0; }
  size_t write(uint8_t v) {
    if (_n == 0) _reg = v;
    else {
      uint8_t r = _reg + _n - 1;
      if (g_rtc.ds3231 && r == 0x0F) v = (g_rtc.reg[0x0F] & v) | (g_rtc.reg[0x0F] & 0x04);   // флаги в статусе сбрасываются записью нуля
      g_rtc.reg[r & 0x3F] = v;
    }
    _n++; return 1;
  }
  uint8_t endTransmission(bool = true) { return (g_rtc.present && _a == g_rtc.addr) ? 0 : 2; }
  uint8_t requestFrom(uint8_t a, uint8_t len) {
    if (!(g_rtc.present && a == g_rtc.addr)) return 0;
    for (uint8_t i = 0; i < len; i++) _buf[i] = g_rtc.reg[(_reg + i) & 0x3F];
    _len = len; _pos = 0; return len;
  }
  int read() { return _pos < _len ? _buf[_pos++] : -1; }
 private:
  uint8_t _a = 0, _reg = 0, _n = 0, _buf[32], _len = 0, _pos = 0;
};
inline TwoWire Wire;
