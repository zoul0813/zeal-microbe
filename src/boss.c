#include <zgdk.h>

#include "boss.h"

Boss boss;

static gfx_sprite* register_sprite(uint8_t tile, uint16_t x, uint16_t y)
{
    gfx_sprite sprite;
    sprite.tile    = tile;
    sprite.x       = x;
    sprite.y       = y;
    sprite.flags   = SPRITE_NONE;
    sprite.options = SPRITE_OPTION_NONE;
    return sprites_register(sprite);
}

static void hide(void)
{
    boss.tl->y = SCREEN_HEIGHT + SPRITE_HEIGHT;
    boss.tr->y = boss.tl->y;
    boss.bl->y = boss.tl->y + SPRITE_HEIGHT;
    boss.br->y = boss.bl->y;
}

zos_err_t boss_init(void)
{
    uint16_t x = ((WIDTH / 2) * SPRITE_WIDTH) - SPRITE_HALF;

    boss.tl = register_sprite(BOSS_INVADER_TL1, x, 0);
    boss.tr = register_sprite(BOSS_INVADER_TR1, x + SPRITE_WIDTH, 0);
    boss.bl = register_sprite(BOSS_INVADER_BL1, x, 0);
    boss.br = register_sprite(BOSS_INVADER_BR1, x + SPRITE_WIDTH, 0);

    if (boss.tl && boss.tr && boss.bl && boss.br)
        return ERR_SUCCESS;
    return 1;
}

void boss_reset(void)
{
    uint16_t x = ((WIDTH / 2) * SPRITE_WIDTH) - SPRITE_HALF;

    boss.active    = false;
    boss.health    = 3;
    boss.frame     = 0;
    boss.direction = 1;
    boss.tl->x     = x;
    boss.tr->x     = x + SPRITE_WIDTH;
    boss.bl->x     = x;
    boss.br->x     = x + SPRITE_WIDTH;

    boss.tl->tile = BOSS_INVADER_TL1;
    boss.tr->tile = BOSS_INVADER_TR1;
    boss.bl->tile = BOSS_INVADER_BL1;
    boss.br->tile = BOSS_INVADER_BR1;
    hide();
}

void boss_activate(void)
{
    if (boss.active || boss.health == 0)
        return;

    boss.active = true;
    boss.tl->y  = SPRITE_HEIGHT;
    boss.tr->y  = boss.tl->y;
    boss.bl->y  = boss.tl->y + SPRITE_HEIGHT;
    boss.br->y  = boss.bl->y;
}

void boss_update(uint16_t frame)
{
    if ((frame & 0x1F) == 0x1F) {
        boss.frame ^= 1;
        if (boss.frame == 0) {
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
    }

    if (!boss.active || (frame & 1) == 0)
        return;

    boss.tl->x += boss.direction;
    boss.tr->x += boss.direction;
    boss.bl->x += boss.direction;
    boss.br->x += boss.direction;

    if (boss.tl->x < 32)
        boss.direction = 1;
    if (boss.tl->x > SCREEN_WIDTH - 48)
        boss.direction = -1;
}

uint8_t boss_hit(uint16_t x, uint16_t y, uint8_t width, uint8_t height)
{
    uint16_t right;
    uint16_t bottom;

    if (!boss.active || boss.health == 0)
        return 0;

    right  = boss.tl->x + (SPRITE_WIDTH * 2);
    bottom = boss.tl->y + (SPRITE_HEIGHT * 2);
    if (x >= right || x + width <= boss.tl->x || y >= bottom || y + height <= boss.tl->y)
        return 0;

    boss.health--;
    if (boss.health == 0) {
        boss.active = false;
        hide();
        return 2;
    }
    return 1;
}

ShotOrigin boss_shot_origin(void)
{
    ShotOrigin origin = {0, 0, false};
    if (boss.active && boss.health > 0) {
        origin.x     = boss.tl->x + SPRITE_HALF;
        origin.y     = boss.bl->y + SPRITE_HEIGHT;
        origin.valid = true;
    }
    return origin;
}
