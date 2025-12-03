cmake_minimum_required(VERSION 3.10)
project(AerialAces VERSION 1.0.0)

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

# Source files
set(SOURCES
    src/main.cpp
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})

# Platform-specific flags
if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Print configuration
message(STATUS "=== Aerial Aces Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "Platform libs: ${PLATFORM_LIBS}")
