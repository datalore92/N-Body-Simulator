#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "particle.h"

// Initializes the SDL renderer
int init_renderer(SDL_Renderer **renderer, SDL_Window **window, int width, int height);

// Clears the screen with a black background
void clear_renderer(SDL_Renderer *renderer);

// Renders a particle on the screen
void render_particle(SDL_Renderer *renderer, Particle *particle);

// Cleans up the renderer and window
void cleanup_renderer(SDL_Renderer *renderer, SDL_Window *window);

#endif // RENDERER_H