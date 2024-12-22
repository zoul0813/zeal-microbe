#include <stdint.h>
#include <zos_errors.h>
#include <zvb_sound.h>

#pragma once

#define MAX_VOICES 4
#define VOICEALL   VOICE0 | VOICE1 | VOICE2 | VOICE3

typedef struct {
        sound_voice_t voice;
        sound_waveform_t waveform;
        uint16_t freq;
        uint16_t duration;
        uint16_t remaining;
} Sound;


zos_err_t sound_init(void);
zos_err_t sound_deinit(void);
Sound* sound_play(uint8_t voice, uint16_t freq, uint16_t duration);
void sound_stop(Sound* sound);
void sound_stop_all(void);
void sound_loop(void);
Sound* sound_get(uint8_t voice);