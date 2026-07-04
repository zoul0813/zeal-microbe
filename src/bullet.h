#pragma once

#include <stdint.h>
#include <zvb_gfx.h>

#include "game.h"

zos_err_t bullet_init(void);
void bullet_reset(void);
uint8_t bullet_fire_player(void);
uint8_t bullet_fire_enemy(ShotOrigin origin);
BulletEvents bullet_update(void);
