#include <stdio.h>
#include <GL/glfw.h>

#include "gfx.h"
#include "geo.h"
#include "util.h"
#include "game.h"
#include "menu.h"

app_data_t   app_data;
game_t*      game;
menu_data_t* menu_data;

bool
init_game(void) {
  bool result;
  texture_t* digits_texture;
  texture_t* test_img;

  log_init("game.log");
  result = gfx_init("Sliding Tile Game", 800, 600);

  if(result) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gfx_begin_2d();
    // Begin test code
    test_img = texture_load("images/landscape.jpg", true);

    result = test_img != NULL;
    game = game_new(SKILL_EASY, test_img);
    // End test code

    digits_texture = texture_load("data/digits.png", true);
    app_data.digits = sprite_sheet_new(digits_texture, 16, 24);
    app_data.hud_words = texture_load("data/hud-words.png", true);
    app_data.menu_font = font_new(texture_load("data/menu-font.png", true),
                                  24, 24);
  }

  return result;
}

void
main_loop(void) {
  bool running = true;
  double current_time = 0.0;
  color_t test_color   = { 255, 0, 0, 255 };

  while(running) {
    glClear(GL_COLOR_BUFFER_BIT);
    game_render(&app_data, game);
    font_render_char(app_data.menu_font, 32, 32, 'a', &test_color);
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
