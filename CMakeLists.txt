cmake_minimum_required(VERSION 3.16)

# -------------------------------
# Toolchain
# -------------------------------
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zos_init.cmake)
include("./zvb_tools.cmake")
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
zeal_use(zvb_gfx zvb_sound)
zeal_use(zgdk)

# -------------------------------
# Assets
# -------------------------------

gif2zeal(microbe FILES
    "assets/letters.gif"
    "assets/numbers.gif"
    COMPRESSED
    TILEMAP)

gif2zeal(microbe FILES
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
