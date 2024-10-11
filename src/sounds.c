#include <zvb_sound.h>
#include <zos_errors.h>
#include <zos_time.h>
#include "sounds.h"

Sound sounds[MAX_VOICES];

zos_err_t sound_init(void) {
  sounds[0].voice = VOICE0;
  sounds[1].voice = VOICE1;
  sounds[2].voice = VOICE2;
  sounds[3].voice = VOICE3;
  for(uint8_t i = 0; i < MAX_VOICES; i++) {
    sounds[i].waveform = WAV_SAWTOOTH;
    sounds[i].freq = 440;
    sounds[i].duration = 0;
    sounds[i].remaining = 0;
  }

  zvb_sound_initialize(1);
  zvb_sound_set_hold(VOICE0, 0);
  zvb_sound_set_hold(VOICE1, 0);
  zvb_sound_set_hold(VOICE2, 0);
  zvb_sound_set_hold(VOICE3, 0);
  zvb_sound_set_volume(VOL_100);

  return ERR_SUCCESS;
}

Sound* sound_play(uint8_t voice, uint16_t freq, uint16_t duration) {
  if(voice >= MAX_VOICES) return;
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
  zvb_sound_set_voices(sound->voice, 0, sound->waveform);
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

  return;
}