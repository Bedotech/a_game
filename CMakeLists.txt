cmake_minimum_required(VERSION 3.16)
project(starship_game)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Force static linking for all libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

# Static linking flags (platform-specific)
if(NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
endif()

# Set the output directory for built objects
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Force vendored SDL3 for static linking
set(SDL_SHARED OFF CACHE BOOL "Build SDL shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build SDL static library" FORCE)
set(SDL_HIDAPI OFF CACHE BOOL "Disable HIDAPI for static build" FORCE)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL/CMakeLists.txt")
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
    message(STATUS "Using vendored SDL3 (static)")
else()
    message(FATAL_ERROR "Vendored SDL3 not found! Run: git submodule update --init --recursive")
endif()

# Include directories
include_directories(include)

# Collect all source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Add game bridge for RL mode
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/agent/utils/game_bridge.c")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link with static SDL3
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-static)
message(STATUS "Linked with SDL3::SDL3-static")

# Link socket libraries for game bridge (RL mode)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# Handle SDL3_image - force static
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_image/CMakeLists.txt")
    # Set options for static SDL3_image build
    set(SDL3IMAGE_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared library" FORCE)
    set(SDL3IMAGE_SAMPLES OFF CACHE BOOL "Build samples" FORCE)
    set(SDL3IMAGE_TESTS OFF CACHE BOOL "Build tests" FORCE)
    set(SDL3IMAGE_VENDORED ON CACHE BOOL "Use vendored dependencies" FORCE)

    # Add vendored SDL3_image
    add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)

    # Link with the static library
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_image::SDL3_image-static)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SDL_IMAGE=1)
    message(STATUS "Using vendored SDL3_image (static)")
else()
    message(STATUS "SDL3_image not found - using BMP fallback only")
endif()

# Embed PNG assets into the executable
file(GLOB ASSET_FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.png")
foreach(ASSET_FILE ${ASSET_FILES})
    get_filename_component(ASSET_NAME ${ASSET_FILE} NAME)
    string(REPLACE "." "_" ASSET_VAR_NAME ${ASSET_NAME})

    # Generate C file with embedded asset data
    file(READ ${ASSET_FILE} ASSET_HEX HEX)
    string(LENGTH ${ASSET_HEX} ASSET_HEX_LEN)
    math(EXPR ASSET_SIZE "${ASSET_HEX_LEN} / 2")

    # Convert hex string to C array
    set(ASSET_C_FILE "${CMAKE_CURRENT_BINARY_DIR}/embedded_${ASSET_VAR_NAME}.c")
    file(WRITE ${ASSET_C_FILE} "// Auto-generated embedded asset: ${ASSET_NAME}\n")
    file(APPEND ${ASSET_C_FILE} "#include <stddef.h>\n")
    file(APPEND ${ASSET_C_FILE} "const unsigned char embedded_${ASSET_VAR_NAME}_data[] = {\n")

    string(REGEX MATCHALL ".." ASSET_BYTES ${ASSET_HEX})
    set(BYTE_COUNT 0)
    foreach(BYTE ${ASSET_BYTES})
        if(BYTE_COUNT EQUAL 0)
            file(APPEND ${ASSET_C_FILE} "    ")
        endif()
        file(APPEND ${ASSET_C_FILE} "0x${BYTE}, ")
        math(EXPR BYTE_COUNT "${BYTE_COUNT} + 1")
        if(BYTE_COUNT EQUAL 12)
            file(APPEND ${ASSET_C_FILE} "\n")
            set(BYTE_COUNT 0)
        endif()
    endforeach()

    file(APPEND ${ASSET_C_FILE} "\n};\n")
    file(APPEND ${ASSET_C_FILE} "const size_t embedded_${ASSET_VAR_NAME}_size = ${ASSET_SIZE};\n")

    # Add generated file to sources
    target_sources(${PROJECT_NAME} PRIVATE ${ASSET_C_FILE})

    message(STATUS "Embedding asset: ${ASSET_NAME} (${ASSET_SIZE} bytes)")
endforeach()

# Copy assets to build directory for runtime loading fallback
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
