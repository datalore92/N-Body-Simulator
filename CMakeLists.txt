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
        # Include directories - Just C:/SDL2/include without SDL2 since we set it up this way
        include_directories(C:/SDL2/include)
        
        # Link directories
        link_directories(C:/SDL2/lib)
        
        # Link libraries directly without imported targets
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
    
    # Only add this command for local builds, not for GitHub Actions
    if(NOT DEFINED ENV{GITHUB_ACTIONS})
        # Copy SDL2.dll to output directory for easy running (if we can find it)
        find_file(SDL2_DLL SDL2.dll)
        if(SDL2_DLL)
            add_custom_command(TARGET ParticlesDemo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${SDL2_DLL}
                $<TARGET_FILE_DIR:ParticlesDemo>
            )
        endif()
    endif()
else()
    # For Linux/macOS builds
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    target_link_libraries(ParticlesDemo ${SDL2_LIBRARIES} m)
endif()