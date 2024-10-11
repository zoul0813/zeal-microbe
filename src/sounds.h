#include <stdint.h>
#include <zos_errors.h>
#include <zvb_sound.h>

#pragma once

#define MAX_VOICES  4

typedef struct {
  sound_voice_t voice;
  sound_waveform_t waveform;
  uint16_t freq;
  uint16_t duration;
  uint16_t remaining;
} Sound;


zos_err_t sound_init(void);
Sound* sound_play(uint8_t voice, uint16_t freq, uint16_t duration);
void sound_stop(Sound *sound)
void sound_loop(void);