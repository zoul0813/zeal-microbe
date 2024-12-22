#include <stdio.h>
#include <zvb_gfx.h>

#include <zgdk.h>

#include "assets.h"

gfx_error load_palette(gfx_context* ctx)
{
    // Load the palette
    const size_t palette_size = &_palette_end - &_palette_start;
    return gfx_palette_load(ctx, &_palette_start, palette_size, 0);
}

gfx_error load_tiles(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_tiles_end - &_tiles_start;
    return gfx_tileset_load(ctx, &_tiles_start, size, options);
}

gfx_error load_numbers(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_numbers_end - &_numbers_start;
    return gfx_tileset_load(ctx, &_numbers_start, size, options);
}

gfx_error load_letters(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_letters_end - &_letters_start;
    return gfx_tileset_load(ctx, &_letters_start, size, options);
}

uint8_t* get_tilemap_start(void)
{
    return &_tilemap_start;
}

uint8_t* get_tilemap_end(void)
{
    return &_tilemap_end;
}

uint8_t* get_splash_start(void)
{
    return &_splash_start;
}

uint8_t* get_splash_end(void)
{
    return &_splash_end;
}

void __assets__(void) __naked
{
    INCLUDE_ASSET("palette", "assets/tiles.ztp");
    INCLUDE_ASSET("tiles", "assets/tiles.zts");
    INCLUDE_ASSET("numbers", "assets/numbers.zts");
    INCLUDE_ASSET("letters", "assets/letters.zts");
    INCLUDE_ASSET("tilemap", "assets/microbe.ztm");
    INCLUDE_ASSET("splash", "assets/splash.ztm");
}