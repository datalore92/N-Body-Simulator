#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>  // For sqrtf
#include "renderer.h"
#include "particle.h"
#include "utils.h"

// Make sure SDL_main is defined properly for Windows
#ifdef _WIN32
#undef main
#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_PARTICLES 500
#define SIMULATION_SPEED 1.0f

// Visualization options
typedef struct {
    bool showGrid;           // Show spatial partitioning grid
    bool showForceLines;     // Show gravity force lines between particles
    bool showVelocityVectors; // Show velocity vectors
    bool pauseSimulation;    // Pause physics simulation
    float timeScale;         // Time scale for simulation speed
} VisualizationOptions;

// Function to draw force lines between particles
void draw_force_lines(SDL_Renderer *renderer, Particle *particles, int count) {
    for (int i = 0; i < count; i++) {
        if (!particles[i].active) continue;
        
        for (int j = i + 1; j < count; j++) {
            if (!particles[j].active) continue;
            
            // Calculate distance
            float dx = particles[j].x - particles[i].x;
            float dy = particles[j].y - particles[i].y;
            float distance_sq = dx * dx + dy * dy;
            
            // Only draw lines for particles within a reasonable distance
            if (distance_sq < 100 * 100) {
                float force = G * particles[i].mass * particles[j].mass / distance_sq;
                
                // Scale the alpha value based on force strength
                Uint8 alpha = (Uint8)(force * 5000.0f);
                if (alpha > 100) alpha = 100;
                
                // Set color based on force strength
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, alpha);
                
                // Draw line between centers of particles
                SDL_RenderDrawLine(renderer, 
                                  (int)particles[i].x, 
                                  (int)particles[i].y, 
                                  (int)particles[j].x, 
                                  (int)particles[j].y);
            }
        }
    }
}

// Function to draw velocity vectors for particles
void draw_velocity_vectors(SDL_Renderer *renderer, Particle *particles, int count) {
    for (int i = 0; i < count; i++) {
        if (!particles[i].active) continue;
        
        // Scale velocity for visualization
        float scale = 5.0f;
        int endX = (int)(particles[i].x + particles[i].vx * scale);
        int endY = (int)(particles[i].y + particles[i].vy * scale);
        
        // Set color for velocity vector (cyan)
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 200);
        
        // Draw velocity vector
        SDL_RenderDrawLine(renderer, 
                          (int)particles[i].x, 
                          (int)particles[i].y, 
                          endX, endY);
                          
        // Draw a small tip at the end of the vector
        SDL_Rect tip = {endX - 2, endY - 2, 4, 4};
        SDL_RenderFillRect(renderer, &tip);
    }
}

// Function to draw spatial grid for debugging
void draw_grid(SDL_Renderer *renderer, int width, int height) {
    float cellWidth = (float)width / GRID_SIZE;
    float cellHeight = (float)height / GRID_SIZE;
    
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 100);
    
    // Draw vertical lines
    for (int i = 1; i < GRID_SIZE; i++) {
        int x = (int)(i * cellWidth);
        SDL_RenderDrawLine(renderer, x, 0, x, height);
    }
    
    // Draw horizontal lines
    for (int i = 1; i < GRID_SIZE; i++) {
        int y = (int)(i * cellHeight);
        SDL_RenderDrawLine(renderer, 0, y, width, y);
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "N-Body Simulator", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Create renderer
    SDL_Renderer* renderer = NULL;
    if (init_renderer(&renderer, &window, WINDOW_WIDTH, WINDOW_HEIGHT) != 0) {
        fprintf(stderr, "Failed to create renderer!\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Initialize random generator
    init_random();

    // Create particles
    int particleCount = 100;
    Particle* particles = create_particles(particleCount);
    if (!particles) {
        fprintf(stderr, "Failed to create particles!\n");
        cleanup_renderer(renderer, window);
        SDL_Quit();
        return -1;
    }

    // Initialize visualization options
    VisualizationOptions visOptions = {
        .showGrid = false,
        .showForceLines = false,
        .showVelocityVectors = false,
        .pauseSimulation = false,
        .timeScale = 1.0f
    };

    // Variables for mouse interaction
    bool leftMouseDown = false;
    bool rightMouseDown = false;
    int mouseX = 0, mouseY = 0;
    float placementMass = 50.0f;  // Default mass for placed particles
    int activeCount = particleCount;

    // Timing variables
    Uint32 lastFrameTime = get_current_time();
    Uint32 currentTime;
    float deltaTime;

    // Main loop
    bool running = true;
    SDL_Event event;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        leftMouseDown = true;
                        mouseX = event.button.x;
                        mouseY = event.button.y;
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        rightMouseDown = true;
                        mouseX = event.button.x;
                        mouseY = event.button.y;
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        leftMouseDown = false;
                        
                        // Create a new particle if there's space
                        if (activeCount < MAX_PARTICLES) {
                            // Find an inactive slot or the end
                            int newIndex = -1;
                            for (int i = 0; i < particleCount; i++) {
                                if (!particles[i].active) {
                                    newIndex = i;
                                    break;
                                }
                            }
                            
                            if (newIndex == -1 && particleCount < MAX_PARTICLES) {
                                newIndex = particleCount++;
                            }
                            
                            if (newIndex != -1) {
                                // Create particle with random velocity
                                float vx = random_float(-0.5f, 0.5f);
                                float vy = random_float(-0.5f, 0.5f);
                                
                                particles[newIndex].x = mouseX;
                                particles[newIndex].y = mouseY;
                                particles[newIndex].vx = vx;
                                particles[newIndex].vy = vy;
                                particles[newIndex].mass = placementMass;
                                particles[newIndex].radius = calculate_radius(placementMass);
                                particles[newIndex].active = 1;
                                
                                // Set color based on mass
                                Uint8 r = (Uint8)(128 + placementMass / 100.0f * 127);
                                Uint8 g = (Uint8)(192 - placementMass / 100.0f * 128);
                                Uint8 b = (Uint8)(255 - placementMass / 100.0f * 128);
                                
                                particles[newIndex].color.r = r;
                                particles[newIndex].color.g = g;
                                particles[newIndex].color.b = b;
                                particles[newIndex].color.a = 255;
                                
                                activeCount++;
                            }
                        }
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        rightMouseDown = false;
                    }
                    break;
                    
                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    break;
                    
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        case SDLK_UP:
                            placementMass += 10.0f;
                            if (placementMass > 200.0f) placementMass = 200.0f;
                            printf("Particle mass: %.1f\n", placementMass);
                            break;
                        case SDLK_DOWN:
                            placementMass -= 10.0f;
                            if (placementMass < 10.0f) placementMass = 10.0f;
                            printf("Particle mass: %.1f\n", placementMass);
                            break;
                        case SDLK_r:
                            // Reset simulation
                            for (int i = 0; i < particleCount; i++) {
                                free_particle(&particles[i]);
                            }
                            free(particles);
                            particleCount = 100;
                            particles = create_particles(particleCount);
                            activeCount = particleCount;
                            break;
                        case SDLK_g:
                            // Toggle grid visualization
                            visOptions.showGrid = !visOptions.showGrid;
                            break;
                        case SDLK_f:
                            // Toggle force lines
                            visOptions.showForceLines = !visOptions.showForceLines;
                            break;
                        case SDLK_v:
                            // Toggle velocity vectors
                            visOptions.showVelocityVectors = !visOptions.showVelocityVectors;
                            break;
                        case SDLK_SPACE:
                            // Toggle pause
                            visOptions.pauseSimulation = !visOptions.pauseSimulation;
                            break;
                        case SDLK_EQUALS:  // Plus key (usually +/= key)
                            // Increase simulation speed
                            visOptions.timeScale *= 1.2f;
                            if (visOptions.timeScale > 5.0f) visOptions.timeScale = 5.0f;
                            printf("Time scale: %.1f\n", visOptions.timeScale);
                            break;
                        case SDLK_MINUS:
                            // Decrease simulation speed
                            visOptions.timeScale /= 1.2f;
                            if (visOptions.timeScale < 0.1f) visOptions.timeScale = 0.1f;
                            printf("Time scale: %.1f\n", visOptions.timeScale);
                            break;
                    }
                    break;
            }
        }

        // Calculate delta time for physics update
        currentTime = get_current_time();
        deltaTime = (float)(currentTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentTime;
        
        // Cap deltaTime to avoid large jumps
        if (deltaTime > 0.05f) deltaTime = 0.05f;
        
        // Apply simulation speed
        deltaTime *= SIMULATION_SPEED * visOptions.timeScale;

        // Update all particles with calculated delta time, unless paused
        if (!visOptions.pauseSimulation) {
            update_particles(particles, particleCount, deltaTime);
        }

        // Count active particles
        activeCount = 0;
        for (int i = 0; i < particleCount; i++) {
            if (particles[i].active) activeCount++;
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Draw optional grid
        if (visOptions.showGrid) {
            draw_grid(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
        }
        
        // Draw force lines if option enabled
        if (visOptions.showForceLines) {
            draw_force_lines(renderer, particles, particleCount);
        }
        
        // Render all particles
        render_particles(renderer, particles, particleCount);
        
        // Draw velocity vectors if option enabled
        if (visOptions.showVelocityVectors) {
            draw_velocity_vectors(renderer, particles, particleCount);
        }
        
        // Render placement preview if mouse button is down
        if (leftMouseDown) {
            // Show a preview of the particle that will be placed
            SDL_SetRenderDrawColor(renderer, 
                128 + (Uint8)(placementMass / 100.0f * 127),
                192 - (Uint8)(placementMass / 100.0f * 128),
                255 - (Uint8)(placementMass / 100.0f * 128),
                128);
            
            int previewRadius = (int)calculate_radius(placementMass);
            
            // Draw preview circle
            for (int dy = -previewRadius; dy <= previewRadius; dy++) {
                int dx = (int)sqrtf(previewRadius * previewRadius - dy * dy);
                SDL_RenderDrawLine(renderer, mouseX - dx, mouseY + dy, mouseX + dx, mouseY + dy);
            }
        }
        
        // Render info text (using printf for now, in a real app we'd use SDL_ttf)
        char title[256];
        sprintf(title, "N-Body Sim - Particles: %d - Mass: %.1f - [G]rid: %s - [F]orce: %s - [V]elocity: %s - [Space]: %s - Scale: %.1fx", 
                activeCount, 
                placementMass,
                visOptions.showGrid ? "On" : "Off",
                visOptions.showForceLines ? "On" : "Off",
                visOptions.showVelocityVectors ? "On" : "Off",
                visOptions.pauseSimulation ? "Paused" : "Running",
                visOptions.timeScale);
        SDL_SetWindowTitle(window, title);
        
        // Present the rendered frame
        SDL_RenderPresent(renderer);
        
        // Small delay to prevent using 100% CPU
        SDL_Delay(1);
    }

    // Cleanup
    free_particles(particles, particleCount);
    cleanup_renderer(renderer, window);
    SDL_Quit();
    
    return 0;
}