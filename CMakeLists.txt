cmake_minimum_required(VERSION 3.10)
project(TopGunMaverick VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Platform-specific setup
if(APPLE)
    find_package(GLUT REQUIRED)
    include_directories(${GLUT_INCLUDE_DIRS})
    link_directories(${GLUT_LIBRARY_DIRS})
    add_definitions(${GLUT_DEFINITIONS})
    add_definitions(-DGL_SILENCE_DEPRECATION)
    set(PLATFORM_LIBS ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
elseif(WIN32)
    # Windows using freeglut
    find_package(GLUT REQUIRED)
    find_package(GLEW REQUIRED)
    set(PLATFORM_LIBS ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES})
else()
    # Linux
    find_package(GLUT REQUIRED)
    find_package(GLEW REQUIRED)
    set(PLATFORM_LIBS ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES} GL GLU)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${OPENGL_INCLUDE_DIRS}
)

# Source files - all cpp files in src and subdirectories
set(SOURCES
    src/main.cpp
    src/game/Game.cpp
    src/game/Level1.cpp
    src/entities/Player.cpp
    src/entities/Collectible.cpp
    src/entities/Obstacle.cpp
    src/rendering/Camera.cpp
    src/rendering/Lighting.cpp
    src/physics/Collision.cpp
    src/utils/Timer.cpp
    src/utils/Input.cpp
)

# Header files for IDE integration
set(HEADERS
    src/game/Game.h
    src/game/Level.h
    src/game/Level1.h
    src/entities/Player.h
    src/entities/Collectible.h
    src/entities/Obstacle.h
    src/rendering/Camera.h
    src/rendering/Lighting.h
    src/physics/Collision.h
    src/utils/Timer.h
    src/utils/Input.h
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})

# Platform-specific flags
if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    # Don't make it a bundle so it's easier to run from terminal
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE FALSE
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Print configuration
message(STATUS "=== Top Gun Maverick Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "Platform libs: ${PLATFORM_LIBS}")
