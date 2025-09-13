cmake_minimum_required(VERSION 3.16)
project(starship_game)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set the output directory for built objects
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Try to find SDL3 (system-installed first, then vendored)
find_package(SDL3 QUIET)

if(NOT SDL3_FOUND)
    # Check if vendored SDL3 exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL/CMakeLists.txt")
        add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
        message(STATUS "Using vendored SDL3")
    else()
        # Try pkg-config as fallback
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL3 REQUIRED sdl3)
        if(SDL3_FOUND)
            message(STATUS "Using system SDL3 via pkg-config")
        endif()
    endif()
endif()

if(SDL3_FOUND AND NOT TARGET SDL3::SDL3)
    # Create imported target for pkg-config found SDL3
    add_library(SDL3::SDL3 INTERFACE IMPORTED)
    target_include_directories(SDL3::SDL3 INTERFACE ${SDL3_INCLUDE_DIRS})
    target_link_libraries(SDL3::SDL3 INTERFACE ${SDL3_LIBRARIES})
    target_compile_options(SDL3::SDL3 INTERFACE ${SDL3_CFLAGS_OTHER})
endif()

# Include directories
include_directories(include)

# Collect all source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
if(TARGET SDL3::SDL3)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
    message(STATUS "Linked with SDL3::SDL3 target")
else()
    message(FATAL_ERROR "SDL3 not found! Please install SDL3 development libraries or provide vendored SDL3 source.")
endif()

# Handle SDL3_image (vendored first, then system)
set(SDL3_IMAGE_FOUND FALSE)

# Check for vendored SDL3_image first
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_image/CMakeLists.txt")
    # Set options for SDL3_image build
    set(SDL3IMAGE_BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared library")
    set(SDL3IMAGE_SAMPLES OFF CACHE BOOL "Build samples")
    set(SDL3IMAGE_TESTS OFF CACHE BOOL "Build tests")
    
    # Add vendored SDL3_image
    add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
    
    # Link with the static library
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_image::SDL3_image)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SDL_IMAGE=1)
    set(SDL3_IMAGE_FOUND TRUE)
    message(STATUS "Using vendored SDL3_image")
    
else()
    # Try to find system SDL3_image as fallback
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL3_IMAGE sdl3-image)
        if(SDL3_IMAGE_FOUND)
            target_include_directories(${PROJECT_NAME} PRIVATE ${SDL3_IMAGE_INCLUDE_DIRS})
            target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL3_IMAGE_LIBRARIES})
            target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SDL_IMAGE=1)
            message(STATUS "Using system SDL3_image via pkg-config")
        endif()
    endif()
endif()

if(NOT SDL3_IMAGE_FOUND)
    message(STATUS "SDL3_image not found - using BMP fallback only")
endif()
