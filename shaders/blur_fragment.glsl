#version 430
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D renderTexture;

void main() {
  // Basic 3x3 blur
  vec2 texelSize = 1.0 / textureSize(renderTexture, 0);

  vec4 result = vec4(0.0);
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      vec2 offset = vec2(float(x), float(y)) * texelSize;
      result += texture(renderTexture, TexCoord + offset) / 9.0;
    }
  }

  FragColor = result;
}
