#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_time.h>

#include <zgdk.h>

#include "game.h"
#include "assets.h"
#include "splash.h"

void load_splash(const char* str, uint8_t* tilemap_start)
{
    if (tilemap_start != NULL) {
        game_load_tilemap(tilemap_start, WIDTH, HEIGHT, INVADERS_LAYER);
    }

    char text[20];
    strcpy(text, str);

    msleep(250);
    uint8_t frames = 0;
    uint8_t boss_frame = 0;

    while (game_input() != ACTION_CONTINUE) {
        gfx_wait_vblank(&vctx);
        frames++;
        if (frames > 59)
            frames = 0;

        if (tilemap_start != NULL && frames % 30 == 0) {
            // animate logo
            uint8_t tl;
            uint8_t tr;
            uint8_t bl;
            uint8_t br;

            if (boss_frame == 0) {
                tl = BOSS_INVADER_TL1;
                tr = BOSS_INVADER_TR1;
                bl = BOSS_INVADER_BL1;
                br = BOSS_INVADER_BR1;
            } else {
                tl = BOSS_INVADER_TL2;
                tr = BOSS_INVADER_TR2;
                bl = BOSS_INVADER_BL2;
                br = BOSS_INVADER_BR2;
            }

            tilemap_place_xy(&vctx, INVADERS_LAYER, tl, 11, 1);
            tilemap_place_xy(&vctx, INVADERS_LAYER, tr, 12, 1);
            tilemap_place_xy(&vctx, INVADERS_LAYER, bl, 11, 2);
            tilemap_place_xy(&vctx, INVADERS_LAYER, br, 12, 2);

            boss_frame ^= 1; // toggle
        }

        if (frames > 29) {
            strcpy(text, "              ");
        } else {
            strcpy(text, str);
        }

        nprint_string(&vctx, text, strlen(text), 3, 11);
        gfx_wait_end_vblank(&vctx);
    } // wait for press
    msleep(100);
    while (game_input() != ACTION_NONE) {} // wait for release

    strcpy(text, "              ");
    nprint_string(&vctx, text, strlen(text), 3, 11);

    input_flush();   // peace of mind
}
