cmake_minimum_required(VERSION 3.10)

project(ParticlesDemo C)

set(CMAKE_C_STANDARD 11)

# Define SDL_MAIN_HANDLED to avoid SDL_main issues
add_definitions(-DSDL_MAIN_HANDLED)

# Find SDL2
find_package(SDL2 REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.c
    src/particle.c
    src/renderer.c
    src/utils.c
)

# Add executable
add_executable(ParticlesDemo ${SOURCES})

# Link libraries
target_link_libraries(ParticlesDemo ${SDL2_LIBRARIES} m)

# Windows-specific settings
if(WIN32)
    # Add 'm' library for math functions if needed
    target_link_libraries(ParticlesDemo m)
    
    # Copy SDL2.dll to output directory for easy running
    add_custom_command(TARGET ParticlesDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:ParticlesDemo>
    )
endif()