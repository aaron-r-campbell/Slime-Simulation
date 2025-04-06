#include <stdint.h>

#ifndef TYPES_H
#define TYPES_H

typedef struct __attribute__((aligned(8))) {
  float x, y;
} vec2;

typedef struct __attribute__((aligned(16))) {
  float x, y, z, w;
} vec4;

typedef struct {
  vec2 position;
  float angle;
  int32_t speciesIndex;
} Agent;

typedef struct {
  float moveSpeed;
  float turnSpeed;
  float sensorAngleDegrees;
  float sensorOffsetDst;
  vec4 color;
  int32_t sensorSize;
} SpeciesSettings;

enum SpawnMode { POINT, RANDOM, CIRCLE_INWARD, CIRCLE_RANDOM };

enum KeyTriggerMode {
  KEY_DOWN, // Trigger on key down (press)
  KEY_UP,   // Trigger on key up (release)
  KEY_HOLD  // Trigger while key is held down
};

#endif
