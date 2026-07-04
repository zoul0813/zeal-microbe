#include <stdint.h>
#include <zvb_gfx.h>

extern uint8_t _tiles_ztp_start;
extern uint8_t _tiles_ztp_end;
extern uint8_t _tiles_zts_start;
extern uint8_t _tiles_zts_end;
extern uint8_t _numbers_zts_start;
extern uint8_t _numbers_zts_end;
extern uint8_t _letters_zts_start;
extern uint8_t _letters_zts_end;
extern uint8_t _microbe_ztm_start;
extern uint8_t _microbe_ztm_end;
extern uint8_t _splash_ztm_start;
extern uint8_t _splash_ztm_end;

gfx_error load_palette(gfx_context* ctx);
gfx_error load_tiles(gfx_context* ctx, gfx_tileset_options* options);
gfx_error load_numbers(gfx_context* ctx, gfx_tileset_options* options);
gfx_error load_letters(gfx_context* ctx, gfx_tileset_options* options);

uint8_t* get_tilemap_start(void);
uint8_t* get_tilemap_end(void);

uint8_t* get_splash_start(void);
uint8_t* get_splash_end(void);
