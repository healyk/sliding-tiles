/**
   @file game.h
   
   Contains the main data structures and function prototypes for the actual
   game.
*/
#ifndef GAME_H
#define GAME_H

#include "gfx.h"

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

/**
   Skill levels.  Each number represents the number of vertical and horizontal
   tiles the board will be cut into.  So for easy, the board will be 3x3.
*/
typedef enum skill_level {
  SKILL_EASY = 4,
  SKILL_MEDIUM = 8,
  SKILL_HARD = 16
} skill_level_t;

/**
   Describes the current play state of the game.  This is used to determine 
   if the game is accepting input, animating, and so forth.
*/
typedef enum play_state {
  /** Indicates that we are currently waiting for player input. */
  PLAY_STATE_WAIT_FOR_INPUT,

  /** 
      Indicates the game is moving a tile currently and no input should
      be accepted. 
  */
  PLAY_STATE_MOVING_TILE,

  /**
     The game has been finished.
  */
  PLAY_STATE_GAME_FINISHED
} play_state_t;

/**
   Each square in the picture will be cut up into a tile.  This tile has 
   knowledge of it's picture, where it is to be rendered, and where it 
   should be for a win condition.
*/
typedef struct game_tile {
  point_t position;
  point_t win_position;

  point_t velocity;
  point_t pixel_offset;

  sprite_t* sprite;
} game_tile_t;

/**
   Models a running game.
*/
typedef struct game {
  skill_level_t   skill;
  play_state_t    play_state;

  /** Holds the sprite sheet used when drawing the board pieces. */
  sprite_sheet_t* board_sheet;

  /** Holds the positions of the sprites on the board. */
  game_tile_t*    board;

  /** Used when rendering the sprites to the screen to scale them properly. */
  float           scale_width;
  /** Used when rendering the sprites to the screen to scale them properly. */
  float           scale_height;

  double          last_update_time;
} game_t;

/**
   Starts a new game.
*/
game_t*
game_new(skill_level_t skill, texture_t* texture);

/**
   Ends a currently running game.
*/
void
game_end(game_t* game);

/**
   Renders the game board to the screen.
*/
void
game_render_board(game_t* game);

/**
   Called when a mouse click occurs.

   @param game
     The currently running game.
   @param x
     The pixel coordinate where the mouse was clicked.
   @param y
     The pixel coordinate where the mouse was clicked.
*/
void
game_on_click(game_t* game, int x, int y);

/**
   Needs to be called to update the current game state/animation.
*/
void
game_update(game_t* game);

#endif
