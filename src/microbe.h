/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// #include <stdint.h>
#include <zvb_gfx.h>

#pragma once

#define DIRECTION_LEFT  -1
#define DIRECTION_RIGHT 1

#define TILE_SIZE       (16 * 16)

void init(void);
void reset(void);
void deinit(void);
void load_tilemap(void);
void load_splash(void);
void draw(void);
void update(void);
uint8_t input(void);

extern gfx_context vctx;

typedef struct {
    gfx_sprite sprite;
    uint8_t sprite_index;

    int8_t direction;
    uint16_t score;
    uint8_t level;
} Player;

typedef struct {
    uint8_t active;
    gfx_sprite sprite;
    uint8_t sprite_index;
    uint8_t direction;
} Bullet;

