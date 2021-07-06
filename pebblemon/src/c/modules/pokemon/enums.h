#pragma once

#include <pebble.h>

typedef enum {
  D_UP,
  D_LEFT,
  D_DOWN,
  D_RIGHT
} PlayerDirection;

typedef enum {
  P_STAND,
  P_WALK,
  P_RUN,
  P_JUMP,
  P_WARP,
  P_WARP_WALK
} PlayerMode;

typedef enum {
  PG_INTRO,
  PG_PLAY,
  PG_PAUSE_QUEUED,
  PG_PAUSE,
  PG_DIALOGUE,
  PG_BATTLE
} PokemonGameState;

typedef enum {
  PB_FLASH,
  PB_WIPE,
  PB_LOAD,
  PB_SLIDE,
  PB_APPEAR,
  PB_GO_POKEMON,
  PB_PLAYER_TURN_PROMPT,
  PB_PLAYER_TURN,
  PB_PLAYER_MOVE,
  PB_PLAYER_EFFECT,
  PB_ENEMY_MOVE,
  PB_ENEMY_EFFECT,
  PB_PLAYER_WIN,
  PB_EXPERIENCE,
  PB_ENEMY_WIN,
  PB_RUN,
  PB_FADEOUT
} PokemonBattleState;

typedef enum {
  PM_BASE,
  PM_PEBBLE,
  PM_STATS,
  PM_PACK,
  PM_OPTION,
  PM_SAVE_CONFIRM,
  PM_SAVE_OVERWRITE,
  PM_SAVING,
  PM_SAVED
} PokemonMenuState;