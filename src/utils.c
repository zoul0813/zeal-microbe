#include "utils.h"
#include <string.h>
#include "microbe.h"

void print_string(const char* str, uint8_t x, uint8_t y)
{
    gfx_tilemap_load(&vctx, str, strlen(str), 1, x, y);
}

void nprint_string(const char* str, uint8_t len, uint8_t x, uint8_t y)
{
    gfx_tilemap_load(&vctx, str, len, 1, x, y);
}

/**
 * Retrieve the value of the R register
 *
 * This is can be used for a quick dirty "random" value
*/
char rand8_quick(void) __naked
{
    __asm__(
    "ld a, r\n"
    "ret\n"
    );
}

/**
 * Return an 8-bit pseudo-random number
 *
 * This is a combination of an LFSR w/ Counter
*/
char rand8(void) __naked
{
    // credit: https://spectrumcomputing.co.uk/forums/viewtopic.php?t=4571
    // __asm
    // ;This code snippet is 9 bytes and 43cc
    // ;Inputs:
    // ;   HL is the input seed and must be non-zero
    // ;Outputs:
    // ;   A is the 8-bit pseudo-random number
    // ;   HL is the new seed value (will be non-zero)

    // ;-------------------------------------------------------------------------------
    // ;Technical details:
    // ;   The concept behind this routine is to combine an LFSR (poor RNG) with a
    // ; counter. The counter improves the RNG quality, while also extending the period
    // ; length.
    // ;   For this routine, I took advantage of the Z80's built-in counter, the `r`
    // ; register. This means that we don't need to store the counter anywhere, and it
    // ; is pretty fast to access!
    // ;   Some caveats:
    // ;     * r is a 7-bit counter
    // ;     * r will increment some number of times between runs of the RNG. In most
    // ;       cases, this will be constant, but if it increments an even number each
    // ;       time, then the bottom bit is always the same, weakening the effect of
    // ;       the counter. In the worst case, it increments a multiple of 128 times,
    // ;       effectively making your RNG just as good/bad as the LFSR. Ideally, you
    // ;       want `r` to increment an odd number of times between runs.
    // ;     * In the best case, the bottom 7 bits have 50/50 chance of being 0 or 1.
    // ;       The top bit is 1 with probability 1/2 + 1/(2^17-2) ~ .5000076295
    // ;     * In the event that your main loop waits for user input between calls,
    // ;       then congatulations, you might have a True RNG :)
    // ;-------------------------------------------------------------------------------

    __asm__(
    "            ;opcode cc\n"
    "add hl,hl   ; 29    11\n"
    "sbc a,a     ; 9F     4\n"
    "and #0x2D   ; E62D   7\n"
    "xor l       ; AD     4\n"
    "ld l,a      ; 6F     4\n"
    "ld a,r      ; ED5F   9\n"
    "add a,h     ; 84     4\n"
    );
}