#include <math.h>

#include "game.h"
#include "util.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

game_t*
game_new(skill_level_t skill, texture_t* texture) {
  game_t* game = new(game_t);
  int     sprite_w;
  int     sprite_h;
  int     iskill = (int)skill;

  sprite_w = texture->width / iskill;
  sprite_h = texture->height / iskill;

  // Set up the game board
  game->skill = skill;
  game->board_sheet = sprite_sheet_new(texture, sprite_w, sprite_h);
  game->board = new_array(sprite_t*, iskill * iskill);
  game->scale_width =  SCREEN_WIDTH / (texture->width * 1.0f);
  game->scale_height =  SCREEN_HEIGHT / (texture->height * 1.0f);

  for(int x = 0; x < iskill; x++) {
    for(int y = 0; y < iskill; y++) {
      // Speical case (0, 0), it's the empty area
      if(0 == x && 0 == y) {
        game->board[0] = NULL;
      } else {
        game->board[x + (y * iskill)] = 
          sprite_sheet_get_sprite(game->board_sheet, x, y);
      }
    }
  }

  return game;
}

void
game_end(game_t* game) {
  delete(game->board);
  sprite_sheet_delete(game->board_sheet);
  delete(game);
}

void
game_render_board(game_t* game) {
  int iskill = (int)game->skill;
  
  for(int x = 0; x < iskill; x++) {
    for(int y = 0; y < iskill; y++) {
      sprite_t* sprite = game->board[x + (y * iskill)];

      if(sprite != NULL) {
        // Calculate the destination.  We have to scale the individual sprites
        // to the screen's resolution.
        int width = (int)ceil(game->scale_width * sprite->area.width);
        int height = (int)ceil(game->scale_height * sprite->area.height);

        rect_t dest = { x * width, y * height, width, height };
        sprite_render(sprite, &dest);
      }
    }
  }
}
