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
#include "microbe.h"
#include "utils.h"
#include "keyboard.h"
#include "controller.h"

#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       240
#define SPRITE_HEIGHT       16
#define WIDTH               20
#define HEIGHT              15

#define TILEMAP_OFFSET      0x80
#define EMPTY_TILE          0x7F

#define BULLET_TILE         0x81
#define MAX_BULLETS         4

#define PLAYER_TILE         0x80
#define PLAYER_SPEED        1
#define PLAYER_BULLET       0

#define INVADERS_LAYER      0
#define UI_LAYER            MAX_BULLETS-1

gfx_context vctx;
Player player;
Bullet bullets[MAX_BULLETS];
uint8_t tiles[WIDTH * HEIGHT];
uint16_t invaders = 0;
static uint8_t controller_mode = 1;
static uint8_t frames = 0;


int main(void) {
    init();

    load_splash();

    reset();

    while (input() != 0) {
        gfx_wait_vblank(&vctx);
        frames++;
        if(frames > 128) {
            frames == 0;
        }

        update();

        draw();

        if(invaders == 0) {
            msleep(1500);
            player.level++;
            reset();
        }

        gfx_wait_end_vblank(&vctx);
    }

    deinit();

    printf("Game complete\n");
    printf("Score: %d\n\n", player.score);

    return 0; // unreachable
}

void init(void) {
    zos_err_t err = keyboard_init();
    if(err != ERR_SUCCESS) {
        printf("Failed to init keyboard: %d", err);
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

    gfx_enable_screen(1);
}

void reset(void) {
    // Draw the tilemap
    load_tilemap();

    // Setup the player sprite
    player.sprite.x = ((WIDTH / 2) * 16) - 8;
    player.sprite.y = 16 * 14;
    player.sprite.flags = SPRITE_NONE;
    player.direction = 0;

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
        bullets[i].sprite.flags = SPRITE_NONE;
        bullets[i].sprite.tile = BULLET_TILE + i;

        err = gfx_sprite_set_tile(&vctx, bullets[i].sprite_index, BULLET_TILE);
        // TODO: error checking
        err = gfx_sprite_render(&vctx, bullets[i].sprite_index, &bullets[i].sprite);
        // TODO: error checking
    }
    // player bullet
    bullets[PLAYER_BULLET].direction = 0; // up

}

void deinit(void) {
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
}

void load_tilemap(void) {
    uint8_t line[WIDTH];
    uint8_t* tilemap = get_tilemap_start();

    // Load the tilemap
    for (uint16_t row = 0; row < HEIGHT; row++) {
        uint16_t offset = row * WIDTH;
        for(uint16_t col = 0; col < WIDTH; col++) {
            line[col] = tilemap[offset + col] + TILEMAP_OFFSET;
        }
        memcpy(&tiles[offset], &line, WIDTH);
        gfx_tilemap_load(&vctx, line, WIDTH, INVADERS_LAYER, 0, row);
    }

    invaders = 0;
    for(uint16_t i = 0; i < WIDTH * HEIGHT; i++) {
        uint8_t tile = tiles[i];
        if(tile > EMPTY_TILE) {
            invaders++;
        }
    }
}

void load_splash(void) {
    uint8_t line[WIDTH];
    uint8_t* tilemap = get_splash_start();
    gfx_error err;
    // Load the tilemap
    for (uint16_t row = 0; row < HEIGHT; row++) {
        uint16_t offset = row * WIDTH;
        for(uint16_t col = 0; col < WIDTH; col++) {
            line[col] = tilemap[offset + col] + TILEMAP_OFFSET;
        }
        err = gfx_tilemap_load(&vctx, line, WIDTH, INVADERS_LAYER, 0, row);
        // TODO: error checking
    }

    // player.sprite.x = ((WIDTH / 2) * 16) - 8;
    // player.sprite.y = 16 * 14;
    // err = gfx_sprite_render(&vctx, player.sprite_index, &player.sprite);
    // // TODO: error checking

    char text[20];
    sprintf(text, "press  start");
    nprint_string(&vctx, text, strlen(text), 4, 13);

    msleep(250);
    // TODO: show "press start to begin"
    while(input() != 0) { } // wait for press
    msleep(100);
    while(input() == 0) { } // wait for release

    sprintf(text, "            ");
    nprint_string(&vctx, text, strlen(text), 4, 13);

    keyboard_flush(); // peace of mind
    controller_flush(); // peace of mind
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
        }
    }
    if(input & SNES_START ) return 0;

    return 255;
}

void draw(void) {
    gfx_error err = GFX_SUCCESS; // TODO: return this?

    // faster to just update the `x` position
    err = gfx_sprite_set_x(&vctx, player.sprite_index, player.sprite.x);

    for(uint8_t i = 0; i < MAX_BULLETS; i++) {
        // faster to just memcpy the whole thing since we're doing quite a bit?
        err = gfx_sprite_render(&vctx, bullets[i].sprite_index, &bullets[i].sprite);
    }

    char text[10];
    sprintf(text, "scr: %03d", player.score);
    nprint_string(&vctx, text, strlen(text), WIDTH - 8, HEIGHT - 1);

    sprintf(text, "lvl: %03d", player.level);
    nprint_string(&vctx, text, strlen(text), 0, HEIGHT - 1);

    // TODO: error checking
}

static void invader_shoot(uint8_t index) {
    bullets[index].active = 1;
    // TODO: random select invader
    uint8_t rng = (rand8() & 0x1F);
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

    if(frames == 16) {
        invader_shoot(1);
    }

    if(frames == 24) {
        invader_shoot(2);
    }

    if(frames == 40) {
        invader_shoot(3);
    }

    // TODO: animate the invaders
    // move the tilemap left/right to show the different invader frames?

    for(uint8_t i = 0; i < MAX_BULLETS; i++) {
        // bullets[i].sprite.x = player.sprite.x + (i * 16);
        if(bullets[i].active == 0) continue;

        // move the bullet
        bullets[i].sprite.y += bullets[i].direction ? 2 : -2;

        uint16_t x = bullets[i].sprite.x;
        uint16_t y = bullets[i].sprite.y;
        if(i > 0) {
            x += 8;
            if(x > player.sprite.x && x < player.sprite.x + 16) {
                if(y >= player.sprite.y - 16) {
                    bullets[i].active = 0;
                    bullets[i].sprite.y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                    bullets[i].sprite.x = rand8() + 32;
                }
            }
        } else {
            // convert x,y to tile position
            x = ((x + 8) >> 4) - 1;
            y = ((y + 8) >> 4) - 1;

            uint16_t offset = (y * WIDTH) + x;
            uint8_t tile = tiles[offset];
            // // TODO: error checking

            if(tile > EMPTY_TILE) {
                // found an invader
                gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x, y);
                tiles[offset] = EMPTY_TILE;
                invaders--;
                player.score++;
                // update the offscreen animation tile
                // gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x + WIDTH, y + HEIGHT);

                // move sprite offscreen
                bullets[i].sprite.y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                bullets[i].active = 0;
            }
        }

        if(bullets[i].sprite.y < (SPRITE_HEIGHT/2) || bullets[i].sprite.y > SCREEN_HEIGHT) {
            // move sprite offscreen
            bullets[i].sprite.y = SCREEN_HEIGHT + SPRITE_HEIGHT;
            bullets[i].active = 0;
        }
    }
}