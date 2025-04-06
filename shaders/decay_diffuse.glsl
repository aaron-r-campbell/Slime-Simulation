#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform image2D TrailMap;
layout(binding = 1, rgba32f) uniform image2D DiffusedTrailMap;
uniform int decayRate;
uniform float diffuseRate;
uniform float deltaTime;
uniform int simWidth;
uniform int simHeight;

void main() {
  ivec2 id = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);

  vec4 sum = vec4(0);
  vec4 originalCol = imageLoad(TrailMap, id);
  // 3x3 blur
  int diffuseSize = 0;
  for (int offsetX = -diffuseSize; offsetX <= diffuseSize; offsetX++) {
    for (int offsetY = -diffuseSize; offsetY <= diffuseSize; offsetY++) {
      int sampleX = min(simWidth - 1, max(0, id.x + offsetX));
      int sampleY = min(simHeight - 1, max(0, id.y + offsetY));
      sum += imageLoad(TrailMap, ivec2(sampleX, sampleY));
    }
  }

  vec4 blurredCol = sum / pow(diffuseSize * 2 + 1, 2);
  float diffuseWeight = clamp(diffuseRate * deltaTime, 0, 1);
  blurredCol = originalCol * (1 - diffuseWeight) + blurredCol * (diffuseWeight);

  imageStore(DiffusedTrailMap, id, max(vec4(0, 0, 0, 0), blurredCol - vec4(decayRate * deltaTime)));
}
