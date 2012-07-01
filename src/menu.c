#include "menu.h"

void 
menu_init(app_data_t* app_data, menu_data_t* menu_data) {
  menu_data->state = MENU_STATE_MAIN;
}

void 
menu_cleanup(menu_data_t* menu_data) {
}

void 
menu_render(app_data_t* app, menu_data_t* menu_data, game_t* game) {
}

void 
menu_update(app_data_t* app, menu_data_t* menu_data, double delta) {
}

void 
menu_handle_input(app_data_t* app, menu_data_t* menu_data) {
}
