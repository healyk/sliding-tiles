#include <stdio.h>
#include <GL/glfw.h>

#include "gfx.h"
#include "geo.h"
#include "util.h"

texture_t* test_img;

bool
init_game(void) {
  bool result;

  log_init("game.log");
  result = gfx_init("Sliding Tile Game", 800, 600);

  if(result) {
    gfx_begin_2d();
    // Begin test code
    test_img = texture_load("../images/landscape.jpg", true);

    result = test_img != NULL;
    // End test code
  }

  return result;
}

void
main_loop(void) {
  bool running = true;
  rect_t dest = { 0, 0, 800, 600 };

  rect_t draw = { 128, 400, 128, 128 };
  color_t c = { 255, 0, 0, 255 };

  while(running) {
    glClear(GL_COLOR_BUFFER_BIT);
    gfx_blit(test_img, NULL, &dest);
    gfx_draw_rect(&draw, &c, true);
    glfwSwapBuffers();

    running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
  }
}

void
shutdown_game(void) {
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
