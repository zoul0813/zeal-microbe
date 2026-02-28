cmake_minimum_required(VERSION 3.16)

# -------------------------------
# Toolchain
# -------------------------------
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)

# -------------------------------
# Project
# -------------------------------
project(microbe C ASM)

# -------------------------------
# Executable
# -------------------------------
add_executable(microbe
    src/microbe.c
    src/assets.c
    src/splash.c
)

# # -------------------------------
# # Include and link SDKs
# # -------------------------------
zos_use(zgdk)
zos_use(zvb)

find_package(ZVB_SDK REQUIRED)
target_link_libraries(microbe PUBLIC zvb_gfx zvb_sound)

find_package(ZGDK REQUIRED)
target_link_libraries(microbe PUBLIC zgdk)

# -------------------------------
# Assets
# -------------------------------

gif2zeal(microbe FILES
    "assets/letters.gif"
    "assets/numbers.gif"
    "assets/microbe.gif"
    "assets/tiles.gif"
    COMPRESSED)

tiled2zeal(microbe FILES
    "assets/splash.tmx"
    "assets/microbe.tmx"
    VERBOSE
    LAYER 0)


# -------------------------------
# Convert to binary
# -------------------------------
zos_add_outputs(microbe)
