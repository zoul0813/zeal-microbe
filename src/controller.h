/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once

#include <stdint.h>

#define SNES_B          0x0001  // 1
#define SNES_Y          0x0002  // 2
#define SNES_SELECT     0x0004  // 3
#define SNES_START      0x0008  // 4
#define SNES_UP         0x0010  // 5
#define SNES_DOWN       0x0020  // 6
#define SNES_LEFT       0x0040  // 7
#define SNES_RIGHT      0x0080  // 8
#define SNES_A          0x0100  // 9
#define SNES_X          0x0200  // 10
#define SNES_L          0x0400  // 11
#define SNES_R          0x0800  // 12
#define SNES_UNUSED1    0x1000  // 13
#define SNES_UNUSED2    0x2000  // 14
#define SNES_UNUSED3    0x4000  // 15
#define SNES_UNUSED4    0x8000  // 16

static uint16_t buttons = 0; // nothing

zos_err_t controller_init(void);
zos_err_t controller_flush(void);

uint16_t controller_read(void);
static inline uint8_t controller_pressed(uint16_t button) {
    return buttons & button;
}