#pragma once

#include <stdint.h>

#include "game.h"

typedef struct {
    uint16_t count;
    uint8_t scroll_x;
    uint8_t animation_frame;
    int8_t scroll_direction;
    uint8_t game_over;
} InvaderFormation;

extern InvaderFormation invaders;

void invader_reset(uint8_t* tilemap_start);
void invader_update(uint16_t frame, uint8_t level, uint16_t player_y);
uint8_t invader_hit(uint16_t x, uint16_t y, uint8_t width, uint8_t height);
ShotOrigin invader_shot_origin(void);
