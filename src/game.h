/**
   @file game.h
   
   Contains the main data structures and function prototypes for the actual
   game.
*/
#ifndef GAME_H
#define GAME_H

/**
   Skill levels.  Each number represents the number of vertical and horizontal
   tiles the board will be cut into.  So for easy, the board will be 3x3.
*/
typedef enum skill_level {
  SKILL_EASY = 3,
  SKILL_MEDIUM = 4,
  SKILL_HARD = 5,
  SKILL_VERY_HARD = 6
} skill_level_t;

/**
   Models a running game.
*/
typedef struct game {
  texture_t*    texture;
  skill_level_t skill;

  rect_t*       board;
} game_t;

/**
   Starts a new game.
*/
game_t*
game_new(skill_level_t* skill, texture_t* texture);

/**
   Ends a currently running game.
*/
void
game_end(game_t* game);

#endif
