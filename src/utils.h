/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#pragma once

#include <stdint.h>
#include <zvb_gfx.h>


#ifdef EMULATOR
__sfr __at(0x86) debug_register; // t-state counter
#define TSTATE_LOG(counter) debug_register = counter;
#else
#define TSTATE_LOGT(counter)
#endif

void print_string(gfx_context* ctx, const char* str, uint8_t x, uint8_t y);
void nprint_string(gfx_context* ctx, const char* str, uint8_t len, uint8_t x, uint8_t y);
char rand8_quick(void);
char rand8(void);