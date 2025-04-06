#include "types.h"

vec2 vec2Add(vec2 a, vec2 b) { return (vec2){a.x + b.x, a.y + b.y}; }

vec2 vec2Sub(vec2 a, vec2 b) { return (vec2){a.x - b.x, a.y - b.y}; }

vec2 vec2Mul(vec2 a, vec2 b) { return (vec2){a.x * b.x, a.y * b.y}; }

vec2 vec2Div(vec2 a, vec2 b) { return (vec2){a.x / b.x, a.y / b.y}; }

vec2 vec2AddC(vec2 a, float c) { return (vec2){a.x + c, a.y + c}; }

vec2 vec2SubC(vec2 a, float c) { return (vec2){a.x - c, a.y - c}; }

vec2 vec2MulC(vec2 a, float c) { return (vec2){a.x * c, a.y * c}; }

vec2 vec2DivC(vec2 a, float c) { return (vec2){a.x / c, a.y / c}; }
