/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 * SPDX-FileContributor: Originally authored by David Higgins <https://github.com/zoul0813>
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_keyboard.h>
#include <zos_time.h>
#include <zvb_gfx.h>
#include <zvb_hardware.h>
#include <zos_video.h>
#include "assets.h"
#include "sounds.h"
#include "microbe.h"
#include "splash.h"
#include "utils.h"
#include "keyboard.h"
#include "controller.h"

gfx_context vctx;
Player player;
Bullet bullets[MAX_BULLETS];
uint8_t tiles[WIDTH * HEIGHT];
uint16_t invaders = 0;
uint16_t frames = 0;

int main(void) {
    init();

    Sound* sound = sound_play(SYSTEM_SOUND, 220, 3);
    msleep(75);
    sound_stop(sound);
    load_splash("press  start", get_splash_start());
    sound = sound_play(SYSTEM_SOUND, 440, 3);
    msleep(75);
    sound_stop(sound);

    reset(true);

    while(true) {
        sound_loop();
        uint8_t action = input();
        switch(action) {
            case ACTION_PAUSE: // start
                load_splash("   paused   ", NULL);
                continue;
            case ACTION_QUIT: // quit
                goto quit_game;
        }


        gfx_wait_vblank(&vctx);
        frames++;
        if(frames > 256) {
            frames = 0;
        }

        if(frames & 0x1) { // every other frame
            update();
            draw();
        }


        if(invaders == 0) {
            msleep(1000);
            next_level();
            reset(false);
        }

        if(player.lives < 1) {
            msleep(500);
            load_splash(" game  over ", NULL);
            msleep(250);
            reset(true);
        }

        gfx_wait_end_vblank(&vctx);
    }
quit_game:
    deinit();

    printf("Game complete\n");
    printf("Score: %d\n\n", player.score);

    return 0;
}

void init(void) {
    zos_err_t err = keyboard_init();
    if(err != ERR_SUCCESS) {
        printf("Failed to init keyboard: %d\n", err);
        exit(1);
    }
    err = controller_init();
    if(err != ERR_SUCCESS && err != ERR_NOT_SUPPORTED) {
        printf("Failed to init controller: %d\n", err);
    }

    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    if (err) exit(1);

    err = load_palette(&vctx);
    if(err) exit(1);

    gfx_tileset_options options = {
        .compression = TILESET_COMP_RLE,
        .from_byte = TILE_SIZE * 44, // 0x6100
    };
    err = load_numbers(&vctx, &options);
    if (err) exit(1);

    options.from_byte = TILE_SIZE * 97;
    err = load_letters(&vctx, &options);
    if (err) exit(1);

    options.from_byte = 0x8000; // 128
    err = load_tiles(&vctx, &options);
    if (err) exit(1);

    player.score = 0;
    player.level = 1;
    player.sprite.tile = PLAYER_TILE;
    player.sprite_index = 0;
    player.sprite.flags = SPRITE_BEHIND_FG;
    player.sprite.x = ((WIDTH / 2) * 16) - 8;
    player.sprite.y = 16 * 14;
    err = gfx_sprite_render(&vctx, player.sprite_index, &player.sprite);
    if (err) exit(1);

    gfx_enable_screen(1);

    sound_init();
    Sound *s_invader = sound_get(INVADER_SOUND);
    s_invader->waveform = WAV_NOISE;
}

void reset(uint8_t player_reset) {
    // Draw the tilemap
    load_tilemap(get_tilemap_start(), WIDTH, HEIGHT, INVADERS_LAYER);

    // Setup the player sprite
    player.sprite.x = ((WIDTH / 2) * 16) - 8;
    player.sprite.y = 16 * 14;
    player.sprite.flags = SPRITE_BEHIND_FG;
    player.direction = 0;
    player.lives = 3;

    gfx_error err = gfx_sprite_set_tile(&vctx, player.sprite_index, PLAYER_TILE);
    // TODO: error checking
    err = gfx_sprite_render(&vctx, player.sprite_index, &player.sprite);
    // TODO: error checking

    for(uint8_t i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
        bullets[i].direction = 1; // down
        bullets[i].sprite_index = i + 1;
        bullets[i].sprite.x = player.sprite.x + (i * 16);
        bullets[i].sprite.y = SCREEN_HEIGHT + SPRITE_HEIGHT; // offscreen
        bullets[i].sprite.flags = SPRITE_BEHIND_FG;
        bullets[i].sprite.tile = BULLET_TILE + i;

        err = gfx_sprite_set_tile(&vctx, bullets[i].sprite_index, BULLET_TILE);
        // TODO: error checking
        err = gfx_sprite_render(&vctx, bullets[i].sprite_index, &bullets[i].sprite);
        // TODO: error checking
    }
    // player bullet
    bullets[PLAYER_BULLET].direction = 0; // up

    if(player_reset) {
        player.level = 1;
        player.score = 0;
        player.lives = 3;
    }

    update_hud();
}

void deinit(void) {
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    sound_deinit();
    // TODO: clear sprites
    // TODO: clear tilesets

}

void load_tilemap(uint8_t* tilemap_start, uint16_t width, uint16_t height, uint8_t layer) {
    uint8_t line[WIDTH];
    uint8_t* tilemap = tilemap_start; // get_tilemap_start();

    // Load the tilemap
    for (uint16_t row = 0; row < height; row++) {
        uint16_t offset = row * width;
        for(uint16_t col = 0; col < width; col++) {
            line[col] = tilemap[offset + col] + TILEMAP_OFFSET;
        }
        memcpy(&tiles[offset], &line, width);
        gfx_tilemap_load(&vctx, line, width, layer, 0, row);
    }

    invaders = 0;
    for(uint16_t i = 0; i < width * height; i++) {
        uint8_t tile = tiles[i];
        if(tile > EMPTY_TILE) {
            invaders++;
        }
    }
}

uint8_t input(void) {
    uint16_t input = keyboard_read();
    input |= controller_read();

    player.direction = 0; // not moving
    if(input & SNES_LEFT) player.direction = DIRECTION_LEFT;
    if(input & SNES_RIGHT) player.direction = DIRECTION_RIGHT;
    if(input & SNES_B) {
        if(bullets[PLAYER_BULLET].active == 0) {
            bullets[PLAYER_BULLET].active = 1;
            bullets[PLAYER_BULLET].sprite.x = player.sprite.x;
            bullets[PLAYER_BULLET].sprite.y = player.sprite.y - 12;
            sound_play(PLAYER_SOUND, 180, 4);
        }
    }
    if(input & SNES_START ) return ACTION_PAUSE;
    if(input & (SNES_START | SNES_SELECT) ) return ACTION_QUIT;

    return ACTION_NONE;
}

void next_level(void) {
    player.level++;
}

void draw(void) {
    gfx_error err = GFX_SUCCESS; // TODO: return this?

    // faster to just update the `x` position
    err = gfx_sprite_set_x(&vctx, player.sprite_index, player.sprite.x);

    for(uint8_t i = 0; i < MAX_BULLETS; i++) {
        // faster to just memcpy the whole thing since we're doing quite a bit?
        err = gfx_sprite_render(&vctx, bullets[i].sprite_index, &bullets[i].sprite);
        // TODO: error checking
    }

    // TODO: error checking
}

void invader_shoot(uint8_t index) {
    // TODO: random select invader
    uint8_t rng = rand8_quick();
    if(rng > 0x80) return;
    rng &= 0x1F;
    while(rng > invaders) {
        rng >>= 1;
    }
    uint8_t invader = 0;
    for(uint8_t y = 0; y < HEIGHT; y++) {
        for(uint8_t x = 0; x < WIDTH; x++) {
            uint16_t offset = (y * WIDTH) + x;
            uint8_t tile = tiles[offset];
            if(tile > EMPTY_TILE) {
                invader++;
                if(invader >= rng) {
                    bullets[index].active = 1;
                    bullets[index].sprite.x = (x * 16) + 16;
                    bullets[index].sprite.y = (y * 16) + 16;
                    return;
                    // goto invader_shoot_done;
                }
            }
        }
    }
    // invader_shoot_done:
}

void update(void) {
    gfx_error err = GFX_SUCCESS; // TODO: return this?

    if(player.direction) {
        player.sprite.x += player.direction * PLAYER_SPEED;
    }

    if(player.sprite.x < 16) player.sprite.x = 16;
    if(player.sprite.x > 320) player.sprite.x = 320;

    if(frames == 31) {
        invader_shoot(1);
    }

    if(frames == 95) {
        invader_shoot(2);
    }

    if(frames == 143) {
        invader_shoot(3);
    }

    // TODO: animate the invaders
    // move the tilemap left/right to show the different invader frames?

    uint8_t index = MAX_BULLETS;
    for(index = 0; index < MAX_BULLETS; index++) {
    // while(index--) {
        // bullets[i].sprite.x = player.sprite.x + (i * 16);
        Bullet *bullet = &bullets[index];
        if(bullet->active == 0) continue;

        // move the bullet
        bullet->sprite.y += bullet->direction ? 2 : -2;

        uint16_t x = bullet->sprite.x;
        uint16_t y = bullet->sprite.y;
        if(index == 0) { // player bullet
            // convert x,y to tile position
            x = ((x + 8) >> 4) - 1;
            y = ((y + 8) >> 4) - 1;

            uint16_t offset = (y * WIDTH) + x;
            uint8_t tile = tiles[offset];
            // // TODO: error checking

            if(tile > EMPTY_TILE) {
                // found an invader
                tiles[offset] = EMPTY_TILE;
                gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x, y);
                // update the offscreen animation tile
                // gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x + WIDTH, y + HEIGHT);

                // move sprite offscreen
                bullet->sprite.y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                bullet->active = 0;

                // update score, decrement remaining invaders
                invaders--;
                player.score++;
                sound_play(INVADER_SOUND, 400, 7);
                update_hud();
            }
        } else { // invader bullet
            x += 8;
            if(x > player.sprite.x && x < player.sprite.x + 16) {
                if(y >= player.sprite.y - 16) {
                    bullet->active = 0;
                    bullet->sprite.y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                    player.lives--;
                    update_hud();
                }
            }
        }

        if(bullet->sprite.y < (SPRITE_HEIGHT/2) || bullet->sprite.y > SCREEN_HEIGHT) {
            // move sprite offscreen
            bullet->sprite.y = SCREEN_HEIGHT + SPRITE_HEIGHT;
            bullet->active = 0;
        }
    }
}

void update_hud(void) {
    char text[10];
    sprintf(text, "scr:%03d", player.score);
    nprint_string(&vctx, text, strlen(text), WIDTH - 7, HEIGHT - 1);

    sprintf(text, "lvl:%03d", player.level);
    nprint_string(&vctx, text, strlen(text), 0, HEIGHT - 1);

    uint8_t lives[3] = {EMPTY_TILE,EMPTY_TILE,EMPTY_TILE};
    for(uint8_t i = 0; i < sizeof(lives); i++) {
        if(i < player.lives) {
            lives[i] = 5U + TILEMAP_OFFSET;
        }
    }
    gfx_tilemap_load(&vctx, lives, 3, UI_LAYER, (WIDTH / 2) - 2, HEIGHT-1);
}