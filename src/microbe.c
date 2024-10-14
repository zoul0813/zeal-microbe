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
Boss boss;
uint8_t boss_frame = 0;
Bullet bullets[MAX_BULLETS];
uint8_t tiles[WIDTH * HEIGHT];
uint16_t invaders = 0;
uint16_t frames = 0;

static uint8_t controller_mode = 1;
static uint8_t tilemap_x = 0;
static int8_t tilemap_scroll_direction = 1;
static uint8_t tilemap_frame = 0;

int main(void) {
    init();

    Sound* sound = sound_play(SYSTEM_SOUND, 220, 3);
    msleep(75);
    sound_stop(sound);
    if(controller_mode) {
        load_splash("press  start{|", get_splash_start());
    } else {
        load_splash(" press  start ", get_splash_start());
    }
    sound = sound_play(SYSTEM_SOUND, 440, 3);
    msleep(75);
    sound_stop(sound);
    sound->waveform = WAV_SAWTOOTH; // player hit

    reset(true);

    while(true) {
        DEBUG_COUNT(2);
        sound_loop();
        uint8_t action = input();
        switch(action) {
            case ACTION_PAUSE: // start
                load_splash("    paused    ", NULL);
                continue;
            case ACTION_QUIT: // quit
                goto quit_game;
        }

        frames++;
        if(frames > 240) frames = 0;

        DEBUG_COUNT(4);
        update();
        DEBUG_COUNT(4);

        gfx_wait_vblank(&vctx);
        DEBUG_COUNT(1);
        DEBUG_COUNT(3);
        draw();
        DEBUG_COUNT(3);
        gfx_wait_end_vblank(&vctx);
        DEBUG_COUNT(1);

        if(invaders == 0) {
            msleep(1000);
            sound_stop_all();
            next_level();
            reset(false);
        }

        if(boss.health > 0) boss.active = true; // TODO: remove
        if(boss.health > 0 && invaders < 20) {
            boss.active = true;
        }

        if(player.lives < 1) {
            msleep(500);
            sound_stop_all();
            load_splash("  game  over  ", NULL);
            msleep(250);
            reset(true);
        }
        DEBUG_COUNT(2);
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
    if(err != ERR_SUCCESS) {
        printf("Failed to init controller: %d", err);
    }
    // verify the controller is actually connected
    uint16_t test = controller_read();
    // if unconnected, we'll get back 0xFFFF (all buttons pressed)
    if(test & 0xFFFF) {
        controller_mode = 0;
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
    player.sprite.x = ((WIDTH / 2) * SPRITE_WIDTH) - (SPRITE_WIDTH/2);
    player.sprite.y = SPRITE_HEIGHT * 14;
    err = gfx_sprite_render(&vctx, player.sprite_index, &player.sprite);
    if (err) exit(1);

    // BOSS INVADER
    boss.active = false;
    boss.direction = 1;
    boss.sprite_index = BOSS_INDEX;

    // Top Left
    boss.tl.tile = BOSS_INVADER_TL1;
    boss.tl.x = ((WIDTH / 2) * SPRITE_WIDTH) - (SPRITE_WIDTH/2);
    boss.tl.y = 16;
    // Top Right
    boss.tr.tile = BOSS_INVADER_TR1;
    boss.tr.x = boss.tl.x + SPRITE_WIDTH;
    boss.tr.y = boss.tl.y;
    // Bottom Left
    boss.bl.tile = BOSS_INVADER_BL1;
    boss.bl.x = boss.tl.x;
    boss.bl.y = boss.tl.y + SPRITE_HEIGHT;
    // Bottom Right
    boss.br.tile = BOSS_INVADER_BR1;
    boss.br.x = boss.bl.x + SPRITE_WIDTH;
    boss.br.y = boss.bl.y;

    gfx_enable_screen(1);

    sound_init();
    Sound *s_invader = sound_get(INVADER_SOUND);
    s_invader->waveform = WAV_NOISE;
}

void reset(uint8_t player_reset) {
    // Draw the tilemap
    load_tilemap(get_tilemap_start(), WIDTH, HEIGHT * 2, INVADERS_LAYER);

    // Setup the player sprite
    player.sprite.x = ((WIDTH / 2) * 16) - 8;
    player.sprite.y = 16 * 14;
    player.sprite.flags = SPRITE_BEHIND_FG;
    player.direction = 0;
    player.lives = 3;

    gfx_error err;
    err = gfx_sprite_set_tile(&vctx, player.sprite_index, PLAYER_TILE);
    // TODO: error checking
    err = gfx_sprite_render(&vctx, player.sprite_index, &player.sprite);
    // TODO: error checking

    boss.health = 3;
    boss.direction = 1;
    boss.active = 0;
    err = gfx_sprite_set_tile(&vctx, boss.sprite_index, BOSS_INVADER_TL1);
    // TODO: error checking
    err = gfx_sprite_set_tile(&vctx, boss.sprite_index+1, BOSS_INVADER_TR1);
    // TODO: error checking
    err = gfx_sprite_set_tile(&vctx, boss.sprite_index+2, BOSS_INVADER_BL1);
    // TODO: error checking
    err = gfx_sprite_set_tile(&vctx, boss.sprite_index+3, BOSS_INVADER_BR1);
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
    zvb_ctrl_l0_scr_x_low = 0;
    zvb_ctrl_l0_scr_x_high = 0;
    zvb_ctrl_l0_scr_y_low = 0;
    zvb_ctrl_l0_scr_y_high = 0;
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    sound_deinit();

    // TODO: clear tilesets

    // TODO: clear sprites
    gfx_error err;
    err = gfx_sprite_set_tile(&vctx, player.sprite_index, EMPTY_TILE);
    // TODO: error checking
    err = gfx_sprite_set_tile(&vctx, boss.sprite_index, EMPTY_TILE);
    // TODO: error checking
    err = gfx_sprite_set_tile(&vctx, boss.sprite_index+1, EMPTY_TILE);
    // TODO: error checking
    err = gfx_sprite_set_tile(&vctx, boss.sprite_index+2, EMPTY_TILE);
    // TODO: error checking
    err = gfx_sprite_set_tile(&vctx, boss.sprite_index+3, EMPTY_TILE);
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
    for(uint16_t i = 0; i < width * (height/2); i++) {
        uint8_t tile = tiles[i];
        if(tile > EMPTY_TILE) {
            invaders++;
        }
    }
}

uint8_t input(void) {
    uint16_t input = keyboard_read();
    if(controller_mode == 1) {
        input |= controller_read();
    }


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

    // tilemap offset
    zvb_ctrl_l0_scr_x_low = tilemap_x;
    zvb_ctrl_l0_scr_x_high = 0;
    zvb_ctrl_l0_scr_y_low = tilemap_frame ? SCREEN_HEIGHT - 1 : 0;
    zvb_ctrl_l0_scr_y_high = 0;

    // faster to just update the `x` position
    err = gfx_sprite_set_x(&vctx, player.sprite_index, player.sprite.x);
    // TODO: error checking?

    err = gfx_sprite_render(&vctx, boss.sprite_index, &boss.tl);
    err = gfx_sprite_render(&vctx, boss.sprite_index+1, &boss.tr);
    err = gfx_sprite_render(&vctx, boss.sprite_index+2, &boss.bl);
    err = gfx_sprite_render(&vctx, boss.sprite_index+3, &boss.br);

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
                    bullets[index].sprite.x = ((x * 16) + 16) - tilemap_x;;
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

#ifndef EMULATOR
    // animate the invaders
    // move the tilemap left/right to show the different invader frames?
    if((frames & 0x07) == 0x07) {
        tilemap_x += tilemap_scroll_direction;
        if(tilemap_x == ((SPRITE_WIDTH * 2) - 1)) tilemap_scroll_direction = -1;
        if(tilemap_x == 0) tilemap_scroll_direction = 1;

    }
#endif
    // toggle between the first and second frame of animation (top and bottom)
    if((frames & 0x1F) == 0x1F) {
#ifndef EMULATOR
        tilemap_frame ^= 1; // toggle frame
#endif

        // always process, we'll only render when boss.active
        if(boss_frame == 0) {
            boss.tl.tile = BOSS_INVADER_TL1;
            boss.tr.tile = BOSS_INVADER_TR1;
            boss.bl.tile = BOSS_INVADER_BL1;
            boss.br.tile = BOSS_INVADER_BR1;
        } else {
            boss.tl.tile = BOSS_INVADER_TL2;
            boss.tr.tile = BOSS_INVADER_TR2;
            boss.bl.tile = BOSS_INVADER_BL2;
            boss.br.tile = BOSS_INVADER_BR2;
        }
        boss_frame ^= 1; // toggle
    }

    if((boss.active && (frames & 0x01) == 0x01)) {
        boss.tl.x += boss.direction;
        boss.tr.x += boss.direction;
        boss.bl.x += boss.direction;
        boss.br.x += boss.direction;

        if(boss.tl.x < 32) boss.direction = 1;
        if(boss.tl.x > SCREEN_WIDTH - 48) boss.direction = -1;
        // if(boss.tl.x > 64) boss.direction = -1;
    }

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
            // offset the origin
            x += 8;
            y += 8;

            // did we hit the boss?
            if(boss.active) {
                if(x >= boss.tl.x && x <= boss.tr.x) {
                    if(y < (boss.bl.y + SPRITE_HEIGHT)) {
                        // HIT
                        boss.health--;
                        bullet->sprite.y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                        bullet->active = 0;
                        if(boss.health == 0) {
                            player.score += 10;
                            boss.active = false;
                            boss.tl.y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                            boss.tr.y = boss.tl.y;
                            boss.bl.y = boss.tl.y + SPRITE_HEIGHT;
                            boss.br.y = boss.tl.y + SPRITE_HEIGHT;
                            update_hud();
                            sound_play(PLAYER_SOUND, 220, 6);
                        } else {
                            sound_play(PLAYER_SOUND, 440, 6);
                        }
                    }
                }
            }


            // offset x to account for horizonal movement
            x += tilemap_x;
            // convert x,y to tile position
            x = (x >> 4) - 1;
            y = (y >> 4) - 1;

            uint16_t offset = (y * WIDTH) + x;
            uint8_t tile = tiles[offset];
            // // TODO: error checking

            if(tile > EMPTY_TILE) {
                // found an invader
                tiles[offset] = EMPTY_TILE;
                gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x, y);
                gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x, y + HEIGHT);
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
                    sound_play(SYSTEM_SOUND, 360, 7);
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