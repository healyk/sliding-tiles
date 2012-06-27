#include <math.h>

#include "game.h"
#include "util.h"

//==============================================================================
// Constants
//==============================================================================
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int SLIDE_VELOCITY = 20;

static void
generate_board(game_t* game) {
  int iskill = (int)game->skill;

  for(int x = 0; x < iskill; x++) {
    for(int y = 0; y < iskill; y++) {
      game_tile_t* tile = game->board + (x + (y * iskill));

      // Speical case (0, 0), it's the empty area
      if(0 == x && 0 == y) {
        tile->sprite = NULL;
      } else {
        tile->sprite = 
          sprite_sheet_get_sprite(game->board_sheet, x, y);
      }

      tile->win_position.x = x;
      tile->win_position.y = y;

      tile->position.x = x;
      tile->position.y = y;
    }
  }
}

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
  game->play_state = PLAY_STATE_WAIT_FOR_INPUT;

  game->board_sheet = sprite_sheet_new(texture, sprite_w, sprite_h);
  game->board = new_array(game_tile_t, iskill * iskill);

  game->scale_width =  SCREEN_WIDTH / (texture->width * 1.0f);
  game->scale_height =  SCREEN_HEIGHT / (texture->height * 1.0f);

  generate_board(game);

  return game;
}

void
game_end(game_t* game) {
  // Clean up the board
  delete(game->board);

  // Remove the graphics resources
  sprite_sheet_delete(game->board_sheet);

  // Clean up memory
  delete(game);
}

void
game_render_board(game_t* game) {
  int iskill = (int)game->skill;
  
  for(int x = 0; x < iskill; x++) {
    for(int y = 0; y < iskill; y++) {
      game_tile_t* tile = game->board + (x + (y * iskill));
      sprite_t* sprite = tile->sprite;

      if(sprite != NULL) {
        // Stationary sprites
        if(0 == tile->velocity.x && 0 == tile->velocity.y) {
          // Calculate the destination.  We have to scale the individual sprites
          // to the screen's resolution.
          int width = (int)ceil(game->scale_width * sprite->area.width);
          int height = (int)ceil(game->scale_height * sprite->area.height);
          
          rect_t dest = { x * width, y * height, width, height };
          sprite_render(sprite, &dest);
        }

        // Moving sprites
        else {
          // Calculate the destination.  We have to scale the individual sprites
          // to the screen's resolution.
          int width = (int)ceil(game->scale_width * sprite->area.width);
          int height = (int)ceil(game->scale_height * sprite->area.height);
          
          rect_t dest = { x * width + tile->pixel_offset.x, 
                          y * height + tile->pixel_offset.y, 
                          width, height };
          sprite_render(sprite, &dest);
        }
      }
    }
  }
}

static bool
is_tile_empty(game_t* game, int x, int y) {
  game_tile_t* empty_tile;

  empty_tile = game->board + (x + (y * (int)game->skill));
  return empty_tile->sprite == NULL;
}

static void
swap_tiles(game_t* game, 
           int empty_x, 
           int empty_y, 
           int current_x, 
           int current_y) 
{
  game_tile_t* current_tile;
  game_tile_t* empty_tile;
  int         iskill = (int)game->skill;
  
  empty_tile = game->board + (empty_x + (empty_y * iskill));
  if(empty_tile->sprite == NULL) {
    // Swap the tiles.
    point_t swap_point;
    current_tile = game->board + (current_x + (current_y * iskill));
    
    empty_tile->sprite = current_tile->sprite;
    current_tile->sprite = NULL;
    
    swap_point = empty_tile->win_position;
    empty_tile->win_position = current_tile->win_position;
    current_tile->win_position = swap_point;
  }
}

void
game_on_click(game_t* game, int x, int y) {
  int iskill = (int)game->skill;

  if(game->play_state == PLAY_STATE_WAIT_FOR_INPUT) {
    // Translate the x, y to a tile x, y
    int tile_x = (x / (SCREEN_WIDTH * 1.0f)) * iskill;
    int tile_y = (y / (SCREEN_HEIGHT * 1.0f)) * iskill;

    // We must find the adjacent NULL sprite tile.
    point_t adj[4] = {
      { tile_x - 1, tile_y },
      { tile_x,     tile_y - 1 },
      { tile_x + 1, tile_y },
      { tile_x,     tile_y + 1 }
    };

    for(int i = 0; i < 4; i++) {
      point_t test = adj[i];

      // Ignore points that are out of bounds.
      if(test.x >= 0 && test.x < iskill && test.y >= 0 && test.y < iskill) {
        if(is_tile_empty(game, test.x, test.y)) {
          game_tile_t* current_tile = 
            game->board + (tile_x + (iskill * tile_y));
          
          current_tile->velocity.x = (test.x - tile_x) * SLIDE_VELOCITY;
          current_tile->velocity.y = (test.y - tile_y) * SLIDE_VELOCITY;

          game->play_state = PLAY_STATE_MOVING_TILE;
        }
      }
    }
  }
}

static void
reset_tile(game_tile_t* tile) {
  point_t reset = { 0, 0 };

  tile->velocity = reset;
  tile->pixel_offset = reset;
}

void
game_update(game_t* game) {
  for(int x = 0; x < game->skill; x++) {
    for(int y = 0; y < game->skill; y++) {
      game_tile_t* tile = game->board + (x + (y * game->skill));

      if(tile->velocity.x != 0) {
        tile->pixel_offset.x += tile->velocity.x;

        // Check to see if the tile has reached it's destination.
        if(abs(tile->pixel_offset.x) >= 
           (tile->sprite->area.width * game->scale_width))
        {
          int xmod = tile->velocity.x / abs(tile->velocity.x);

          swap_tiles(game, 
                     tile->position.x + xmod,
                     tile->position.y,
                     tile->position.x,
                     tile->position.y);

          reset_tile(tile);
          game->play_state = PLAY_STATE_WAIT_FOR_INPUT;
        }
      }

      if(tile->velocity.y != 0) {
        tile->pixel_offset.y += tile->velocity.y;
        
        if(abs(tile->pixel_offset.y) >= 
           (tile->sprite->area.height * game->scale_height)) 
        {
          int ymod = tile->velocity.y / abs(tile->velocity.y);
          
          swap_tiles(game, 
                     tile->position.x,
                     tile->position.y + ymod,
                     tile->position.x,
                     tile->position.y);

          reset_tile(tile);
          game->play_state = PLAY_STATE_WAIT_FOR_INPUT;
        }
      }
    }
  }
}
