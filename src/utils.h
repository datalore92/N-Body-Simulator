#ifndef UTILS_H
#define UTILS_H

#include <SDL2/SDL.h>

// Initialize random number generator
void init_random();

// Function to generate a random float between min and max
float random_float(float min, float max);

// Function to get the current time in milliseconds
Uint32 get_current_time();

#endif // UTILS_H