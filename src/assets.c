#include <stdio.h>
#include <zvb_gfx.h>
#include "assets.h"

gfx_error load_palette(gfx_context* ctx)
{
    // Load the palette
    const size_t palette_size = &_palette_end - &_palette_start;
    return gfx_palette_load(ctx, &_palette_start, palette_size, 0);
}

gfx_error load_tiles(gfx_context* ctx)
{
    // Load the tiles
    const size_t size = &_tiles_end - &_tiles_start;
    gfx_tileset_options options = {
        .compression = TILESET_COMP_NONE,
        .from_byte = (16 * 16) * 0x80, // 0x8000 ???
    };

    // sprites
    return gfx_tileset_load(ctx, &_tiles_start, size, &options);
}

gfx_error load_numbers(gfx_context* ctx)
{
    // Load the numbers
    const size_t size = &_numbers_end - &_numbers_start;
    gfx_tileset_options options = {
        .compression = TILESET_COMP_NONE,
        .from_byte = (16 * 16) * 44, // 0x2C00
    };

    // sprites
    return gfx_tileset_load(ctx, &_numbers_start, size, &options);
}

// gfx_error load_letters(gfx_context* ctx)
// {
//     // Load the leters
//     const size_t size = &_letters_end - &_letters_start;
//     gfx_tileset_options options = {
//         .compression = TILESET_COMP_NONE,
//         .from_byte = (16*16) * 97, // 0x6100
//     };

//     // sprites
//     return gfx_tileset_load(ctx, &_letters_start, size, &options);
// }

uint8_t* get_tilemap_start(void) {
    return &_tilemap_start;
}

uint8_t* get_tilemap_end(void) {
    return &_tilemap_end;
}

uint8_t* get_splash_start(void) {
    return &_splash_start;
}

uint8_t* get_splash_end(void) {
    return &_splash_end;
}


void __assets__(void) __naked
{
    __asm__(
        // shared palette
        "__palette_start:\n"
        "    .incbin \"assets/tiles.ztp\"\n"
        "__palette_end:\n"

        // tiles
        "__tiles_start:\n"
        "    .incbin \"assets/tiles.zts\"\n"
        "__tiles_end:\n"

        // numbers
        "__numbers_start:\n"
        "    .incbin \"assets/numbers.zts\"\n"
        "__numbers_end:\n"

        // // letters
        // "__letters_start:\n"
        // "    .incbin \"assets/letters.zts\"\n"
        // "__letters_end:\n"

        // tilemap
        "__tilemap_start:\n"
        "    .incbin \"assets/microbe.ztm\"\n"
        "__tilemap_end:\n"

        // splash
        "__splash_start:\n"
        "    .incbin \"assets/splash.ztm\"\n"
        "__splash_end:\n"
        //
    );
}