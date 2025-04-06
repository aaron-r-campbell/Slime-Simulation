#version 430 core

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;

struct Agent {
  vec2 position;
  float angle;
  int speciesIndex;
};

struct SpeciesSettings {
  float moveSpeed;
  float turnSpeed;
  float sensorAngleDegrees;
  float sensorOffsetDst;
  vec4 color;
  int sensorSize;
};

layout(binding = 0, rgba32f) uniform image2D TrailMap;
layout(std430, binding = 1) buffer SpeciesSettingsBuffer { SpeciesSettings speciesSettings[]; };
layout(std430, binding = 2) buffer AgentBuffer { Agent agents[]; };
uniform int numAgents;
uniform int simWidth;
uniform int simHeight;
uniform float trailWeight;
uniform float deltaTime;
uniform float time;

// Hash function and such to fake random
uint hash(uint state) {
  state ^= 2747636419u;
  state *= 2654435769u;
  state ^= state >> 16;
  state *= 2654435769u;
  state ^= state >> 16;
  state *= 2654435769u;
  return state;
}

float scaleToRange01(uint state) { return float(state) / 4294967295.0; } // 2^32-1

float sense(Agent agent, vec4 senseWeight, SpeciesSettings settings, float sensorAngleOffset) {
  ivec2 sensorPos =
      ivec2(agent.position + vec2(cos(agent.angle + sensorAngleOffset), sin(agent.angle + sensorAngleOffset)) *
                                 settings.sensorOffsetDst);

  float sum = 0;

  for (int offsetX = -settings.sensorSize; offsetX <= settings.sensorSize; ++offsetX) {
    for (int offsetY = -settings.sensorSize; offsetY <= settings.sensorSize; ++offsetY) {
      ivec2 sampleXY = clamp(sensorPos + ivec2(offsetX, offsetY), ivec2(0), ivec2(simWidth - 1, simHeight - 1));
      sum += dot(senseWeight, imageLoad(TrailMap, sampleXY));
    }
  }

  return sum;
}

void main() {
  uint id = gl_GlobalInvocationID.x;
  if (id >= agents.length()) {
    return;
  }

  Agent agent = agents[id];
  SpeciesSettings settings = speciesSettings[agent.speciesIndex];
  vec2 pos = agent.position;

  vec4 speciesMask = vec4(0);
  speciesMask[agents[id].speciesIndex] = 1;

  // Steer based on sensory data
  vec4 senseWeight = speciesMask * 2 - 1;
  float sensorAngleRad = settings.sensorAngleDegrees * 3.1415 / 180;
  float weightForward = sense(agent, senseWeight, settings, 0);
  float weightLeft = sense(agent, senseWeight, settings, sensorAngleRad);
  float weightRight = sense(agent, senseWeight, settings, -sensorAngleRad);
  uint random = hash(uint(pos.x * simWidth + pos.y + id + time * 100000));
  float randomSteerStrength = scaleToRange01(random);
  float turnSpeed = settings.turnSpeed * 2 * 3.1415;
  if (weightForward > weightLeft && weightForward > weightRight) {
    // Forward
  } else if (weightRight > weightLeft) {
    // Left
    agents[id].angle -= randomSteerStrength * turnSpeed * deltaTime;
  } else if (weightLeft > weightRight) {
    // Right
    agents[id].angle += randomSteerStrength * turnSpeed * deltaTime;
  } else {
    // Random when either direciton is a better option than forward
    agents[id].angle += (randomSteerStrength - 0.5) * 2 * turnSpeed * deltaTime;
  }

  // Update position
  vec2 direction = vec2(cos(agent.angle), sin(agent.angle));
  vec2 newPos = pos + direction * settings.moveSpeed * deltaTime;
  if (newPos.x < 0 || newPos.x >= simWidth || newPos.y < 0 || newPos.y >= simHeight) {
    newPos = clamp(newPos, vec2(0), vec2(simWidth - 1, simHeight - 1));
    random = hash(random);
    agents[id].angle = scaleToRange01(random) * 2 * 3.1415;
  }

  // Update the map
  ivec2 coord = ivec2(newPos);
  vec4 oldTrail = imageLoad(TrailMap, coord);
  imageStore(TrailMap, coord, min(vec4(1), oldTrail + speciesMask * trailWeight * deltaTime));
  agents[id].position = newPos;
}
