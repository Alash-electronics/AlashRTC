// Минимальная имитация Arduino.h для запуска тестов на компьютере (g++).
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using __FlashStringHelper = char;
#define F(x) ((const __FlashStringHelper *)(x))
#define INPUT 0
#define OUTPUT 1
#define LOW 0
#define HIGH 1
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
int digitalRead(uint8_t pin);
inline void delayMicroseconds(unsigned) {}
inline uint32_t g_ms = 100000;
inline uint32_t millis() { return g_ms; }
class Print {
 public:
  void print(const char *s) { printf("%s", s); }
  void print(float v, int d = 2) { printf("%.*f", d, v); }
  void println(const char *s = "") { printf("%s\n", s); }
  void println(float v, int d = 2) { print(v, d); printf("\n"); }
};
