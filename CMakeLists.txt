cmake_minimum_required(VERSION 3.15)
project(CricketGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/external/rapidjson/include
    ${SDL2_INCLUDE_DIRS}
)

# GLAD source
set(GLAD_SOURCES
    external/glad/src/glad.c
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/Logger.cpp
    src/core/ConfigManager.cpp
    src/rendering/Renderer.cpp
    src/rendering/Shader.cpp
    src/rendering/Camera.cpp
    src/rendering/Mesh.cpp
    src/rendering/Texture.cpp
    src/rendering/ResourceManager.cpp
    src/physics/PhysicsEngine.cpp
    src/physics/Ball.cpp
    src/physics/CollisionDetector.cpp
    src/input/InputHandler.cpp
    src/gameplay/MatchManager.cpp
    src/gameplay/AIController.cpp
    src/gameplay/Player.cpp
    src/audio/AudioManager.cpp
    ${GLAD_SOURCES}
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    SDL2_mixer
)

# Compiler flags
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)
endif()

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
