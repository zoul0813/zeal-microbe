#include <stdint.h>
#include <zvb_gfx.h>

void __assets__(void);

extern uint8_t _palette_end;
extern uint8_t _palette_start;
extern uint8_t _tiles_end;
extern uint8_t _tiles_start;
extern uint8_t _numbers_end;
extern uint8_t _numbers_start;
extern uint8_t _letters_end;
extern uint8_t _letters_start;
extern uint8_t _tilemap_start;
extern uint8_t _tilemap_end;
extern uint8_t _splash_start;
extern uint8_t _splash_end;


gfx_error load_palette(gfx_context* ctx);
gfx_error load_tiles(gfx_context* ctx, gfx_tileset_options* options);
gfx_error load_numbers(gfx_context* ctx, gfx_tileset_options* options);
gfx_error load_letters(gfx_context* ctx, gfx_tileset_options* options);

uint8_t* get_tilemap_start(void);
uint8_t* get_tilemap_end(void);

uint8_t* get_splash_start(void);
uint8_t* get_splash_end(void);