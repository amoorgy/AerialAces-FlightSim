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
    # Windows using freeglut and GLEW
    find_package(GLUT REQUIRED)
    find_package(GLEW REQUIRED)
    
    # Include GLEW headers
    include_directories(${GLEW_INCLUDE_DIRS})
    
    set(PLATFORM_LIBS 
        ${OPENGL_LIBRARIES} 
        ${GLUT_LIBRARIES} 
        GLEW::GLEW
        opengl32
        glu32
    )
    
    # Copy DLLs to output directory for runtime
    if(GLUT_FOUND AND EXISTS "${GLUT_LIBRARY_DIRS}")
        file(GLOB GLUT_DLLS "${GLUT_LIBRARY_DIRS}/*.dll")
        foreach(dll ${GLUT_DLLS})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${dll} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
        endforeach()
    endif()
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
    src/rendering/Model.cpp
    src/rendering/Texture.cpp
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
    src/rendering/Model.h
    src/rendering/Texture.h
    src/rendering/tiny_obj_loader.h
    src/rendering/stb_image.h
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
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /EHsc)
    # Set subsystem to CONSOLE so we can see debug output
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
    # Disable specific MSVC warnings
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /wd4996  # Disable deprecation warnings
        /wd4244  # Disable conversion warnings
        /wd4267  # Disable size_t conversion warnings
    )
elseif(MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    # Link statically to avoid DLL dependencies
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
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
