cmake_minimum_required(VERSION 3.10)

project(ParticlesDemo C)

set(CMAKE_C_STANDARD 11)

# Define SDL_MAIN_HANDLED to avoid SDL_main issues
add_definitions(-DSDL_MAIN_HANDLED)

# Source files
set(SOURCES
    src/main.c
    src/particle.c
    src/renderer.c
    src/utils.c
)

# Add executable
add_executable(ParticlesDemo ${SOURCES})

# Handle SDL2 differently based on platform
if(WIN32)
    # For Windows builds in GitHub Actions
    if(DEFINED ENV{GITHUB_ACTIONS})
        # Set include directories directly
        include_directories(include)
        
        # Add manual link directories
        link_directories(lib)
        
        # Link libraries directly
        target_link_libraries(ParticlesDemo 
            mingw32
            SDL2main
            SDL2
            m
        )
    else()
        # For local Windows builds
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIRS})
        target_link_libraries(ParticlesDemo ${SDL2_LIBRARIES} m)
    endif()
else()
    # For Linux/macOS builds
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(ParticlesDemo ${SDL2_LIBRARIES} m)
endif()