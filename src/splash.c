#include "microbe.h"
#include "splash.h"

void load_splash(void) {
    load_tilemap(get_splash_start(), WIDTH, HEIGHT, INVADERS_LAYER);

    player.sprite.x = ((WIDTH / 2) * 16) - 8;
    player.sprite.y = 16 * 14;
    gfx_error err = gfx_sprite_render(&vctx, player.sprite_index, &player.sprite);
    // // TODO: error checking

    char text[20];
    sprintf(text, "            ");

    msleep(250);
    frames = 0;
    uint8_t boss_frame = 0;

    while(input() != 0) {
        gfx_wait_vblank(&vctx);
        frames++;

        if(frames % 48 == 0) {
            // animate logo
            if(boss_frame == 0) {
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

        if(frames > 192) {
            sprintf(text, "            ");
            frames = 0;
        } else if (frames == 96) {
            sprintf(text, "press  start");
        }

        nprint_string(&vctx, text, strlen(text), 4, 11);
        gfx_wait_end_vblank(&vctx);
    } // wait for press
    msleep(100);
    while(input() == 0) { } // wait for release

    sprintf(text, "            ");
    nprint_string(&vctx, text, strlen(text), 4, 11);

    keyboard_flush(); // peace of mind
    controller_flush(); // peace of mind
}
