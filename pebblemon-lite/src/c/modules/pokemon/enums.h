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
  P_WALK
} PlayerMode;

typedef enum {
  PG_PLAY,
  PG_DIALOGUE,
  PG_BATTLE
} PokemonGameState;

typedef enum {
  PB_LOAD,
  PB_SLIDE,
  PB_APPEAR,
  PB_PLAYER_TURN_PROMPT,
  PB_PLAYER_TURN,
  PB_PLAYER_MOVE,
  PB_PLAYER_EFFECT,
  PB_ENEMY_MOVE,
  PB_ENEMY_EFFECT,
  PB_PLAYER_WIN,
  PB_EXPERIENCE,
  PB_ENEMY_WIN,
  PB_FADEOUT
} PokemonBattleState;