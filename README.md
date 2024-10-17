[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# Microbe

Microbe is a Space Invaders inspired game.

It runs in 320x240@8bpp and uses RLE Compressed tiles

## Controls

Has support for both Keyboard and SNES controller.  The game attempts to automatically detect if you have a SNES controller plugged in, and will enable support for it if found.  You will see a small SNES icon on the startup screen if the SNES controller is detected.

* SNES D-Pad, WASD, Arrows for movement (left/right)
* SNES B or Space to shoot
* SNES Start or Enter to Pause/Resume
* SNES Select or Single Quote (`'`) to Quit

## Screenshots

| Startup Screen  | Gameplay |
| - | - |
| ![image](https://github.com/user-attachments/assets/336c9370-8b3e-45fe-adcf-34c75a33ed26) | ![image](https://github.com/user-attachments/assets/8534cbc3-66d0-4b53-950d-de060542d207) |



## TODO

- [ ] multiple levels
- [ ] menu
- [ ] high score table
- [x] boss invader on top
- [x] moving invaders (tilemap scroll)
- [x] player lives/health
- [x] pause screen