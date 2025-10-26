// util.h - small utility helpers for DrawingGameBackend
// Provides float/double versions of Arduino's map() function

#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>

// Map floats safely. Returns out_min if in_min == in_max to avoid divide-by-zero.
static inline float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  if (in_max == in_min) return out_min;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Map doubles (on many MCUs double==float). Returns out_min on divide-by-zero.
static inline double mapDouble(double x, double in_min, double in_max, double out_min, double out_max) {
  if (in_max == in_min) return out_min;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Optional clamping helper
static inline float clampFloat(float v, float a, float b) {
  if (a < b) {
    if (v < a) return a;
    if (v > b) return b;
    return v;
  } else {
    if (v < b) return b;
    if (v > a) return a;
    return v;
  }
}

#endif // UTIL_H
