#include <stdio.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_keyboard.h>
#include "keyboard.h"
#include "controller.h"

static uint8_t key_buffer[32];

zos_err_t keyboard_flush(void) {
  /* Flush the keyboard fifo */
  keys = 0;
  uint16_t size = sizeof(key_buffer);
  while (size) {
    zos_err_t err = read(DEV_STDIN, key_buffer, &size);
    if(err != ERR_SUCCESS) {
      return err;
    }
  }
  return ERR_SUCCESS;
}

zos_err_t keyboard_init(void) {
  /* Initialize the keyboard by setting it to raw and non-blocking */
  void* arg = (void*) (KB_READ_NON_BLOCK | KB_MODE_RAW);
  zos_err_t err = ioctl(DEV_STDIN, KB_CMD_SET_MODE, arg);
  if(err != ERR_SUCCESS) {
    return err;
  }

  return keyboard_flush();
}

uint16_t keyboard_read(void) {
  uint16_t size = sizeof(key_buffer);
  while(1) {
    zos_err_t err = read(DEV_STDIN, key_buffer, &size);
    if(err != ERR_SUCCESS) {
      printf("Failed to read DEV_STDIN, clearing keys: %d", err);
      keys = 0;
      return keys;
    }

    if (size == 0) {
      return keys;
    }

    uint8_t released = 0;
    for (uint8_t i = 0; i < size; i++) {
      if (key_buffer[i] == KB_RELEASED) {
        released = 1;
      } else {
        switch(key_buffer[i]) {
          case KB_KEY_W:
          case KB_UP_ARROW:
            if(released) {
              keys &= ~SNES_UP;
            } else {
              keys |= SNES_UP;
            }
            break;

          case KB_KEY_S:
          case KB_DOWN_ARROW:
            if(released) {
              keys &= ~SNES_DOWN;
            } else {
              keys |= SNES_DOWN;
            }
            break;

          case KB_KEY_A:
          case KB_LEFT_ARROW:
            if(released) {
              keys &= ~SNES_LEFT;
            } else {
              keys |= SNES_LEFT;
            }
            break;

          case KB_KEY_D:
          case KB_RIGHT_ARROW:
            if(released) {
              keys &= ~SNES_RIGHT;
            } else {
              keys |= SNES_RIGHT;
            }
            break;

          case KB_KEY_ENTER:
            if(released) {
              keys &= ~SNES_START;
            } else {
              keys |= SNES_START;
            }
            break;
          case KB_KEY_SPACE:
            if(released) {
              keys &= ~SNES_B;
            } else {
              keys |= SNES_B;
            }
            break;
        }
        released = 0;
      }
    }
  }
}
