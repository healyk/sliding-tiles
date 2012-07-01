#ifndef MENU_H
#define MENU_H

#include "game.h"

typedef enum menu_state {
  /** At the main menu */
  MENU_STATE_MAIN,

  /** At the skill selection screen. */
  MENU_STATE_SKILL_SELECT,
} menu_state_t;

/**
   Holds the internal state of the current menu.
*/
typedef struct menu_data {
  menu_state_t state;
} menu_data_t;

/**
   Should be called when initializing the menu at the start of the game.
*/
void menu_init(app_data_t* app_data, menu_data_t* menu_data);

/**
   Cleans up menu resources.
*/
void menu_cleanup(menu_data_t* menu_data);

/**
   Renders the menu to the current screen.
*/
void menu_render(app_data_t* app, menu_data_t* menu_data, game_t* game);

void menu_update(app_data_t* app, menu_data_t* menu_data, double delta);

void menu_handle_input(app_data_t* app, menu_data_t* menu_data);

#endif
