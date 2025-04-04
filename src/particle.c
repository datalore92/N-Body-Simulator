#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "particle.h"
#include "utils.h"

// Helper functions for grid (moved to top of file)
static inline int max_int(int a, int b) {
    return a > b ? a : b;
}

static inline int min_int(int a, int b) {
    return a < b ? a : b;
}

// Calculate radius based on mass (assuming density is constant)
float calculate_radius(float mass) {
    return 2.0f + sqrtf(mass) * 2.0f; // Simple scaling formula
}

// Create a system of particles
Particle* create_particles(int count) {
    Particle* particles = (Particle*)malloc(count * sizeof(Particle));
    if (particles == NULL) {
        fprintf(stderr, "Failed to allocate memory for particles\n");
        return NULL;
    }
    
    // Initialize particles with random values
    for (int i = 0; i < count; i++) {
        float mass = random_float(10.0f, 100.0f);
        float x = random_float(50.0f, 750.0f);
        float y = random_float(50.0f, 550.0f);
        float vx = random_float(-1.0f, 1.0f);
        float vy = random_float(-1.0f, 1.0f);
        
        particles[i].x = x;
        particles[i].y = y;
        particles[i].vx = vx;
        particles[i].vy = vy;
        particles[i].mass = mass;
        particles[i].radius = calculate_radius(mass);
        particles[i].active = 1;
        
        // Assign a color based on mass
        Uint8 r = (Uint8)(128 + mass / 100.0f * 127); // Red increases with mass
        Uint8 g = (Uint8)(192 - mass / 100.0f * 128); // Green decreases with mass
        Uint8 b = (Uint8)(255 - mass / 100.0f * 128); // Blue decreases with mass
        
        particles[i].color.r = r;
        particles[i].color.g = g;
        particles[i].color.b = b;
        particles[i].color.a = 255;
    }
    
    return particles;
}

// Create a single particle with specified parameters
Particle* create_particle(float x, float y, float vx, float vy, float mass) {
    Particle* p = (Particle*)malloc(sizeof(Particle));
    if (p == NULL) {
        fprintf(stderr, "Failed to allocate memory for particle\n");
        return NULL;
    }
    
    p->x = x;
    p->y = y;
    p->vx = vx;
    p->vy = vy;
    p->mass = mass;
    p->radius = calculate_radius(mass);
    p->active = 1;
    
    // Assign a color based on mass
    Uint8 r = (Uint8)(128 + mass / 100.0f * 127);
    Uint8 g = (Uint8)(192 - mass / 100.0f * 128);
    Uint8 b = (Uint8)(255 - mass / 100.0f * 128);
    
    p->color.r = r;
    p->color.g = g;
    p->color.b = b;
    p->color.a = 255;
    
    return p;
}

// Initialize spatial grid for efficient collision detection and gravity calculation
void init_grid(SpatialGrid* grid, int width, int height) {
    grid->width = width;
    grid->height = height;
    grid->cellWidth = (float)width / GRID_SIZE;
    grid->cellHeight = (float)height / GRID_SIZE;
    
    // Initialize all cells to empty
    clear_grid(grid);
}

// Clear the grid for the next frame update
void clear_grid(SpatialGrid* grid) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid->cells[i][j].count = 0;
        }
    }
}

// Add particle to the appropriate grid cell
void add_particle_to_grid(SpatialGrid* grid, Particle* particles, int index) {
    Particle* p = &particles[index];
    if (!p->active) return;
    
    // Calculate grid cell coordinates
    int cellX = (int)(p->x / grid->cellWidth);
    int cellY = (int)(p->y / grid->cellHeight);
    
    // Clamp to grid boundaries
    if (cellX < 0) cellX = 0;
    if (cellX >= GRID_SIZE) cellX = GRID_SIZE - 1;
    if (cellY < 0) cellY = 0;
    if (cellY >= GRID_SIZE) cellY = GRID_SIZE - 1;
    
    // Add particle index to cell
    GridCell* cell = &grid->cells[cellY][cellX];
    if (cell->count < MAX_PARTICLES_PER_CELL) {
        cell->particleIndices[cell->count++] = index;
    }
}

// Apply gravitational force between two particles
void apply_gravity(Particle* p1, Particle* p2, float dt) {
    if (!p1->active || !p2->active) return;
    
    // Calculate distance between particles
    float dx = p2->x - p1->x;
    float dy = p2->y - p1->y;
    float distance_sq = dx * dx + dy * dy;
    
    // Avoid division by zero and dampen extreme forces when very close
    if (distance_sq < 1.0f) distance_sq = 1.0f;
    
    // Calculate gravitational force
    float distance = sqrtf(distance_sq);
    float force_magnitude = G * p1->mass * p2->mass / distance_sq;
    
    // Calculate force components
    float force_x = force_magnitude * dx / distance;
    float force_y = force_magnitude * dy / distance;
    
    // Apply acceleration to both particles (F = ma, a = F/m)
    p1->vx += force_x / p1->mass * dt;
    p1->vy += force_y / p1->mass * dt;
    p2->vx -= force_x / p2->mass * dt;
    p2->vy -= force_y / p2->mass * dt;
}

// Check if two particles are colliding
int check_collision(Particle* p1, Particle* p2) {
    if (!p1->active || !p2->active) return 0;
    
    float dx = p2->x - p1->x;
    float dy = p2->y - p1->y;
    float distance_sq = dx * dx + dy * dy;
    float radii_sum = p1->radius + p2->radius;
    
    return distance_sq <= (radii_sum * radii_sum);
}

// Merge two colliding particles
void merge_particles(Particle* p1, Particle* p2) {
    // Ensure p1 is the larger mass (we'll keep p1 and deactivate p2)
    if (p1->mass < p2->mass) {
        Particle* temp = p1;
        p1 = p2;
        p2 = temp;
    }
    
    // Conservation of momentum
    float total_mass = p1->mass + p2->mass;
    p1->vx = (p1->vx * p1->mass + p2->vx * p2->mass) / total_mass;
    p1->vy = (p1->vy * p1->mass + p2->vy * p2->mass) / total_mass;
    
    // Update mass and radius
    p1->mass = total_mass;
    p1->radius = calculate_radius(total_mass);
    
    // Update color based on new mass
    Uint8 r = (Uint8)(128 + p1->mass / 100.0f * 127);
    Uint8 g = (Uint8)(192 - p1->mass / 100.0f * 128);
    Uint8 b = (Uint8)(255 - p1->mass / 100.0f * 128);
    if (r > 255) r = 255;
    
    p1->color.r = r;
    p1->color.g = g;
    p1->color.b = b;
    
    // Deactivate the second particle
    p2->active = 0;
}

// Update a single particle
void update_particle(Particle* p, float dt) {
    if (p != NULL && p->active) {
        // Update position based on velocity
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        
        // Simple boundary collision - bounce off the edges
        if (p->x - p->radius < 0) {
            p->x = p->radius;
            p->vx = -p->vx * 0.8f; // Lose some energy
        } 
        else if (p->x + p->radius > 800) {
            p->x = 800 - p->radius;
            p->vx = -p->vx * 0.8f;
        }
        
        if (p->y - p->radius < 0) {
            p->y = p->radius;
            p->vy = -p->vy * 0.8f;
        }
        else if (p->y + p->radius > 600) {
            p->y = 600 - p->radius;
            p->vy = -p->vy * 0.8f;
        }
    }
}

// Update all particles using spatial grid for optimization
void update_particles(Particle* particles, int count, float dt) {
    static SpatialGrid grid;
    static int gridInitialized = 0;
    
    // Initialize grid on first call
    if (!gridInitialized) {
        init_grid(&grid, 800, 600); // Assuming window size is 800x600
        gridInitialized = 1;
    }
    
    // Clear grid for this frame
    clear_grid(&grid);
    
    // Add all particles to the grid
    for (int i = 0; i < count; i++) {
        if (particles[i].active) {
            add_particle_to_grid(&grid, particles, i);
        }
    }
    
    // Process gravity and collisions using the spatial grid
    for (int cellY = 0; cellY < GRID_SIZE; cellY++) {
        for (int cellX = 0; cellX < GRID_SIZE; cellX++) {
            GridCell* currentCell = &grid.cells[cellY][cellX];
            
            // Process particles within same cell
            for (int i = 0; i < currentCell->count; i++) {
                int p1Index = currentCell->particleIndices[i];
                if (!particles[p1Index].active) continue;
                
                // Check against other particles in same cell
                for (int j = i + 1; j < currentCell->count; j++) {
                    int p2Index = currentCell->particleIndices[j];
                    if (!particles[p2Index].active) continue;
                    
                    apply_gravity(&particles[p1Index], &particles[p2Index], dt);
                    
                    if (check_collision(&particles[p1Index], &particles[p2Index])) {
                        merge_particles(&particles[p1Index], &particles[p2Index]);
                    }
                }
                
                // Check against particles in neighboring cells
                for (int nCellY = max_int(0, cellY-1); nCellY <= min_int(GRID_SIZE-1, cellY+1); nCellY++) {
                    for (int nCellX = max_int(0, cellX-1); nCellX <= min_int(GRID_SIZE-1, cellX+1); nCellX++) {
                        // Skip current cell as we already processed it
                        if (nCellY == cellY && nCellX == cellX) continue;
                        
                        GridCell* neighborCell = &grid.cells[nCellY][nCellX];
                        
                        for (int j = 0; j < neighborCell->count; j++) {
                            int p2Index = neighborCell->particleIndices[j];
                            if (!particles[p2Index].active) continue;
                            
                            apply_gravity(&particles[p1Index], &particles[p2Index], dt);
                            
                            if (check_collision(&particles[p1Index], &particles[p2Index])) {
                                merge_particles(&particles[p1Index], &particles[p2Index]);
                            }
                        }
                    }
                }
                
                // Update particle position
                update_particle(&particles[p1Index], dt);
            }
        }
    }
}

// Render all particles
void render_particles(SDL_Renderer* renderer, Particle* particles, int count) {
    for (int i = 0; i < count; i++) {
        if (particles[i].active) {
            render_particle(renderer, &particles[i]);
        }
    }
}

// Free a particle
void free_particle(Particle* p) {
    free(p);
}

// Free all particles
void free_particles(Particle* particles, int count) {
    free(particles);
}