/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <zos_errors.h>
#include <zos_keyboard.h>
#include "controller.h"

/**
 * @brief Define some variables to access the PIO from C code
 */
__sfr __at(0xd0) IO_PIO_DATA_A;
__sfr __at(0xd2) IO_PIO_CTRL_A;

#define CLOCK_ONCE() do { IO_PIO_DATA_A = 0; IO_PIO_DATA_A = 1 << IO_CLOCK; } while (0)
#define GET_DATA()   (IO_PIO_DATA_A & (1 << IO_DATA))

#define IO_PIO_DISABLE_INT  0x03
#define IO_PIO_BITCTRL      0xcf

#define IO_DATA     0
#define IO_LATCH    2
#define IO_CLOCK    3

zos_err_t controller_flush(void) {
    buttons = 0;
    return ERR_SUCCESS;
}

zos_err_t controller_init(void)
{
    /**
    * Initialize the user port (port A) of the PIO
    * Set it to bit control mode so that each I/O can be controlled independently.
    */
    IO_PIO_CTRL_A = IO_PIO_BITCTRL;
    /**
     * After setting the port as a bit-controlled one, we need to give a bitmask of
     * pins that needs to be output (0) and input (1).
     * Set them all to output except DATA pins.
     */
    IO_PIO_CTRL_A = 3;
    /* Disable the interrupts for this port just in case it was activated */
    IO_PIO_CTRL_A = IO_PIO_DISABLE_INT;
    /**
     * Set the default value of each pin:
     *  - LATCH must be LOW (0)
     *  - CLOCK must be HIGH (1)
     * Set other pins to 0, not very important
     */
    IO_PIO_DATA_A = 1 << IO_CLOCK;

    return ERR_SUCCESS;
}

/**
 * @brief Read the controller state
 */
uint16_t controller_read(void)
{
    /**
     * Generate a pulse on the LATCH pin, CLOCK must remain high during this process
     * Thanks to the preconfigured registers, this takes 24 T-States (2.4 microseconds @ 10MHz)
     */
    IO_PIO_DATA_A = 1 << IO_CLOCK | 1 << IO_LATCH;
    IO_PIO_DATA_A = 1 << IO_CLOCK;
    // Now, the DATA lines contain the first button (B) state.

    buttons = GET_DATA() == 0 ? 0x8000 : 0;
    // process the remaining 1 buttons (last 4 are unused)
    for(uint8_t i = 0; i < 11; ++i) {
        buttons = buttons >> 1;
        CLOCK_ONCE(); // pulse the clock
        buttons |= GET_DATA() == 0 ? 0x8000 : 0; // OR the current button
    }
    // shift over the 4 last unused bits
    buttons >>= 4;

    return buttons;
}