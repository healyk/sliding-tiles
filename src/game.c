#include <math.h>
#include <stdlib.h>

#include "game.h"
#include "util.h"

//==============================================================================
// Constants
//==============================================================================
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int TILE_AREA_WIDTH = 800;
const int TILE_AREA_HEIGHT = (600 - 32);
const int HEIGHT_OFFSET = 32;

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

  game->scale_width =  TILE_AREA_WIDTH / (texture->width * 1.0f);
  game->scale_height =  TILE_AREA_HEIGHT / (texture->height * 1.0f);

  game->last_update_time = game->time_game_begin = glfwGetTime();

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

static void
draw_game_board(game_t* game) {
  int iskill = (int)game->skill;

  // Calculate the destination.  We have to scale the individual sprites
  // to the screen's resolution.
  int width = (int)ceil(game->scale_width * game->board_sheet->sprite_width);
  int height = (int)ceil(game->scale_height * game->board_sheet->sprite_height);
  
  for(int x = 0; x < iskill; x++) {
    for(int y = 0; y < iskill; y++) {
      game_tile_t* tile = game->board + (x + (y * iskill));
      sprite_t* sprite = tile->sprite;

      if(sprite != NULL) {
        // Stationary sprites
        if(tile->velocity.x != 0 && tile->velocity.y != 0) {
          rect_t dest = { x * width, 
                          y * height + HEIGHT_OFFSET, 
                          width, height };
          sprite_render(sprite, &dest, NULL);
        }

      
        // Moving sprite
        else {
          rect_t dest = { x * width + tile->pixel_offset.x, 
                          y * height + tile->pixel_offset.y + HEIGHT_OFFSET, 
                          width, height };
          sprite_render(sprite, &dest, NULL);
        }
      }
    }
  }
}

static void
draw_clock_background(app_data_t* app, game_t* game) {
  color_t grey = { 64, 64, 64, 255 };
  color_t green = { 0, 255, 0, 255 };

  // Draw the template for the time digits
  for(int i = 0; i < 5; i++) {
    sprite_t* sprite;
    rect_t dest = { 4 + i * app->digits->sprite_width, 4, 0, 0 };

    if(i != 2) {
      sprite = sprite_sheet_get_sprite(app->digits, 8, 0);
      sprite_render(sprite, &dest, &grey);
    } else {
      sprite = sprite_sheet_get_sprite(app->digits, 10, 0);
      sprite_render(sprite, &dest, &green);
    }
  }
}

static void
time_to_digits_array(game_t* game, int* arr) {
  int time = (int)(game->last_update_time - game->time_game_begin);

  int seconds = time % 60;
  int minutes = time / 60;

  int result[4];

  result[3] = seconds % 10; seconds /= 10;
  result[2] = seconds % 10; seconds /= 10;
  result[1] = minutes % 10; minutes /= 10;
  result[0] = minutes % 10; minutes /= 10;

  memcpy(arr, result, sizeof(int) * 4);
}

static void
draw_game_hud(app_data_t* app, game_t* game) {
  // Fill the top in with a bloack rectangle.
  rect_t dest = { 0, 0, SCREEN_WIDTH, HEIGHT_OFFSET };
  color_t black = { 0, 0, 0, 255 };
  color_t green = { 0, 255, 0, 255 };
 
  gfx_draw_rect(&dest, &black, true);
  draw_clock_background(app, game);

  // Now render the time
  int time_digits[4];
  time_to_digits_array(game, time_digits);

  for(int i = 4; i >= 0; i--) {
    rect_t dest = { 4 + i * app->digits->sprite_width, 4, 0, 0 };

    // skip the colon
    if(i > 2) {
      sprite_t* sprite;

      sprite = sprite_sheet_get_sprite(app->digits, time_digits[i - 1], 0);
      sprite_render(sprite, &dest, &green);
    } else if(i < 2) {
      sprite_t* sprite;

      sprite = sprite_sheet_get_sprite(app->digits, time_digits[i], 0);
      sprite_render(sprite, &dest, &green);
    }
  }
}

void
game_render(app_data_t* app, game_t* game) {
  draw_game_board(game);
  draw_game_hud(app, game);
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

/**
   Resets the properties of a moving tile.
*/
static void
reset_moving_tile_to_stationary(game_tile_t* tile) {
  point_t reset = { 0, 0 };

  tile->velocity = reset;
  tile->pixel_offset = reset;
}

/**
   Performs the tile's movement calculations.

   @param game
     Current game instance.
   @param tile
     Moving tile.  The tile's velocity should not be zero.
*/
static void
move_tile_calculation(game_t* game, game_tile_t* tile) {
  tile->pixel_offset.x += tile->velocity.x;
  tile->pixel_offset.y += tile->velocity.y;
  
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
    
    reset_moving_tile_to_stationary(tile);
    game->play_state = PLAY_STATE_WAIT_FOR_INPUT;
  } else if(abs(tile->pixel_offset.y) >= 
            (tile->sprite->area.height * game->scale_height)) 
  {
    int ymod = tile->velocity.y / abs(tile->velocity.y);
    
    swap_tiles(game, 
               tile->position.x,
               tile->position.y + ymod,
               tile->position.x,
               tile->position.y);
    
    reset_moving_tile_to_stationary(tile);
    game->play_state = PLAY_STATE_WAIT_FOR_INPUT;
  }
}

void
game_update(game_t* game) {
  for(int x = 0; x < game->skill; x++) {
    for(int y = 0; y < game->skill; y++) {
      game_tile_t* tile = game->board + (x + (y * game->skill));

      if(tile->velocity.x != 0 || tile->velocity.y != 0) {
        move_tile_calculation(game, tile);
      }
    }
  }
}
