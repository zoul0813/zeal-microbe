#pragma once

#include <stdint.h>
#include <zvb_gfx.h>

#define true  1
#define false 0

#define ACTION_NONE     0
#define ACTION_PAUSE    1
#define ACTION_CONTINUE 1
#define ACTION_QUIT     10

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define WIDTH         20
#define HEIGHT        15

#define TILEMAP_OFFSET 0x80U
#define EMPTY_TILE     0x7F

#define SYSTEM_SOUND 0

#define BULLET_TILE   0x81
#define MAX_BULLETS   4
#define PLAYER_BULLET 0
#define PLAYER_SOUND  1

#define PLAYER_TILE  0x80
#define PLAYER_SPEED 1

#define INVADERS_LAYER 0
#define INVADER_SOUND  3
#define UI_LAYER       (MAX_BULLETS - 1)

#define BOSS_INVADER_TL1 0x98
#define BOSS_INVADER_TR1 0x99
#define BOSS_INVADER_BL1 0xA8
#define BOSS_INVADER_BR1 0xA9
#define BOSS_INVADER_TL2 0x9E
#define BOSS_INVADER_TR2 0x9F
#define BOSS_INVADER_BL2 0xAE
#define BOSS_INVADER_BR2 0xAF

#define SPRITE_COUNT      (1 + MAX_BULLETS + 4)
#define SPRITE_ARENA_SIZE (SPRITE_COUNT + 1)

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t valid;
} ShotOrigin;

typedef struct {
    uint8_t invader_hits;
    uint8_t boss_hits;
    uint8_t boss_kills;
    uint8_t player_hits;
} BulletEvents;

extern gfx_context vctx;

void game_load_tilemap(uint8_t* tilemap_start, uint16_t width, uint16_t height, uint8_t layer);
uint8_t game_input(void);
