#include <stdio.h>
#include <zvb_gfx.h>

#include "assets.h"

gfx_error load_palette(gfx_context* ctx)
{
    // Load the palette
    const size_t palette_size = &_tiles_ztp_end - &_tiles_ztp_start;
    return gfx_palette_load(ctx, &_tiles_ztp_start, palette_size, 0);
}

gfx_error load_tiles(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_tiles_zts_end - &_tiles_zts_start;
    return gfx_tileset_load(ctx, &_tiles_zts_start, size, options);
}

gfx_error load_numbers(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_numbers_zts_end - &_numbers_zts_start;
    return gfx_tileset_load(ctx, &_numbers_zts_start, size, options);
}

gfx_error load_letters(gfx_context* ctx, gfx_tileset_options* options)
{
    const size_t size = &_letters_zts_end - &_letters_zts_start;
    return gfx_tileset_load(ctx, &_letters_zts_start, size, options);
}

uint8_t* get_tilemap_start(void)
{
    return &_microbe_ztm_start;
}

uint8_t* get_tilemap_end(void)
{
    return &_microbe_ztm_end;
}

uint8_t* get_splash_start(void)
{
    return &_splash_ztm_start;
}

uint8_t* get_splash_end(void)
{
    return &_splash_ztm_end;
}
