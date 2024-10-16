#include <stdio.h>
#include <stdint.h>
#include <zvb_sound.h>
#include <zos_errors.h>
#include <zos_time.h>
#include "sounds.h"

static Sound sounds[MAX_VOICES];

zos_err_t sound_init(void) {
  sounds[0].voice = VOICE0;
  sounds[1].voice = VOICE1;
  sounds[2].voice = VOICE2;
  sounds[3].voice = VOICE3;
  for(uint8_t i = 0; i < MAX_VOICES; i++) {
    sounds[i].waveform = WAV_SQUARE;
    sounds[i].freq = 440;
    sounds[i].duration = 0;
    sounds[i].remaining = 0;
  }

  zvb_sound_initialize(1);

  zvb_sound_set_voices(VOICEALL, 0, WAV_SQUARE);
  zvb_sound_set_hold(VOICEALL, 0);

  zvb_sound_set_volume(VOL_75);

  return ERR_SUCCESS;
}

zos_err_t sound_deinit(void) {
  zvb_sound_set_voices(VOICEALL, 0, WAV_SQUARE);
  zvb_sound_set_hold(VOICEALL, 1);
  zvb_sound_set_volume(VOL_0);
  return ERR_SUCCESS;
}

Sound* sound_get(uint8_t voice) {
  if(voice >= MAX_VOICES) return NULL;
  return &sounds[voice];
}


Sound* sound_play(uint8_t voice, uint16_t freq, uint16_t duration) {
  if(voice >= MAX_VOICES) return NULL;
  Sound *sound = &sounds[voice];
  sound->freq = freq;
  sound->duration = duration;
  sound->remaining = duration;

  uint16_t note = SOUND_FREQ_TO_DIV(sound->freq);
  zvb_sound_set_voices(sound->voice, note, sound->waveform);
  return sound;
}

void sound_stop(Sound *sound) {
  sound->remaining = 0;
  zvb_sound_set_voices(sound->voice, 0, WAV_SQUARE);
}

void sound_stop_all(void) {
  zvb_sound_set_voices(VOICEALL, 0, WAV_SQUARE);
}

void sound_loop(void) {
  for(uint8_t i = 0; i < MAX_VOICES; i++) {
    Sound *sound = &sounds[i];
    if(sound->remaining > 0) {
      sound->remaining--;
      if(sound->remaining < 1) {
        sound_stop(sound);
      }
    }
  }
}