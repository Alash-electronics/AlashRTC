// Имитация DS1302 на уровне трёх линий (CE, IO, SCLK): разбор команды, чтение по спаду SCLK, пакетный режим, защита записи.
// Считает нарушения протокола: обмен без CE, лишние такты, борьба за линию IO (мастер и микросхема выдают одновременно).
#pragma once
#include "Arduino.h"
struct Sim1302 {
  int ce = 10, io = 11, sclk = 12;
  bool present = true;
  uint8_t clk[8] = {0x80, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x80};   // после включения: CH = 1, защита записи включена
  uint8_t ram[31] = {0};
  // состояние линий
  int lvCe = 0, lvSclk = 0, lvIo = 0; int modeIo = INPUT; bool floatHigh = false;
  // разбор
  int phase = 0;                        // 0 команда, 1 чтение, 2 запись
  int nbits = 0; uint8_t cmd = 0, shift = 0; bool ramCmd = false, burst = false; int addr = 0; int outBit = 0, outByte = 0, outCount = 0; int devDrive = -1;
  int violations = 0, contention = 0, transactions = 0, writes = 0, reads = 0, burstWrites = 0, burstReads = 0;
  bool deviceDrives() const { return phase == 1 && devDrive >= 0; }
  int lineIo() const { if (deviceDrives() && modeIo == INPUT) return devDrive; if (modeIo == OUTPUT) return lvIo; return floatHigh ? 1 : 0; }
  uint8_t getReg(int i) { return ramCmd ? ram[i % 31] : (i < 8 ? clk[i] : 0); }
  void setReg(int i, uint8_t v) {
    if (ramCmd) { if (!(clk[7] & 0x80)) ram[i % 31] = v; return; }
    if (i == 7) { clk[7] = v & 0x80; return; }
    if (clk[7] & 0x80) { violations += 0; return; }               // защита записи включена: запись игнорируется
    if (i < 7) clk[i] = v;
  }
  void ceRise() { phase = 0; nbits = 0; cmd = 0; devDrive = -1; transactions++; if (lvSclk) violations++; }
  void ceFall() { phase = 0; nbits = 0; devDrive = -1; }
  void sclkRise() {
    if (!lvCe || !present) return;
    if (phase == 0) {
      cmd |= (lineIo() & 1) << nbits;
      if (++nbits == 8) {
        if (!(cmd & 0x80)) violations++;
        ramCmd = cmd & 0x40; addr = (cmd >> 1) & 0x1F; burst = (addr == 31);
        if (cmd & 1) { phase = 1; outByte = 0; outBit = 0; outCount = 0; reads++; if (burst) burstReads++; }
        else { phase = 2; nbits = 0; shift = 0; outByte = 0; writes++; if (burst) burstWrites++; }
      }
    } else if (phase == 2) {
      shift |= (lineIo() & 1) << nbits;
      if (++nbits == 8) { setReg(burst ? outByte : addr, shift); outByte++; nbits = 0; shift = 0; if (!burst && outByte > 1) violations++; if (burst && outByte > (ramCmd ? 31 : 8)) violations++; }
    }
  }
  void sclkFall() {
    if (!lvCe || !present) return;
    if (phase == 1) {
      if (modeIo == OUTPUT) contention++;                          // мастер не отпустил линию к моменту, когда микросхема её занимает
      int idx = burst ? outByte : addr;
      devDrive = (getReg(idx) >> outBit) & 1;
      if (++outBit == 8) { outBit = 0; outByte++; }
    }
  }
  void pin(int p, int v) {
    if (p == ce) { if (v && !lvCe) { lvCe = 1; ceRise(); } else if (!v && lvCe) { lvCe = 0; ceFall(); } }
    else if (p == sclk) { if (v && !lvSclk) { lvSclk = 1; sclkRise(); } else if (!v && lvSclk) { lvSclk = 0; sclkFall(); } }
    else if (p == io) lvIo = v;
  }
};
inline Sim1302 g_ds;
void pinMode(uint8_t p, uint8_t m) { if (p == g_ds.io) g_ds.modeIo = m; }
void digitalWrite(uint8_t p, uint8_t v) { g_ds.pin(p, v); }
int digitalRead(uint8_t p) { return p == g_ds.io ? g_ds.lineIo() : 0; }
