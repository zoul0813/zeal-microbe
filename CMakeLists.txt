cmake_minimum_required(VERSION 3.16)

# -------------------------------
# Toolchain
# -------------------------------
set(ZOS_TOOLCHAIN sdcc)
include($ENV{ZOS_PATH}/cmake/zealos.cmake)

# -------------------------------
# Project
# -------------------------------
project(microbe C ASM)

# -------------------------------
# Copy assets BEFORE compiling
# -------------------------------
# This ensures that any .incbin in assets.c or other source files can find the files.
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets/ into build directory..."
)

# -------------------------------
# Executable
# -------------------------------
add_executable(microbe
    src/microbe.c
    src/assets.c
    src/splash.c
)

# Make sure microbe depends on copied assets
add_dependencies(microbe copy_assets)

# -------------------------------
# Include and link SDKs
# -------------------------------
target_include_directories(microbe PUBLIC
    $ENV{ZVB_SDK_PATH}/include
    $ENV{ZGDK_PATH}/include
    ${CMAKE_BINARY_DIR}         # for assets relative to build dir
)

target_link_libraries(microbe PUBLIC
    $ENV{ZVB_SDK_PATH}/lib/zvb_gfx.lib
    $ENV{ZGDK_PATH}/lib/zgdk.lib
)

# -------------------------------
# Convert to binary
# -------------------------------
set(MICROBE_IHX ${CMAKE_BINARY_DIR}/microbe.ihx)
ihx_to_bin(microbe)
