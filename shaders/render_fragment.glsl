#version 430

struct SpeciesSettings {
  float moveSpeed;
  float turnSpeed;
  float sensorAngleDegrees;
  float sensorOffsetDst;
  vec4 color;
  int sensorSize;
};

in vec2 TexCoord;
out vec4 FragColor;
uniform int numSpecies;
uniform vec4 backgroundColor;
uniform sampler2D TrailMap;
layout(std430, binding = 1) buffer SpeciesSettingsBuffer { SpeciesSettings speciesSettings[]; };

void main() {
  vec4 map = texture(TrailMap, TexCoord);
  vec4 color = backgroundColor;

  for (uint i = 0; i < numSpecies; i++) {
    vec4 mask = vec4(i == 0, i == 1, i == 2, i == 3);
    color += speciesSettings[i].color * dot(map, mask);
  }

  FragColor = color / vec4(vec3(255), 1);
}
