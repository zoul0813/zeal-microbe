#pragma once

#include <stdint.h>
#include <zvb_gfx.h>

#include "game.h"

typedef struct {
    uint8_t active;
    uint8_t health;
    uint8_t frame;
    int8_t direction;
    gfx_sprite* tl;
    gfx_sprite* tr;
    gfx_sprite* bl;
    gfx_sprite* br;
} Boss;

extern Boss boss;

zos_err_t boss_init(void);
void boss_reset(void);
void boss_activate(void);
void boss_update(uint16_t frame);
uint8_t boss_hit(uint16_t x, uint16_t y, uint8_t width, uint8_t height);
ShotOrigin boss_shot_origin(void);
