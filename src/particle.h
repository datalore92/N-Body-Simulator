#ifndef PARTICLE_H
#define PARTICLE_H

#include <SDL2/SDL.h>

// Gravitational constant
#define G 6.67430e-2 // Scaled for simulation

// Spatial partitioning grid
#define GRID_SIZE 8
#define MAX_PARTICLES_PER_CELL 50

typedef struct {
    float x;          // Position X
    float y;          // Position Y
    float vx;         // Velocity X
    float vy;         // Velocity Y
    float mass;       // Mass of particle
    float radius;     // Radius based on mass
    SDL_Color color;  // Color for rendering
    int active;       // Whether the particle is active (1) or not (0)
} Particle;

typedef struct {
    int particleIndices[MAX_PARTICLES_PER_CELL];
    int count;
} GridCell;

typedef struct {
    GridCell cells[GRID_SIZE][GRID_SIZE];
    int width;
    int height;
    float cellWidth;
    float cellHeight;
} SpatialGrid;

// Calculate radius based on mass
float calculate_radius(float mass);

// Create particles system with a specific count
Particle* create_particles(int count);

// Create a single particle
Particle* create_particle(float x, float y, float vx, float vy, float mass);

// Update particle position based on physics
void update_particle(Particle* p, float dt);

// Initialize spatial partitioning grid
void init_grid(SpatialGrid* grid, int width, int height);

// Clear the grid for the next frame update
void clear_grid(SpatialGrid* grid);

// Add particle to the appropriate grid cell
void add_particle_to_grid(SpatialGrid* grid, Particle* particles, int index);

// Update all particles, including gravity calculations
void update_particles(Particle* particles, int count, float dt);

// Apply gravitational force between two particles
void apply_gravity(Particle* p1, Particle* p2, float dt);

// Check for collision between two particles
int check_collision(Particle* p1, Particle* p2);

// Merge two particles that have collided
void merge_particles(Particle* p1, Particle* p2);

// Render a single particle
void render_particle(SDL_Renderer* renderer, Particle* p);

// Render all particles
void render_particles(SDL_Renderer* renderer, Particle* particles, int count);

// Free a particle
void free_particle(Particle* p);

// Free all particles
void free_particles(Particle* particles, int count);

#endif // PARTICLE_H