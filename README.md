# Slime-Simulation

Based on [Slime-Simulation by Sebastian Lague](https://github.com/SebLague/Slime-Simulation). Built in C using the [OpenGL 4](https://registry.khronos.org/OpenGL-Refpages/gl4/) libraries.

## Building and Running

1. Install dependencies `gl glfw glew` according to your system.
2. Configure settings for simulation, trails, and species in [src/config.h](src/config.h).
3. Build the program with `gcc -o main src/main.c -lm -lglfw -lGL -lGLEW`
4. Run the program with `./main`.

## Controls

Q/ESC to quit
A/D to `add` and `delete` a species

## TODO

- Graphical smoothing option (gaussian blur?)
- CLI args for configuration
- Loadable config files
- Spawn agents according to image using color quantization
- Mouse interaction for attract/repel
