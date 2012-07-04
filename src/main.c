#include <stdio.h>
#include <string.h>

#include <GL/glfw.h>

#include "gfx.h"
#include "geo.h"
#include "util.h"
#include "game.h"

app_data_t   app_data;
game_t*      game;

/**
   Translates a command-line argument flag to a skill level.
*/
skill_level_t
skill_flag_to_level(int flag) {
  skill_level_t skill;
  
  switch(flag) {
  case 'e': skill = SKILL_EASY; break;
  case 'm': skill = SKILL_MEDIUM; break;
  case 'h': skill = SKILL_HARD; break;
  default:
    printf("Unknown skill %c.  Defaulting to easy.\n", flag);
    skill = SKILL_EASY;
    break;
  }

  return skill;
}

bool
init_game(char* image_filename, skill_level_t skill) {
  bool result;
  texture_t* digits_texture;
  texture_t* game_image;
 
  log_init("game.log");
  result = gfx_init("Sliding Tile Game", SCREEN_WIDTH, SCREEN_HEIGHT);

  if(result) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gfx_begin_2d();

    game_image = texture_load(image_filename, true);
    if(game_image == NULL) {
      printf("Cannot load image %s\n", image_filename);
      result = false;
    } else {
      game = game_new(skill, game_image);

      digits_texture = texture_load("data/digits.png", true);
      app_data.digits = sprite_sheet_new(digits_texture, 16, 24);
      app_data.hud_words = texture_load("data/hud-words.png", true);
    }
  }

  return result;
}

void
main_loop(void) {
  bool running = true;
  double current_time = 0.0;

  while(running) {
    glClear(GL_COLOR_BUFFER_BIT);
    game_render(&app_data, game);
    glfwSwapBuffers();

    if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      int x, y;

      glfwGetMousePos(&x, &y);
      game_on_click(game, x, y);
    }

    current_time = glfwGetTime();

    if(current_time - game->last_update_time >= (32.0 / 1000.0)) {
      game_update(game, current_time - game->last_update_time);
      game->last_update_time = current_time;
    }

    running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
  }
}

void
shutdown_game(void) {
  if(game != NULL) {
    game_end(game);
    sprite_sheet_delete(app_data.digits);
  }

  gfx_end_2d();
  gfx_shutdown();
}

void print_usage() {
  char* usage =
    "Sliding Tile Game usage.\n"
    "slidingtile [options]\n"
    "\n"
    "Options:\n"
    "\t--(s)kill [e|m|h]     Chooses a difficulty.  Easy, Medium and Hard.\n"
    "\t--(i)mage [filename]  Selects the image to use.\n";

  printf(usage);
}

int main(int argc, char** argv) {
  char* img_name = "default.jpg";
  int   skill_flag = 'e';
  bool  should_run = true;

  // Process command line args
  for(int i = 1; i < argc; i++) {
    if((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--skill") == 0)
       && argc >= (i + 1)) 
    {
      skill_flag = argv[i + 1][0];
      i++;
    }

    else if((strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image"))
            && argc >= (i + 1)) 
    {
      img_name = argv[i + 1];
    }

    else {
      print_usage();
      should_run = false;
    }
  }

  if(should_run && init_game(img_name, skill_flag_to_level(skill_flag))) {
    main_loop();
  }

  shutdown_game();

  return 0;
}
