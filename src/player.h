#pragma once

#include <stdint.h>
#include <zvb_gfx.h>

typedef struct {
    gfx_sprite* sprite;
    int8_t direction;
    uint16_t score;
    uint8_t level;
    uint8_t lives;
} Player;

extern Player player;

zos_err_t player_init(void);
void player_reset(uint8_t full_reset);
void player_set_direction(int8_t direction);
void player_update(void);
void player_damage(void);
void player_add_score(uint16_t points);
void player_next_level(void);
void player_update_hud(void);
