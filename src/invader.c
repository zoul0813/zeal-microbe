#include <zgdk.h>

#include "invader.h"

InvaderFormation invaders;

static void descend(uint16_t player_y)
{
    uint8_t line[WIDTH];
    uint8_t game_over_row = (player_y >> 4) - 1;

    for (uint8_t frame = 0; frame < 2; frame++) {
        uint8_t frame_offset = frame * HEIGHT;

        for (int8_t row = HEIGHT - 2; row >= 0; row--) {
            uint8_t destination = frame_offset + row + 1;

            for (uint8_t col = 0; col < WIDTH; col++) {
                uint8_t tile = tilemap_get_xy(col, frame_offset + row);
                tilemap_set_xy(col, destination, tile);
                line[col] = tile;
                if (frame == 0 && row + 1 >= game_over_row && tile > EMPTY_TILE)
                    invaders.game_over = true;
            }
            gfx_tilemap_load(&vctx, line, WIDTH, INVADERS_LAYER, 0, destination);
        }

        for (uint8_t col = 0; col < WIDTH; col++) {
            tilemap_set_xy(col, frame_offset, EMPTY_TILE);
            line[col] = EMPTY_TILE;
        }
        gfx_tilemap_load(&vctx, line, WIDTH, INVADERS_LAYER, 0, frame_offset);
    }
}

void invader_reset(uint8_t* tilemap_start)
{
    invaders.scroll_x         = 0;
    invaders.animation_frame  = 0;
    invaders.scroll_direction = 1;
    invaders.game_over        = false;

    game_load_tilemap(tilemap_start, WIDTH, HEIGHT * 2, INVADERS_LAYER);

    invaders.count = 0;
    for (uint8_t row = 0; row < HEIGHT; row++) {
        for (uint8_t col = 0; col < WIDTH; col++) {
            if (tilemap_get_xy(col, row) > EMPTY_TILE)
                invaders.count++;
        }
    }
}

void invader_update(uint16_t frame, uint8_t level, uint16_t player_y)
{
    uint8_t interval = 8;

    if (level > 1) {
        uint8_t reduction = level - 1;
        if (reduction >= 6)
            interval = 2;
        else
            interval = 8 - reduction;
    }

    if (frame % interval == interval - 1) {
        invaders.scroll_x += invaders.scroll_direction;
        if (invaders.scroll_x == (SPRITE_WIDTH * 2) - 1) {
            invaders.scroll_direction = -1;
            descend(player_y);
        } else if (invaders.scroll_x == 0) {
            invaders.scroll_direction = 1;
            descend(player_y);
        }
    }

    if ((frame & 0x1F) == 0x1F)
        invaders.animation_frame ^= 1;
}

uint8_t invader_hit(uint16_t x, uint16_t y, uint8_t width, uint8_t height)
{
    int16_t first_col = ((x + invaders.scroll_x) >> 4) - 1;
    int16_t last_col  = ((x + width - 1 + invaders.scroll_x) >> 4) - 1;
    int16_t first_row = (y >> 4) - 1;
    int16_t last_row  = ((y + height - 1) >> 4) - 1;

    // A bullet can overlap at most four tiles.
    for (int16_t row = first_row; row <= last_row; row++) {
        if (row < 0 || row >= HEIGHT)
            continue;

        for (int16_t col = first_col; col <= last_col; col++) {
            if (col < 0 || col >= WIDTH || tilemap_get_xy(col, row) <= EMPTY_TILE)
                continue;

            tilemap_set_xy(col, row, EMPTY_TILE);
            tilemap_set_xy(col, row + HEIGHT, EMPTY_TILE);
            tilemap_place_xy(&vctx, INVADERS_LAYER, EMPTY_TILE, col, row);
            tilemap_place_xy(&vctx, INVADERS_LAYER, EMPTY_TILE, col, row + HEIGHT);
            invaders.count--;
            return true;
        }
    }
    return false;
}

ShotOrigin invader_shot_origin(void)
{
    ShotOrigin origin = {0, 0, false};

    if (invaders.count == 0)
        return origin;

    uint8_t start = rand8_quick() % WIDTH;
    for (uint8_t offset = 0; offset < WIDTH; offset++) {
        uint8_t col = (start + offset) % WIDTH;

        // Only the bottom-most invader in a column can shoot.
        for (int8_t row = HEIGHT - 1; row >= 0; row--) {
            if (tilemap_get_xy(col, row) > EMPTY_TILE) {
                int16_t x = ((uint16_t)(col + 1) * SPRITE_WIDTH) - invaders.scroll_x;
                if (x < 0)
                    origin.x = 0;
                else
                    origin.x = x;
                origin.y     = ((uint16_t)row + 1) * SPRITE_HEIGHT;
                origin.valid = true;
                return origin;
            }
        }
    }
    return origin;
}
