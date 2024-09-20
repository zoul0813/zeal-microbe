#pragma once

#include <stdint.h>

static uint16_t keys = 0;

zos_err_t keyboard_init(void);
zos_err_t keyboard_flush(void);

uint16_t keyboard_read(void);
static inline uint8_t keyboard_pressed(uint16_t key) {
    return keys & key;
}