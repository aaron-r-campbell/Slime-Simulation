#include "config.h"
#include "types.c"
#include "types.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Shaders
GLuint agentUpdateShaderProgram = 0;
GLuint decayDiffuseShaderProgram = 0;
GLuint renderShaderProgram = 0;
GLuint blurShaderProgram = 0;

// Textures
GLuint agentTexture = 0;
GLuint tempTexture = 0;
GLuint renderTexture = 0;

// Fullscreen quad
GLuint quadVAO = 0;
GLuint quadVBO = 0;

// FBO
GLuint renderFBO = 0;

float rand01() { return (float)rand() / (float)RAND_MAX; }

// Initialize random agents
void initAgents(Agent *agents, int numAgents) {
  vec2 center = {simWidth / 2.0f, simHeight / 2.0f};

  for (int i = 0; i < numAgents; i++) {
    vec2 position;
    float angle;

    float randomAngle = rand01() * M_PI * 2.0f;
    float r = sqrt(rand01()) * fmin(simHeight, simWidth) * 0.45f;

    switch (spawnMode) {
    case POINT:
      position = center;
      angle = randomAngle;
      break;
    case RANDOM:
      position = (vec2){rand() % simWidth, rand() % simHeight};
      angle = randomAngle;
      break;
    case CIRCLE_INWARD:
      position = (vec2){center.x + r * cos(randomAngle), center.y + r * sin(randomAngle)};
      angle = -randomAngle;
      break;
    case CIRCLE_RANDOM:
      position = (vec2){center.x + r * cos(randomAngle), center.y + r * sin(randomAngle)};
      angle = rand01() * M_PI * 2.0f;
      break;
    }

    agents[i].position = position;
    agents[i].angle = angle;
    agents[i].speciesIndex = (int)((rand() / (float)RAND_MAX) * numSpecies);
  }
}

// Window resize callback
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  windowWidth = width;
  windowHeight = height;
  glViewport(0, 0, width, height);
}

const char *readFile(const char *filePath) {
  FILE *file = fopen(filePath, "r");
  if (!file) {
    fprintf(stderr, "Failed to open compute shader file: %s\n", filePath);
    exit(EXIT_FAILURE);
  }
  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  rewind(file);
  char *code = (char *)malloc(length + 1);
  fread(code, 1, length, file);
  code[length] = '\0';
  fclose(file);
  return (const char *)code;
}

GLuint compileShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  // Check for compilation errors
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    fprintf(stderr, "Shader compilation error: %s\n", infoLog);
    return 0;
  }

  return shader;
}

GLuint createShaderProgram(const char *vertexShaderSource, const char *fragmentShaderSource) {
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  glDeleteShader(vertexShader);
  glDeleteProgram(fragmentShader);

  // Check for linking errors
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    fprintf(stderr, "Program linking error: %s\n", infoLog);
    return 0;
  }

  return program;
}

GLuint createComputeProgram(const char *source) {
  GLuint computeShader = compileShader(GL_COMPUTE_SHADER, source);
  GLuint program = glCreateProgram();
  glAttachShader(program, computeShader);
  glLinkProgram(program);

  // Check for linking errors
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    fprintf(stderr, "Compute program linking error: %s\n", infoLog);
    return 0;
  }

  glDeleteShader(computeShader);
  return program;
}

void createTextures() {
  // Create agent texture
  glGenTextures(1, &agentTexture);
  glBindTexture(GL_TEXTURE_2D, agentTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, simWidth, simHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  // Create temp texture
  glGenTextures(1, &tempTexture);
  glBindTexture(GL_TEXTURE_2D, tempTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, simWidth, simHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  // Create render texture
  glGenTextures(1, &renderTexture);
  glBindTexture(GL_TEXTURE_2D, renderTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, simWidth, simHeight, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void createQuad() {
  // Vertices for a fullscreen quad (positions and texture coordinates)
  float vertices[] = {// positions        // texture coords
                      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

                      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 1.0f};

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);

  glBindVertexArray(quadVAO);

  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Texture coordinate attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void createFBOs() {
  // Create FBO for render texture
  glGenFramebuffers(1, &renderFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "Framebuffer is not complete!\n");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cleanup() {
  glDeleteProgram(agentUpdateShaderProgram);
  glDeleteProgram(decayDiffuseShaderProgram);
  glDeleteProgram(renderShaderProgram);
  glDeleteProgram(blurShaderProgram);

  glDeleteTextures(1, &agentTexture);
  glDeleteTextures(1, &tempTexture);
  glDeleteTextures(1, &renderTexture);

  glDeleteVertexArrays(1, &quadVAO);
  glDeleteBuffers(1, &quadVBO);

  glDeleteFramebuffers(1, &renderFBO);
}

void changeSpeciesCount(Agent *agents, int change) {
  numSpecies = numSpecies + change;

  if (numSpecies < 1) {
    numSpecies = 1;
    return;
  } else if (numSpecies > numSpeciesSettings) {
    numSpecies = numSpeciesSettings;
    return;
  }

  if (change > 0) {
    for (int i = 0; i < numAgents; i++) {
      if (agents[i].speciesIndex < numSpecies - 1) { // change proportional amount of old species into new
        if (rand() % numSpecies == 0) {
          agents[i].speciesIndex = numSpecies - 1;
        }
      }
    }
  } else {
    for (int i = 0; i < numAgents; i++) {
      if (agents[i].speciesIndex == numSpecies) { // Change only removed species' agents
        agents[i].speciesIndex = (int)((rand() / (float)RAND_MAX) * numSpecies);
      }
    }
  }
  fprintf(stdout, "Changed speciesNum to %d\n", numSpecies);
}

enum KeyAction { NO_ACTION, INCREASE_SPECIES, DECREASE_SPECIES, CLOSE_WINDOW };
enum KeyAction keyAction = NO_ACTION;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (action == GLFW_PRESS && keyAction == NO_ACTION) {
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
      keyAction = CLOSE_WINDOW;
    } else if (key == GLFW_KEY_A) {
      keyAction = INCREASE_SPECIES;
    } else if (key == GLFW_KEY_D) {
      keyAction = DECREASE_SPECIES;
    }
  } else if (action == GLFW_RELEASE) {
    keyAction = NO_ACTION;
  }
}

int main() {
  // -------------- VALIDATION --------------
  if (numSpecies > numSpeciesSettings) {
    fprintf(stderr, "Not all species have settings definition\n");
    return -1;
  }

  // -------------- INITIALIZATION --------------
  // Initialize random seed
  srand(time(NULL));

  // -------------- GLFW INITIALIZATION --------------
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  // Set OpenGL version and profile
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a window
  GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Agent Simulation", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // -------------- GLEW INITIALIZATION --------------
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwTerminate();
    return -1;
  }

  // Check for compute shader support
  if (!GLEW_ARB_compute_shader) {
    fprintf(stderr, "Compute shaders not supported!\n");
    glfwTerminate();
    return -1;
  }

  // -------------- SPECIES SETTINGS INITIALIZATION --------------
  GLuint speciesSettingsSSBO;
  glGenBuffers(1, &speciesSettingsSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, speciesSettingsSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numSpeciesSettings * sizeof(SpeciesSettings), speciesSettings,
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, speciesSettingsSSBO);

  // -------------- AGENT INITIALIZATION --------------
  // Allocate and initialize agents
  Agent *agents = (Agent *)malloc(numAgents * sizeof(Agent));
  if (agents == NULL) {
    fprintf(stderr, "Memory allocation failed for Agents.\n");
    glfwTerminate();
    return -1;
  }
  initAgents(agents, numAgents);

  // Create shader storage buffer object for agents
  GLuint agentSSBO;
  glGenBuffers(1, &agentSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, agentSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numAgents * sizeof(Agent), agents, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, agentSSBO);

  // Create vertex array object for agents
  GLuint agentVAO;
  glGenVertexArrays(1, &agentVAO);

  // -------------- SHADERS SETUP --------------
  agentUpdateShaderProgram = createComputeProgram(readFile("shaders/agent_update.glsl"));
  decayDiffuseShaderProgram = createComputeProgram(readFile("shaders/decay_diffuse.glsl"));
  renderShaderProgram = createShaderProgram(readFile("shaders/vertex.glsl"), readFile("shaders/render_fragment.glsl"));
  blurShaderProgram = createShaderProgram(readFile("shaders/vertex.glsl"), readFile("shaders/blur_fragment.glsl"));

  GLuint agentUpdateNumAgentsLoc = glGetUniformLocation(agentUpdateShaderProgram, "numAgents");
  GLuint agentUpdateSimWidthLoc = glGetUniformLocation(agentUpdateShaderProgram, "simWidth");
  GLuint agentUpdateSimHeightLoc = glGetUniformLocation(agentUpdateShaderProgram, "simHeight");
  GLuint agentUpdateTrailWeightLoc = glGetUniformLocation(agentUpdateShaderProgram, "trailWeight");
  GLuint agentUpdateDeltaTimeLoc = glGetUniformLocation(agentUpdateShaderProgram, "deltaTime");
  GLuint agentUpdateTimeLoc = glGetUniformLocation(agentUpdateShaderProgram, "time");

  GLuint renderNumSpeciesLoc = glGetUniformLocation(renderShaderProgram, "numSpecies");
  GLuint renderBackgroundColorLoc = glGetUniformLocation(renderShaderProgram, "backgroundColor");
  GLfloat renderBackgroundColor[4] = {backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w};

  GLuint decayDiffuseDecayRateLoc = glGetUniformLocation(decayDiffuseShaderProgram, "decayRate");
  GLuint decayDiffuseDiffuseRateLoc = glGetUniformLocation(decayDiffuseShaderProgram, "diffuseRate");
  GLuint decayDiffuseDeltaTimeLoc = glGetUniformLocation(decayDiffuseShaderProgram, "deltaTime");
  GLuint decayDiffuseSimWidthLoc = glGetUniformLocation(decayDiffuseShaderProgram, "simWidth");
  GLuint decayDiffuseSimHeightLoc = glGetUniformLocation(decayDiffuseShaderProgram, "simHeight");

  createTextures();
  createQuad();
  createFBOs();

  int currentBuffer = 0;
  int keydown = 0;
  float lastFrameTime = 0.0f;

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    if (keyAction == INCREASE_SPECIES) {
      changeSpeciesCount(agents, 1);
      glBufferData(GL_SHADER_STORAGE_BUFFER, numAgents * sizeof(Agent), agents, GL_DYNAMIC_DRAW);
      keyAction = NO_ACTION; // Reset action to avoid retriggering
    } else if (keyAction == DECREASE_SPECIES) {
      changeSpeciesCount(agents, -1);
      glBufferData(GL_SHADER_STORAGE_BUFFER, numAgents * sizeof(Agent), agents, GL_DYNAMIC_DRAW);
      keyAction = NO_ACTION; // Reset action to avoid retriggering
    } else if (keyAction == CLOSE_WINDOW) {
      glfwSetWindowShouldClose(window, true);
      keyAction = NO_ACTION; // Reset action to avoid retriggering
    }

    // Step 1: Run compute shader to update agent texture
    glUseProgram(agentUpdateShaderProgram);
    glUniform1i(agentUpdateNumAgentsLoc, numAgents);
    glUniform1i(agentUpdateSimWidthLoc, simWidth);
    glUniform1i(agentUpdateSimHeightLoc, simHeight);
    glUniform1f(agentUpdateTrailWeightLoc, trailWeight);
    glUniform1f(agentUpdateDeltaTimeLoc, deltaTime);
    glUniform1f(agentUpdateTimeLoc, currentTime);
    glBindImageTexture(0, agentTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glDispatchCompute((numAgents + 15) / 16, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Get updated agent positions
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, agentSSBO);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numAgents * sizeof(Agent), agents);

    glCopyImageSubData(agentTexture, GL_TEXTURE_2D, 0, 0, 0, 0, tempTexture, GL_TEXTURE_2D, 0, 0, 0, 0, simWidth,
                       simHeight, 1);

    // Step 2: decay and diffuse
    glUseProgram(decayDiffuseShaderProgram);
    glUniform1i(decayDiffuseDecayRateLoc, decayRate);
    glUniform1f(decayDiffuseDiffuseRateLoc, diffuseRate);
    glUniform1f(decayDiffuseDeltaTimeLoc, deltaTime);
    glBindImageTexture(0, tempTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, agentTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute((simWidth + 15) / 16, (simHeight + 15) / 16, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Step 3: Render from agent texture to render texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(renderShaderProgram);
    glUniform1i(renderNumSpeciesLoc, numSpecies);
    glUniform4fv(renderBackgroundColorLoc, 1, renderBackgroundColor);
    glBindVertexArray(quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, agentTexture);
    glUniform1i(glGetUniformLocation(renderShaderProgram, "agentTexture"), 0);

    glDispatchCompute(16, 16, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Step 4: Blur

    // Step 5: Clear textures for next frame
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glClearTexImage(renderTexture, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();

    // // Exit on Escape key
    // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    //   glfwSetWindowShouldClose(window, true);
    // }
    //
    // else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    //   changeSpeciesCount(agents, 1);
    //   glBufferData(GL_SHADER_STORAGE_BUFFER, numAgents * sizeof(Agent), agents, GL_DYNAMIC_DRAW);
    // } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    //   changeSpeciesCount(agents, -1);
    //   glBufferData(GL_SHADER_STORAGE_BUFFER, numAgents * sizeof(Agent), agents, GL_DYNAMIC_DRAW);
    // }
  }

  // Cleanup
  cleanup();
  glfwTerminate();
  return 0;
}
