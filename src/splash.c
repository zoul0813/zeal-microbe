#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_time.h>

#include <zgdk.h>

#include "microbe.h"
#include "assets.h"
#include "splash.h"

void load_splash(const char* str, uint8_t* tilemap_start)
{
    if (tilemap_start != NULL) {
        load_tilemap(tilemap_start, WIDTH, HEIGHT, INVADERS_LAYER);
    }

    char text[20];
    sprintf(text, str);

    msleep(250);
    frames = 0;

    while (input() != ACTION_CONTINUE) {
        gfx_wait_vblank(&vctx);
        frames++;
        if (frames > 59)
            frames = 0;

        if (tilemap_start != NULL && frames % 30 == 0) {
            // animate logo
            if (boss_frame == 0) {
                gfx_tilemap_place(&vctx, BOSS_INVADER_TL1, INVADERS_LAYER, 11, 1);
                gfx_tilemap_place(&vctx, BOSS_INVADER_TR1, INVADERS_LAYER, 12, 1);
                gfx_tilemap_place(&vctx, BOSS_INVADER_BL1, INVADERS_LAYER, 11, 2);
                gfx_tilemap_place(&vctx, BOSS_INVADER_BR1, INVADERS_LAYER, 12, 2);
            } else {
                gfx_tilemap_place(&vctx, BOSS_INVADER_TL2, INVADERS_LAYER, 11, 1);
                gfx_tilemap_place(&vctx, BOSS_INVADER_TR2, INVADERS_LAYER, 12, 1);
                gfx_tilemap_place(&vctx, BOSS_INVADER_BL2, INVADERS_LAYER, 11, 2);
                gfx_tilemap_place(&vctx, BOSS_INVADER_BR2, INVADERS_LAYER, 12, 2);
            }
            boss_frame ^= 1; // toggle
        }

        if (frames > 29) {
            sprintf(text, "              ");
        } else {
            sprintf(text, str);
        }

        nprint_string(&vctx, text, strlen(text), 3, 11);
        gfx_wait_end_vblank(&vctx);
    } // wait for press
    msleep(100);
    while (input() != ACTION_NONE) {} // wait for release

    sprintf(text, "              ");
    nprint_string(&vctx, text, strlen(text), 3, 11);

    keyboard_flush();   // peace of mind
    controller_flush(); // peace of mind
}
