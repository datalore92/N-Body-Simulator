#include <SDL2/SDL.h>
#include "renderer.h"
#include "particle.h"

// Initialize the SDL renderer
int init_renderer(SDL_Renderer **renderer, SDL_Window **window, int width, int height) {
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

// Clear the screen with black color
void clear_renderer(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

// Render a single particle
void render_particle(SDL_Renderer *renderer, Particle *particle) {
    if (particle != NULL && particle->active) {
        // Set color for the particle
        SDL_SetRenderDrawColor(renderer, 
                              particle->color.r, 
                              particle->color.g, 
                              particle->color.b, 
                              particle->color.a);
        
        // Draw filled circle
        int radius = (int)particle->radius;
        int cx = (int)particle->x;
        int cy = (int)particle->y;
        
        // Draw a filled circle using multiple lines
        for (int dy = -radius; dy <= radius; dy++) {
            int dx = (int)sqrtf(radius * radius - dy * dy);
            SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
        }
    }
}

// Clean up the renderer and window
void cleanup_renderer(SDL_Renderer *renderer, SDL_Window *window) {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
}