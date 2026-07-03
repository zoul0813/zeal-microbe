cmake_minimum_required(VERSION 3.16)

# Application Version
set(APP_VERSION_MAJOR 0)
set(APP_VERSION_MINOR 2)
set(APP_VERSION_PATCH 0)
set(APP_NAME "Microbe")
set(target_name "microbe")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# -------------------------------
# Toolchain
# -------------------------------
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

# -------------------------------
# Project
# -------------------------------
project(${target_name} C ASM)

# -------------------------------
# Executable
# -------------------------------
add_executable(${target_name}
    src/microbe.c
    src/assets.c
    src/splash.c
)

# # -------------------------------
# # Include and link SDKs
# # -------------------------------
zos_use(zvb)
find_package(ZVB_SDK REQUIRED)
target_link_libraries(${target_name} PUBLIC zvb_gfx zvb_sound)

zos_use(zgdk)
find_package(ZGDK REQUIRED)
target_link_libraries(${target_name} PUBLIC zgdk)

zos_use(zar)
find_package(ZAR REQUIRED)
target_link_libraries(${target_name} PUBLIC zar)

# -------------------------------
# Assets
# -------------------------------
gif2zeal(${target_name} FILES
    "assets/letters.gif"
    "assets/numbers.gif"
    "assets/microbe.gif"
    "assets/tiles.gif"
    COMPRESSED)

tiled2zeal(${target_name} FILES
    "assets/splash.tmx"
    "assets/microbe.tmx"
    VERBOSE
    LAYER 0)

# -------------------------------
# Convert to binary
# -------------------------------
zos_add_outputs(${target_name})
