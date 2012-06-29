#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

const int TIME_WORD_WIDTH = 64;
const int COUNT_WORD_WIDTH = 80;
const int COUNT_OFFSET = 256;

/** 
    This is how fast (in pixels per game loop iteration) the tiles move
    when they slide. 
*/
const int SLIDE_VELOCITY = 20;

/**
   Number of times that the board tiles will be moved around before it
   is presented to the player in a new game.
*/
const int BOARD_RANDOMIZATION_ITERATIONS = 256;

//==============================================================================
// Prototypes
//==============================================================================
static void swap_tiles(game_t* game, int empty_x, int empty_y, int current_x, 
                       int current_y);
static game_tile_t* get_game_tile(game_t* game, int x, int y);

static void
generate_board(game_t* game) {
  int iskill = (int)game->skill;

  for(int x = 0; x < iskill; x++) {
    for(int y = 0; y < iskill; y++) {
      game_tile_t* tile = get_game_tile(game, x, y);

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

/**
   Pre-game randomization of the tiles on the board.  The board is always
   valid because this function simulates board swaps instead of just
   randomly exchanging tiles.
*/
static void
randomize_board_tiles(game_t* game) {
  point_t empty = { 0, 0 };

  for(int i = 0; i < BOARD_RANDOMIZATION_ITERATIONS; i++) {
    // Pick a random adjacent tile to swap with the current empty one
    point_t adjacents[] = {
      { max(empty.x - 1, 0),               empty.y },
      { min(empty.x + 1, game->skill - 1), empty.y },
      { empty.x, max(empty.y - 1, 0) },
      { empty.x, min(empty.y + 1, game->skill - 1) }
    };

    int index = rand_int(0, 4);
    
    // Swap it
    point_t swap_point = adjacents[index];
    swap_tiles(game, empty.x, empty.y, swap_point.x, swap_point.y);
    empty = swap_point;
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
  randomize_board_tiles(game);

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

//==============================================================================
// Game Rendering
//==============================================================================

static void
draw_game_board(game_t* game) {
  // Calculate the destination.  We have to scale the individual sprites
  // to the screen's resolution.
  int width = (int)ceil(game->scale_width * game->board_sheet->sprite_width);
  int height = (int)ceil(game->scale_height * game->board_sheet->sprite_height);
  
  for(int x = 0; x < game->skill; x++) {
    for(int y = 0; y < game->skill; y++) {
      game_tile_t* tile = get_game_tile(game, x, y);
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

/**
   Draws a given number of digits.  This will draw the number and pad any
   empty spaces with zeros.
   @param start_x
     Where to start drawing on the hud, x coordinate.
   @param num
     Number to draw.
   @param count
     Number of total digits to draw (filled in with zeros if num isn't this
     many digits).
   @param color
     Color to draw it in.
*/
static void
draw_digits(app_data_t* app, game_t* game, int start_x, int num, int count,
            color_t* color) 
{
  for(int i = count - 1; i >= 0; i--) {
    sprite_t* sprite;
    rect_t dest = { start_x + i * app->digits->sprite_width, 4, 
                    0, 0 };

    sprite = sprite_sheet_get_sprite(app->digits, num % 10, 0);
    sprite_render(sprite, &dest, color);

    num /= 10;
  }
}

static void
draw_hud_words(app_data_t* app) {
  rect_t dest = { 0, 4, 0, 0 };
  rect_t src = { 0, 0, TIME_WORD_WIDTH, 24 };
  color_t green = { 0, 255, 0, 255 };

  // Draw time
  gfx_blit(app->hud_words, &src, &dest, &green);

  // Draw 'count'
  dest.x = COUNT_OFFSET;
  src.x = TIME_WORD_WIDTH;
  src.width = COUNT_WORD_WIDTH;

  gfx_blit(app->hud_words, &src, &dest, &green);
}

static void
draw_current_time(app_data_t* app, game_t* game) {
  color_t   green = { 0, 255, 0, 255 };
  color_t   gray  = { 64, 64, 64, 255 };
  int       time;
  int       seconds;
  int       minutes;
  sprite_t* colon;
  rect_t    dest = { 48 + TIME_WORD_WIDTH + 4, 4, 0, 0 };

  time = (int)(game->play_time - game->time_game_begin);

  seconds = time % 60;
  minutes = time / 60;

  draw_digits(app, game, 16 + TIME_WORD_WIDTH, 88, 2, &gray);
  draw_digits(app, game, 16 + TIME_WORD_WIDTH, minutes, 2, &green);
  
  // Draw colon
  colon = sprite_sheet_get_sprite(app->digits, 10, 0);
  sprite_render(colon, &dest, &green);

  draw_digits(app, game, 64 + TIME_WORD_WIDTH, 88, 2, &gray);
  draw_digits(app, game, 64 + TIME_WORD_WIDTH, seconds, 2, &green);
}

static void
draw_current_count(app_data_t* app, game_t* game) {
  color_t   green = { 0, 255, 0, 255 };
  color_t   gray  = { 64, 64, 64, 255 };
  int       offset = 16 + COUNT_WORD_WIDTH + COUNT_OFFSET;

  draw_digits(app, game, offset, 88888, 5, &gray);
  draw_digits(app, game, offset, game->move_count, 5, &green);
}

static void
draw_game_hud(app_data_t* app, game_t* game) {
  draw_hud_words(app);

  draw_current_time(app, game);
  draw_current_count(app, game);
}

void
game_render(app_data_t* app, game_t* game) {
  draw_game_board(game);
  draw_game_hud(app, game);
}

//==============================================================================
// Game Movement/Win
//==============================================================================

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
  
  empty_tile = get_game_tile(game, empty_x, empty_y);
  if(empty_tile->sprite == NULL) {
    // Swap the tiles.
    point_t swap_point;
    current_tile = get_game_tile(game, current_x, current_y);
    
    empty_tile->sprite = current_tile->sprite;
    current_tile->sprite = NULL;
    
    swap_point = empty_tile->win_position;
    empty_tile->win_position = current_tile->win_position;
    current_tile->win_position = swap_point;
  }
}

static game_tile_t*
get_game_tile(game_t* game, int x, int y) {
  return game->board + (x + (y * game->skill));
}

static bool
check_for_win(game_t* game) {
  bool is_win = true;

  for(int x = 0; x < game->skill && is_win; x++) {
    for(int y = 0; y < game->skill && is_win; y++) {
      game_tile_t* tile = get_game_tile(game, x, y);

      if(tile->win_position.x != x || tile->win_position.y != y) {
        is_win = false;
      }
    }
  }

  if(is_win) {
    game->play_state = PLAY_STATE_GAME_FINISHED;
  }
  
  return is_win;
}

void
game_on_click(game_t* game, int x, int y) {
  if(game->play_state == PLAY_STATE_WAIT_FOR_INPUT) {
    // Translate the x, y to a tile x, y
    int tile_x = (x / (SCREEN_WIDTH * 1.0f)) * game->skill;
    int tile_y = (y / (SCREEN_HEIGHT * 1.0f)) * game->skill;

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
      if(test.x >= 0 && test.x < game->skill && 
         test.y >= 0 && test.y < game->skill) 
      {
        if(is_tile_empty(game, test.x, test.y)) {
          game_tile_t* current_tile = get_game_tile(game, tile_x, tile_y);
          
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
  bool tile_adjusted = false;
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
    tile_adjusted = true;
  } else if(abs(tile->pixel_offset.y) >= 
            (tile->sprite->area.height * game->scale_height)) 
  {
    int ymod = tile->velocity.y / abs(tile->velocity.y);
    
    swap_tiles(game, 
               tile->position.x,
               tile->position.y + ymod,
               tile->position.x,
               tile->position.y);
    tile_adjusted = true;
  }

  if(tile_adjusted) {
    reset_moving_tile_to_stationary(tile);
    game->move_count++;

    if(!check_for_win(game)) {
      game->play_state = PLAY_STATE_WAIT_FOR_INPUT;
    }
  }
}

void
game_update(game_t* game, double delta) {
  if(game->play_state != PLAY_STATE_GAME_FINISHED) {
    game->play_time += delta;
    
    for(int x = 0; x < game->skill; x++) {
      for(int y = 0; y < game->skill; y++) {
        game_tile_t* tile = get_game_tile(game, x, y);
        
        if(tile->velocity.x != 0 || tile->velocity.y != 0) {
          move_tile_calculation(game, tile);
        }
      }
    }
  }
}
