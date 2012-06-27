#include <stdio.h>
#include <GL/glfw.h>

#include "gfx.h"
#include "geo.h"
#include "util.h"
#include "game.h"

texture_t* test_img;
game_t*    game;

bool
init_game(void) {
  bool result;

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
  }

  return result;
}

void
main_loop(void) {
  bool running = true;

  while(running) {
    glClear(GL_COLOR_BUFFER_BIT);
    game_render_board(game);
    glfwSwapBuffers();

    if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      int x;
      int y;

      glfwGetMousePos(&x, &y);

      game_on_click(game, x, y);
    }

    running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
  }
}

void
shutdown_game(void) {
  game_end(game);
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
