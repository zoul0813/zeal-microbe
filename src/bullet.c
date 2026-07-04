#include <zgdk.h>

#include "boss.h"
#include "bullet.h"
#include "invader.h"
#include "player.h"

typedef struct {
    uint8_t active;
    int8_t velocity;
    gfx_sprite* sprite;
} Bullet;

static Bullet bullets[MAX_BULLETS];

static void deactivate(Bullet* bullet)
{
    bullet->active    = false;
    bullet->sprite->y = SCREEN_HEIGHT + SPRITE_HEIGHT;
}

zos_err_t bullet_init(void)
{
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        gfx_sprite sprite;
        sprite.tile    = BULLET_TILE + i;
        sprite.x       = 0;
        sprite.y       = SCREEN_HEIGHT + SPRITE_HEIGHT;
        sprite.flags   = SPRITE_BEHIND_FG;
        sprite.options = SPRITE_OPTION_NONE;

        bullets[i].sprite = sprites_register(sprite);
        if (!bullets[i].sprite)
            return 1;
    }
    return ERR_SUCCESS;
}

void bullet_reset(void)
{
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        if (i == PLAYER_BULLET)
            bullets[i].velocity = -2;
        else
            bullets[i].velocity = 2;
        bullets[i].sprite->tile  = BULLET_TILE + i;
        bullets[i].sprite->flags = SPRITE_BEHIND_FG;
        deactivate(&bullets[i]);
    }
}

uint8_t bullet_fire_player(void)
{
    gfx_sprite* sprite;
    if (bullets[PLAYER_BULLET].active)
        return false;

    sprite = player.sprite;
    bullets[PLAYER_BULLET].active    = true;
    bullets[PLAYER_BULLET].sprite->x = sprite->x;
    bullets[PLAYER_BULLET].sprite->y = sprite->y - 12;
    return true;
}

uint8_t bullet_fire_enemy(ShotOrigin origin)
{
    if (!origin.valid)
        return false;

    for (uint8_t i = 1; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active    = true;
            bullets[i].sprite->x = origin.x;
            bullets[i].sprite->y = origin.y;
            return true;
        }
    }
    return false;
}

BulletEvents bullet_update(void)
{
    BulletEvents events = {0, 0, 0, 0};
    gfx_sprite* target = player.sprite;

    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        Bullet* bullet = &bullets[i];
        uint8_t boss_result;
        if (!bullet->active)
            continue;

        bullet->sprite->y += bullet->velocity;
        if (bullet->sprite->y < SPRITE_HALF || bullet->sprite->y > SCREEN_HEIGHT) {
            deactivate(bullet);
            continue;
        }

        if (i == PLAYER_BULLET) {
            uint16_t hit_x = bullet->sprite->x + SPRITE_HALF;
            uint16_t hit_y = bullet->sprite->y + SPRITE_HALF;

            boss_result = boss_hit(hit_x, hit_y, 1, 1);
            if (boss_result) {
                events.boss_hits++;
                if (boss_result == 2)
                    events.boss_kills++;
                deactivate(bullet);
                continue;
            }

            if (invader_hit(hit_x, hit_y, 1, 1)) {
                events.invader_hits++;
                deactivate(bullet);
            }
        } else {
            if (bullet->sprite->x < target->x + SPRITE_WIDTH &&
                bullet->sprite->x + SPRITE_WIDTH > target->x &&
                bullet->sprite->y < target->y + SPRITE_HEIGHT &&
                bullet->sprite->y + SPRITE_HEIGHT > target->y) {
                events.player_hits++;
                deactivate(bullet);
            }
        }
    }
    return events;
}
