#pragma once

#include <stdint.h>

zos_err_t keyboard_init(void);
zos_err_t keyboard_flush(void);

uint16_t keyboard_read(void);
uint8_t keyboard_pressed(uint16_t key);