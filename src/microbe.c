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

#include <zgdk.h>
#include <zgdk/tilemap/scroll.h>

#include "assets.h"
#include "microbe.h"
#include "splash.h"

gfx_context vctx;
Player player;
Boss boss;
uint8_t boss_frame = 0;
Bullet bullets[MAX_BULLETS];
uint8_t tiles[WIDTH * (HEIGHT * 2)];
uint16_t invaders = 0;
uint16_t frames   = 0;

static gfx_sprite sprite_arena[SPRITE_ARENA_SIZE];
static uint8_t tilemap_x               = 0;
static int8_t tilemap_scroll_direction = 1;
static uint8_t tilemap_frame           = 0;

static gfx_sprite* register_sprite(uint8_t tile, uint16_t x, uint16_t y, uint8_t flags)
{
    gfx_sprite sprite;
    sprite.tile    = tile;
    sprite.x       = x;
    sprite.y       = y;
    sprite.flags   = flags;
    sprite.options = SPRITE_OPTION_NONE;
    return sprites_register(sprite);
}

void handle_error(zos_err_t err, const char* message, uint8_t fatal)
{
    if (err != ERR_SUCCESS) {
        if (fatal)
            deinit();
        printf("\nError[%d] (%02x) %s", err, err, message);
        if (fatal)
            exit(err);
    }
}

int main(void)
{
    init();

    Sound* sound = sound_play(SYSTEM_SOUND, 220, 0);
    msleep(75);
    sound_stop(sound);
    load_splash("press  start{|", get_splash_start());

    sound = sound_play(SYSTEM_SOUND, 440, 3);
    msleep(75);
    sound_stop(sound);
    sound->waveform = WAV_SAWTOOTH; // player hit

    reset(true);

    while (true) {
        sound_loop();
        uint8_t action = input();
        switch (action) {
            case ACTION_PAUSE: // start
                load_splash("    paused    ", NULL);
                continue;
            case ACTION_QUIT: // quit
                goto quit_game;
        }

        frames++;
        if (frames > SCREEN_HEIGHT)
            frames = 0;

        update();

        gfx_wait_vblank(&vctx);
        draw();
        gfx_wait_end_vblank(&vctx);

        if (player.lives < 1) {
            msleep(500);
            sound_stop_all();
            load_splash("  game  over  ", NULL);
            msleep(250);
            reset(true);
        } else if (invaders == 0 && (boss.active && boss.health == 0)) {
            msleep(1000);
            sound_stop_all();
            next_level();
            reset(false);
        }

        if (!boss.active && boss.health > 0 && invaders < 20) {
            boss.active = true;
            boss.tl->y  = SPRITE_HEIGHT;
            boss.tr->y  = boss.tl->y;
            boss.bl->y  = boss.tl->y + SPRITE_HEIGHT;
            boss.br->y  = boss.bl->y;
        }

    }
quit_game:
    deinit();

    printf("Game complete\n");
    printf("Score: %d\n\n", player.score);

    return 0;
}

void init(void)
{
    zos_err_t err = input_init(true);
    handle_error(err, "Failed to init input", 1);

    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    handle_error(err, "Failed to init graphics", 1);

    tilemap_fill(&vctx, LAYER1, EMPTY_TILE, 0, 0, 80, 40);

    err = load_palette(&vctx);
    handle_error(err, "Failed to load palette", 1);

    gfx_tileset_options options = {
        .compression = TILESET_COMP_RLE,
        .from_byte   = TILE_SIZE * 44, // 0x6100
    };
    err = load_numbers(&vctx, &options);
    handle_error(err, "Failed to load number tiles", 1);

    options.from_byte = TILE_SIZE * 97;
    err               = load_letters(&vctx, &options);
    handle_error(err, "Failed to load letter tiles", 1);

    ascii_map(0x20, 1, EMPTY_TILE);

    options.from_byte = 0x8000; // 128
    err               = load_tiles(&vctx, &options);
    handle_error(err, "Failed to load tiles", 1);

    err = sprites_register_arena(sprite_arena, SPRITE_ARENA_SIZE);
    handle_error(err, "Failed to initialize sprite arena", 1);

    player.score  = 0;
    player.level  = 1;
    player.sprite = register_sprite(PLAYER_TILE,
                                    ((WIDTH / 2) * SPRITE_WIDTH) - (SPRITE_WIDTH / 2),
                                    SPRITE_HEIGHT * 14,
                                    SPRITE_BEHIND_FG);
    handle_error(player.sprite ? ERR_SUCCESS : 1, "Failed to register player sprite", 1);

    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        bullets[i].sprite = register_sprite(BULLET_TILE,
                                            0,
                                            SCREEN_HEIGHT + SPRITE_HEIGHT,
                                            SPRITE_BEHIND_FG);
        handle_error(bullets[i].sprite ? ERR_SUCCESS : 1, "Failed to register bullet sprite", 1);
    }

    // BOSS INVADER
    boss.active    = false;
    boss.direction = 1;

    // Top Left
    boss.tl = register_sprite(BOSS_INVADER_TL1,
                              ((WIDTH / 2) * SPRITE_WIDTH) - (SPRITE_WIDTH / 2),
                              0,
                              SPRITE_NONE);
    // Top Right
    boss.tr = register_sprite(BOSS_INVADER_TR1, boss.tl->x + SPRITE_WIDTH, 0, SPRITE_NONE);
    // Bottom Left
    boss.bl = register_sprite(BOSS_INVADER_BL1, boss.tl->x, 0, SPRITE_NONE);
    // Bottom Right
    boss.br = register_sprite(BOSS_INVADER_BR1, boss.bl->x + SPRITE_WIDTH, 0, SPRITE_NONE);
    handle_error(boss.tl && boss.tr && boss.bl && boss.br ? ERR_SUCCESS : 1,
                 "Failed to register boss sprites", 1);

    err = sprites_render(&vctx);
    handle_error(err, "Failed to render sprite arena", 1);

    gfx_enable_screen(1);

    sound_init();
    sound_set(INVADER_SOUND, WAV_NOISE);
}

void reset(uint8_t player_reset)
{
    // Draw the tilemap
    load_tilemap(get_tilemap_start(), WIDTH, HEIGHT * 2, INVADERS_LAYER);

    // Setup the player sprite
    player.sprite->x     = ((WIDTH / 2) * SPRITE_WIDTH) - 8;
    player.sprite->y     = SPRITE_WIDTH * 14;
    player.sprite->flags = SPRITE_BEHIND_FG;
    player.direction    = 0;
    if (player_reset) {
        player.level = 1;
        player.score = 0;
        player.lives = 3;
    }

    player.sprite->tile = PLAYER_TILE;

    boss.health    = 3;
    boss.direction = 1;
    boss.active    = 0;

    boss.tl->y = SCREEN_HEIGHT + SPRITE_HEIGHT;
    boss.tr->y = boss.tl->y;
    boss.bl->y = boss.tl->y + SPRITE_HEIGHT;
    boss.br->y = boss.bl->y;

    boss.tl->tile = BOSS_INVADER_TL1;
    boss.tr->tile = BOSS_INVADER_TR1;
    boss.bl->tile = BOSS_INVADER_BL1;
    boss.br->tile = BOSS_INVADER_BR1;

    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active        = 0;
        bullets[i].direction     = 1; // down
        bullets[i].sprite->x     = player.sprite->x + (i * SPRITE_WIDTH);
        bullets[i].sprite->y     = SCREEN_HEIGHT + SPRITE_HEIGHT; // offscreen
        bullets[i].sprite->flags = SPRITE_BEHIND_FG;
        bullets[i].sprite->tile  = BULLET_TILE + i;
    }
    // player bullet
    bullets[PLAYER_BULLET].direction = 0; // up

    update_hud();
}

void deinit(void)
{
    tilemap_scroll(0, 0, 0);
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    sound_deinit();

    // TODO: clear tilesets

    sprites_deregister();
}

void load_tilemap(uint8_t* tilemap_start, uint16_t width, uint16_t height, uint8_t layer)
{
    uint8_t line[WIDTH];
    uint8_t* tilemap = tilemap_start; // get_tilemap_start();

    // Load the tilemap
    for (uint16_t row = 0; row < height; row++) {
        uint16_t offset = row * width;
        for (uint16_t col = 0; col < width; col++) {
            line[col] = tilemap[offset + col] + TILEMAP_OFFSET;
        }
        memcpy(&tiles[offset], &line, width);
        gfx_tilemap_load(&vctx, line, width, layer, 0, row);
    }

    invaders = 0;
    for (uint16_t i = 0; i < width * (height / 2); i++) {
        uint8_t tile = tiles[i];
        if (tile > EMPTY_TILE) {
            invaders++;
        }
    }
}

uint8_t input(void)
{
    uint16_t input = input_read();


    player.direction = 0; // not moving
    if (input & BUTTON_LEFT)
        player.direction = DIRECTION_LEFT;
    if (input & BUTTON_RIGHT)
        player.direction = DIRECTION_RIGHT;
    if (input & BUTTON_B) {
        if (bullets[PLAYER_BULLET].active == 0) {
            bullets[PLAYER_BULLET].active    = 1;
            bullets[PLAYER_BULLET].sprite->x = player.sprite->x;
            bullets[PLAYER_BULLET].sprite->y = player.sprite->y - 12;
            sound_play(PLAYER_SOUND, 180, 4);
        }
    }
    if (input & BUTTON_START)
        return ACTION_PAUSE;
    if (input & BUTTON_SELECT)
        return ACTION_QUIT;

    return ACTION_NONE;
}

void next_level(void)
{
    player.level++;
}

void draw(void)
{
    // tilemap offset
    tilemap_scroll(0, tilemap_x, tilemap_frame ? SCREEN_HEIGHT - 1 : 0);

    gfx_error err = sprites_render(&vctx);
    handle_error(err, "Failed to render sprite arena", 0);
}

void invader_shoot(uint8_t index)
{
    // TODO: random select invader
    uint8_t rng = rand8_quick();
    if (rng > 0x80)
        return;
    rng &= 0x1F;
    while (rng > invaders) {
        rng >>= 1;
    }
    uint8_t invader = 0;
    for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
            uint16_t offset = (y * WIDTH) + x;
            uint8_t tile    = tiles[offset];
            if (tile > EMPTY_TILE) {
                invader++;
                if (invader >= rng) {
                    bullets[index].active    = 1;
                    bullets[index].sprite->x = ((x * SPRITE_WIDTH) + SPRITE_WIDTH) - tilemap_x;
                    bullets[index].sprite->y = (y * SPRITE_WIDTH) + SPRITE_WIDTH;
                    return;
                    // goto invader_shoot_done;
                }
            }
        }
    }
    // invader_shoot_done:
}

void update(void)
{
    gfx_error err = GFX_SUCCESS; // TODO: return this?

    if (player.direction) {
        player.sprite->x += player.direction * PLAYER_SPEED;
    }

    if (player.sprite->x < SPRITE_WIDTH)
        player.sprite->x = SPRITE_WIDTH;
    if (player.sprite->x > SCREEN_WIDTH)
        player.sprite->x = SCREEN_WIDTH;

    if (frames == 31) {
        invader_shoot(1);
    }

    if (frames == 95) {
        invader_shoot(2);
    }

    if (frames == 143) {
        invader_shoot(3);
    }

    // animate the invaders
    // move the tilemap left/right to show the different invader frames?
    if ((frames & 0x07) == 0x07) {
        tilemap_x += tilemap_scroll_direction;
        if (tilemap_x == ((SPRITE_WIDTH * 2) - 1))
            tilemap_scroll_direction = -1;
        if (tilemap_x == 0)
            tilemap_scroll_direction = 1;
    }

    // toggle between the first and second frame of animation (top and bottom)
    if ((frames & 0x1F) == 0x1F) {
        tilemap_frame ^= 1; // toggle frame

        // always process, we'll only render when boss.active
        if (boss_frame == 0) {
            boss.tl->tile = BOSS_INVADER_TL1;
            boss.tr->tile = BOSS_INVADER_TR1;
            boss.bl->tile = BOSS_INVADER_BL1;
            boss.br->tile = BOSS_INVADER_BR1;
        } else {
            boss.tl->tile = BOSS_INVADER_TL2;
            boss.tr->tile = BOSS_INVADER_TR2;
            boss.bl->tile = BOSS_INVADER_BL2;
            boss.br->tile = BOSS_INVADER_BR2;
        }
        boss_frame ^= 1; // toggle
    }

    if ((boss.active && (frames & 0x01) == 0x01)) {
        boss.tl->x += boss.direction;
        boss.tr->x += boss.direction;
        boss.bl->x += boss.direction;
        boss.br->x += boss.direction;

        if (boss.tl->x < 32)
            boss.direction = 1;
        if (boss.tl->x > SCREEN_WIDTH - 48)
            boss.direction = -1;
        // if(boss.tl->x > 64) boss.direction = -1;
    }

    uint8_t index = MAX_BULLETS;
    for (index = 0; index < MAX_BULLETS; index++) {
        // while(index--) {
        // bullets[i].sprite->x = player.sprite->x + (i * SPRITE_WIDTH);
        Bullet* bullet = &bullets[index];
        if (bullet->active == 0)
            continue;

        // move the bullet
        bullet->sprite->y += bullet->direction ? 2 : -2;

        if (bullet->sprite->y < (SPRITE_HEIGHT / 2) || bullet->sprite->y > SCREEN_HEIGHT) {
            // move sprite offscreen
            bullet->sprite->y = SCREEN_HEIGHT + SPRITE_HEIGHT;
            bullet->active    = 0;
            continue; // moved off screen, inactive
        }

        uint16_t x = bullet->sprite->x;
        uint16_t y = bullet->sprite->y;
        if (index == 0) { // player bullet
            // offset the origin
            x += 8;
            y += 8;

            // did we hit the boss?
            if (boss.active && boss.health > 0) {
                if (x >= boss.tl->x && x <= boss.tr->x + SPRITE_WIDTH) {
                    if (y < (boss.bl->y + SPRITE_HEIGHT)) {
                        // HIT
                        boss.health--;
                        bullet->sprite->y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                        bullet->active    = 0;
                        if (boss.health == 0) {
                            player.score += 10;
                            boss.tl->y     = SCREEN_HEIGHT + SPRITE_HEIGHT;
                            boss.tr->y     = boss.tl->y;
                            boss.bl->y     = boss.tl->y + SPRITE_HEIGHT;
                            boss.br->y     = boss.tl->y + SPRITE_HEIGHT;
                            update_hud();
                            sound_play(PLAYER_SOUND, 220, 6);
                        } else {
                            sound_play(PLAYER_SOUND, 440, 6);
                        }
                        continue; // we hit the boss, stop processing
                    }
                }
            }


            // offset x to account for horizonal movement
            x += tilemap_x;
            // convert x,y to tile position
            x = (x >> 4) - 1;
            y = (y >> 4) - 1;

            if (x >= WIDTH || y >= HEIGHT)
                continue;

            uint16_t offset = (y * WIDTH) + x;
            uint8_t tile    = tiles[offset];
            // // TODO: error checking

            if (tile > EMPTY_TILE) {
                // found an invader
                tiles[offset] = EMPTY_TILE;
                gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x, y);
                gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x, y + HEIGHT);
                // update the offscreen animation tile
                // gfx_tilemap_place(&vctx, EMPTY_TILE, INVADERS_LAYER, x + WIDTH, y + HEIGHT);

                // move sprite offscreen
                bullet->sprite->y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                bullet->active    = 0;

                // update score, decrement remaining invaders
                invaders--;
                player.score++;
                sound_play(INVADER_SOUND, 400, 7);
                update_hud();
            }
        } else { // invader bullet
            x += 8;
            if (x > player.sprite->x && x < player.sprite->x + SPRITE_WIDTH) {
                if (y >= player.sprite->y - SPRITE_WIDTH) {
                    bullet->active    = 0;
                    bullet->sprite->y = SCREEN_HEIGHT + SPRITE_HEIGHT;
                    if(player.lives > 0) {
                        player.lives--;
                    }
                    sound_play(SYSTEM_SOUND, 360, 7);
                    update_hud();
                }
            }
        }
    }
}

void update_hud(void)
{
    char text[10];
    sprintf(text, "scr:%03d", player.score);
    nprint_string(&vctx, text, strlen(text), WIDTH - 7, HEIGHT - 1);

    sprintf(text, "lvl:%03d", player.level);
    nprint_string(&vctx, text, strlen(text), 0, HEIGHT - 1);

    uint8_t lives[3] = {EMPTY_TILE, EMPTY_TILE, EMPTY_TILE};
    for (uint8_t i = 0; i < DIM(lives); i++) {
        if (i < player.lives) {
            lives[i] = 5U + TILEMAP_OFFSET;
        }
    }
    gfx_tilemap_load(&vctx, lives, 3, UI_LAYER, (WIDTH / 2) - 2, HEIGHT - 1);
}
