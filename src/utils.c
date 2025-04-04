#include <stdlib.h>
#include <time.h>
#include "utils.h"

void init_random() {
    srand((unsigned int)time(NULL));
}

float random_float(float min, float max) {
    return min + (float)rand() / ((float)RAND_MAX / (max - min));
}

Uint32 get_current_time() {
    return SDL_GetTicks();
}