/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// #include <stdint.h>
#include <zvb_gfx.h>
#include <zvb_sound.h>

#pragma once

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

#define BULLET_TILE  0x81
#define BULLET_SOUND 2
#define MAX_BULLETS  4

#define PLAYER_TILE   0x80
#define PLAYER_SPEED  1
#define PLAYER_BULLET 0
#define PLAYER_SOUND  1

#define INVADERS_LAYER 0
#define INVADER_SOUND  3
#define UI_LAYER       MAX_BULLETS - 1

#define BOSS_INDEX       32
#define BOSS_INVADER_TL1 0x98 // 24 + 0x80
#define BOSS_INVADER_TR1 0x99 // 25 + 0x80
#define BOSS_INVADER_BL1 0xA8 // 40 + 0x80
#define BOSS_INVADER_BR1 0xA9 // 41 + 0x80
#define BOSS_INVADER_TL2 0x9E // 29 + 0x80
#define BOSS_INVADER_TR2 0x9F // 30 + 0x80
#define BOSS_INVADER_BL2 0xAE // 45 + 0x80
#define BOSS_INVADER_BR2 0xAF // 46 + 0x80

void init(void);
void reset(uint8_t player_reset);
void deinit(void);
void load_tilemap(uint8_t* tilemap_start, uint16_t width, uint16_t height, uint8_t layer);

void draw(void);
void invader_shoot(uint8_t index);
void update(void);
void update_hud(void);
uint8_t input(void);

void next_level(void);

extern gfx_context vctx;

typedef struct {
        gfx_sprite sprite;
        uint8_t sprite_index;

        int8_t direction;
        uint16_t score;
        uint8_t level;
        uint8_t lives;
} Player;

typedef struct {
        uint8_t active;
        gfx_sprite sprite;
        uint8_t sprite_index;
        uint8_t direction;
} Bullet;

typedef struct {
        uint8_t active;
        uint8_t health;
        uint8_t sprite_index;
        int8_t direction;

        // sprites
        gfx_sprite tl;
        gfx_sprite tr;
        gfx_sprite bl;
        gfx_sprite br;
} Boss;

extern gfx_context vctx;
extern Player player;
extern Boss boss;
extern uint8_t boss_frame;
extern Bullet bullets[MAX_BULLETS];
extern uint8_t tiles[WIDTH * (HEIGHT * 2)];
extern uint16_t invaders;
extern uint16_t frames;