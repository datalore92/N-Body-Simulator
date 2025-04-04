# N-Body Simulator

A physics-based gravitational simulation that models the interaction of particles with varying mass and radius. Particles are attracted to each other according to Newton's law of universal gravitation and merge when they collide.

![N-Body Simulator](assets/particle.png)

## Features

- Real-time gravitational simulation based on physical laws (F = G * m1 * m2 / r²)
- Particles with different mass and corresponding visual size
- Dynamic particle merging upon collision with conservation of momentum
- Interactive particle placement via mouse clicks
- Adjustable mass for new particles
- Visual color coding based on particle mass (heavier = redder)
- **NEW**: Spatial partitioning grid for optimized performance with many particles
- **NEW**: Visualization options for force lines, velocity vectors, and grid
- **NEW**: Simulation speed control and pause functionality
- **NEW**: Real-time status information in window title

## Requirements

- Windows, macOS, or Linux
- C compiler (GCC, MSVC, or Clang)
- SDL2 library

## Installation

### Windows

1. Install MinGW-w64 or MSVC
2. Install SDL2 development libraries
   - Download from [SDL website](https://www.libsdl.org/download-2.0.php)
   - Extract and place in your compiler's library path or in the project directory

### macOS

```bash
brew install sdl2
```

### Linux

```bash
sudo apt-get install libsdl2-dev
```

## Building

### Direct Compilation (Windows with MinGW)

```
gcc -o particles-demo src/main.c src/particle.c src/renderer.c src/utils.c -Isrc -DSDL_MAIN_HANDLED -lSDL2 -lm
```

### Using Makefile

```bash
make
```

### Using CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running the Simulator

After building, run the executable:

```bash
# From main directory if using direct compilation or Makefile
./particles-demo.exe   # On Windows
./particles-demo       # On macOS/Linux

# From build directory if using CMake
./ParticlesDemo.exe    # On Windows
./ParticlesDemo        # On macOS/Linux
```

Note: If you encounter issues running the program, make sure SDL2.dll is in the same directory as your executable (Windows only).

## Controls

### Particle Management
- **Left Mouse Button**: Click to place a new particle
- **Up/Down Arrow Keys**: Increase/decrease the mass of new particles
- **R Key**: Reset the simulation

### Visualization Options
- **G Key**: Toggle spatial grid visibility
- **F Key**: Toggle force lines between particles
- **V Key**: Toggle velocity vectors
- **Space**: Pause/resume simulation
- **Plus/Minus Keys**: Increase/decrease simulation speed

### System
- **ESC Key**: Exit the application

## How It Works

The simulator uses Newtonian physics to calculate gravitational forces between all particles. Each particle has:

- Position (x, y)
- Velocity (vx, vy)
- Mass (affects gravitational pull and visual size)

The physics simulation:
1. Calculates gravitational force between each particle pair
2. Updates velocities based on the forces
3. Updates positions based on velocities
4. Detects and handles collisions by merging particles
5. Conserves momentum during mergers

### Performance Optimization

The simulator uses spatial partitioning to optimize performance:
- The screen is divided into a grid of cells
- Particles are assigned to cells based on their position
- Only particles in the same or adjacent cells interact
- This reduces computational complexity from O(n²) to nearly O(n)

## Future Improvements

- Further optimization with Barnes-Hut algorithm for very large numbers of particles
- Custom gravitational constants and simulation parameters
- Preset scenarios (solar system, binary stars, galaxy formation)
- Particle trails to visualize orbits
- Support for different gravitational laws
- UI improvements with text rendering for statistics
- Save/Load simulation state