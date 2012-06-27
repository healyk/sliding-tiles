#include <stdio.h>
#include <GL/glfw.h>

#include "gfx.h"
#include "geo.h"
#include "util.h"
#include "game.h"

app_data_t app_data;
texture_t* test_img;
game_t*    game;

bool
init_game(void) {
  bool result;
  texture_t* digits_texture;

  log_init("game.log");
  result = gfx_init("Sliding Tile Game", 800, 600);

  if(result) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    gfx_begin_2d();
    // Begin test code
    test_img = texture_load("images/landscape.jpg", true);

    result = test_img != NULL;
    game = game_new(SKILL_EASY, test_img);
    // End test code

    digits_texture = texture_load("images/digits.png", true);
    app_data.digits = sprite_sheet_new(digits_texture, 16, 24);
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
      game->last_update_time = current_time;
      game_update(game);
    }

    running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
  }
}

void
shutdown_game(void) {
  game_end(game);
  sprite_sheet_delete(app_data.digits);
  gfx_end_2d();
  gfx_shutdown();
}

int main() {
  if(init_game()) {
    main_loop();
    shutdown_game();
  }

  return 0;
}
