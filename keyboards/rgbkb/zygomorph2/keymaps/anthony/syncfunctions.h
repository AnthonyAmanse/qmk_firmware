#include <quantum.h>

typedef struct user_runtime_config {
    bool caps_pressed;
    uint16_t  layer;
} user_runtime_config;

extern user_runtime_config user_state;
// _Static_assert(sizeof(user_runtime_config) == 4, "Invalid data transfer size for keyboard sync data %u");


void set_alphakeys_color(void);
void set_game_layer_colors(void);
void set_fn_layer_colors(void);
