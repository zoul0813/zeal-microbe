#include <stdio.h>
#include <string.h>

#include <zgdk.h>

#include "game.h"
#include "player.h"

Player player;

zos_err_t player_init(void)
{
    gfx_sprite sprite;
    sprite.tile    = PLAYER_TILE;
    sprite.x       = ((WIDTH / 2) * SPRITE_WIDTH) - SPRITE_HALF;
    sprite.y       = SPRITE_HEIGHT * 14;
    sprite.flags   = SPRITE_BEHIND_FG;
    sprite.options = SPRITE_OPTION_NONE;
    player.sprite  = sprites_register(sprite);
    if (player.sprite)
        return ERR_SUCCESS;
    return 1;
}

void player_reset(uint8_t full_reset)
{
    player.sprite->x     = ((WIDTH / 2) * SPRITE_WIDTH) - SPRITE_HALF;
    player.sprite->y     = SPRITE_HEIGHT * 14;
    player.sprite->flags = SPRITE_BEHIND_FG;
    player.sprite->tile  = PLAYER_TILE;
    player.direction     = 0;

    if (full_reset) {
        player.level = 1;
        player.score = 0;
        player.lives = 3;
    }
}

void player_set_direction(int8_t direction)
{
    player.direction = direction;
}

void player_update(void)
{
    int16_t next_x = player.sprite->x;

    if (player.direction)
        next_x += player.direction * PLAYER_SPEED;

    if (next_x < SPRITE_WIDTH)
        next_x = SPRITE_WIDTH;
    if (next_x > SCREEN_WIDTH - SPRITE_WIDTH)
        next_x = SCREEN_WIDTH - SPRITE_WIDTH;

    player.sprite->x = next_x;
}

void player_damage(void)
{
    if (player.lives > 0)
        player.lives--;
}

void player_add_score(uint16_t points)
{
    player.score += points;
}

void player_next_level(void)
{
    player.level++;
}

void player_update_hud(void)
{
    char text[10];
    uint8_t lives[3] = {EMPTY_TILE, EMPTY_TILE, EMPTY_TILE};

    sprintf(text, "scr:%03d", player.score);
    nprint_string(&vctx, text, strlen(text), WIDTH - 7, HEIGHT - 1);

    sprintf(text, "lvl:%03d", player.level);
    nprint_string(&vctx, text, strlen(text), 0, HEIGHT - 1);

    for (uint8_t i = 0; i < DIM(lives); i++) {
        if (i < player.lives)
            lives[i] = 5U + TILEMAP_OFFSET;
    }
    gfx_tilemap_load(&vctx, lives, 3, UI_LAYER, (WIDTH / 2) - 2, HEIGHT - 1);
}
