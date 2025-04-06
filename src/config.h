#include "types.h"

#ifndef CONFIG_H
#define CONFIG_H

// Simulation settings
int stepsPerFrame = 1;
int simWidth = 1920;
int simHeight = 1080;
int numSpecies = 1;
int numAgents = 25000;
vec4 backgroundColor = (vec4){13, 16, 22, 1};
static enum SpawnMode spawnMode = CIRCLE_RANDOM;

// Trail settings
int trailWeight = 12;
float decayRate = 0.5;
int diffuseRate = 3;

// Window settings
int windowWidth = 1920;
int windowHeight = 1080;

SpeciesSettings speciesSettings[] = {(SpeciesSettings){
                                         .moveSpeed = 70,
                                         .turnSpeed = 30,
                                         .sensorAngleDegrees = 60,
                                         .sensorOffsetDst = 20,
                                         .sensorSize = 1,
                                         .color = (vec4){90, 193, 254, 1},
                                     },
                                     (SpeciesSettings){
                                         .moveSpeed = 50,
                                         .turnSpeed = -90,
                                         .sensorAngleDegrees = 112,
                                         .sensorOffsetDst = 30,
                                         .sensorSize = 1,
                                         .color = (vec4){210, 166, 255, 1},
                                     },
                                     (SpeciesSettings){
                                         .moveSpeed = 50,
                                         .turnSpeed = 50,
                                         .sensorAngleDegrees = 70,
                                         .sensorOffsetDst = 30,
                                         .sensorSize = 1,
                                         .color = (vec4){254, 143, 64, 1},
                                     }};

int numSpeciesSettings = sizeof(speciesSettings) / sizeof(SpeciesSettings);

#endif
