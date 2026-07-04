#include <stdio.h>
#include <stdint.h>
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
#include "boss.h"
#include "bullet.h"
#include "game.h"
#include "invader.h"
#include "player.h"
#include "splash.h"

gfx_context vctx;
static gfx_sprite sprite_arena[SPRITE_ARENA_SIZE];
static uint8_t tiles[WIDTH * (HEIGHT * 2)];
static Tilemap game_tilemap;
static uint16_t frames;
static uint8_t alternate_enemy_source;

static void deinit(void);

static void handle_error(zos_err_t err, const char* message, uint8_t fatal)
{
    if (err != ERR_SUCCESS) {
        if (fatal)
            deinit();
        printf("\nError[%d] (%02x) %s", err, err, message);
        if (fatal)
            exit(err);
    }
}

void game_load_tilemap(uint8_t* tilemap_start, uint16_t width, uint16_t height, uint8_t layer)
{
    uint8_t line[WIDTH];

    // Load the tilemap
    for (uint16_t row = 0; row < height; row++) {
        uint16_t offset = row * width;

        for (uint16_t col = 0; col < width; col++) {
            uint8_t tile = tilemap_start[offset + col] + TILEMAP_OFFSET;
            tilemap_set_xy(col, row, tile);
            line[col] = tile;
        }
        gfx_tilemap_load(&vctx, line, width, layer, 0, row);
    }
}

uint8_t game_input(void)
{
    uint16_t input = input_read();

    player_set_direction(0);
    if (input & BUTTON_LEFT)
        player_set_direction(DIRECTION_LEFT);
    if (input & BUTTON_RIGHT)
        player_set_direction(DIRECTION_RIGHT);
    if (input & BUTTON_B) {
        if (bullet_fire_player())
            sound_play(PLAYER_SOUND, 180, 4);
    }
    if (input & BUTTON_START)
        return ACTION_PAUSE;
    if (input & BUTTON_SELECT)
        return ACTION_QUIT;
    return ACTION_NONE;
}

static void init(void)
{
    zos_err_t err = input_init(true);
    gfx_tileset_options options = {
        .compression = TILESET_COMP_RLE,
        .from_byte   = TILE_BYTE_OFFSET(TILESET_8BIT, 44)
    };

    handle_error(err, "Failed to init input", true);

    /* Disable the screen to prevent artifacts from showing */
    gfx_enable_screen(0);

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    handle_error(err, "Failed to init graphics", true);

    game_tilemap.rect.x = 0;
    game_tilemap.rect.y = 0;
    game_tilemap.rect.w = WIDTH;
    game_tilemap.rect.h = HEIGHT * 2;
    err                 = tilemap_register(&game_tilemap, tiles);
    handle_error(err, "Failed to register tilemap", true);

    tilemap_fill(&vctx, LAYER1, EMPTY_TILE, 0, 0, 80, 40);
    err = load_palette(&vctx);
    handle_error(err, "Failed to load palette", true);

    err = load_numbers(&vctx, &options);
    handle_error(err, "Failed to load number tiles", true);

    options.from_byte = TILE_BYTE_OFFSET(TILESET_8BIT, 97);
    err               = load_letters(&vctx, &options);
    handle_error(err, "Failed to load letter tiles", true);
    ascii_map(0x20, 1, EMPTY_TILE);

    options.from_byte = 0x8000; // 128
    err               = load_tiles(&vctx, &options);
    handle_error(err, "Failed to load tiles", true);

    err = sprites_register_arena(sprite_arena, SPRITE_ARENA_SIZE);
    handle_error(err, "Failed to initialize sprite arena", true);
    handle_error(player_init(), "Failed to register player sprite", true);
    handle_error(bullet_init(), "Failed to register bullet sprites", true);
    handle_error(boss_init(), "Failed to register boss sprites", true);

    err = sprites_render(&vctx);
    handle_error(err, "Failed to render sprite arena", true);
    gfx_enable_screen(1);

    sound_init();
    sound_set(INVADER_SOUND, WAV_NOISE);
}

static void reset(uint8_t full_reset)
{
    frames                 = 0;
    alternate_enemy_source = false;
    player_reset(full_reset);
    invader_reset(get_tilemap_start());
    boss_reset();
    bullet_reset();
    player_update_hud();
}

static void deinit(void)
{
    tilemap_scroll(0, 0, 0);
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    sound_deinit();
    sprites_deregister();
}

static uint8_t enemy_fire_interval(void)
{
    uint8_t level = player.level;
    uint8_t reduction;

    if (level <= 1)
        return 64;
    if (level >= 11)
        return 24;
    reduction = (level - 1) * 4;
    return 64 - reduction;
}

static void fire_enemy(void)
{
    ShotOrigin origin;

    if (boss.active && (alternate_enemy_source || invaders.count == 0))
        origin = boss_shot_origin();
    else
        origin = invader_shot_origin();

    if (!origin.valid && boss.active)
        origin = boss_shot_origin();
    if (!origin.valid)
        origin = invader_shot_origin();

    bullet_fire_enemy(origin);
    alternate_enemy_source ^= 1;
}

static void update(void)
{
    BulletEvents events;
    uint8_t interval;

    player_update();
    invader_update(frames, player.level, player.sprite->y);
    boss_update(frames);

    if (!boss.active && boss.health > 0 && invaders.count < 20)
        boss_activate();

    interval = enemy_fire_interval();
    if (frames % interval == interval - 1)
        fire_enemy();

    events = bullet_update();
    if (events.invader_hits) {
        player_add_score(events.invader_hits);
        sound_play(INVADER_SOUND, 400, 7);
    }
    if (events.boss_hits) {
        if (events.boss_kills) {
            player_add_score(10);
            sound_play(PLAYER_SOUND, 220, 6);
        } else {
            sound_play(PLAYER_SOUND, 440, 6);
        }
    }
    for (uint8_t i = 0; i < events.player_hits; i++)
        player_damage();
    if (events.player_hits)
        sound_play(SYSTEM_SOUND, 360, 7);

    if (events.invader_hits || events.boss_hits || events.player_hits)
        player_update_hud();
}

static void draw(void)
{
    gfx_error err;
    uint16_t scroll_y = 0;

    if (invaders.animation_frame)
        scroll_y = SCREEN_HEIGHT - 1;

    tilemap_scroll(0, invaders.scroll_x, scroll_y);
    err = sprites_render(&vctx);
    handle_error(err, "Failed to render sprite arena", false);
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
    sound_set(SYSTEM_SOUND, WAV_SAWTOOTH);
    reset(true);

    while (true) {
        sound_loop();

        uint8_t action = game_input();
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

        if (player.lives < 1 || invaders.game_over) {
            msleep(500);
            sound_stop_all();
            load_splash("  game  over  ", NULL);
            msleep(250);
            reset(true);
        } else if (invaders.count == 0 && boss.health == 0) {
            msleep(1000);
            sound_stop_all();
            player_next_level();
            reset(false);
        }
    }

quit_game:
    deinit();
    printf("Game complete\n");
    printf("Score: %d\n\n", player.score);
    return 0;
}
