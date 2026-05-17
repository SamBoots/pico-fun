#include <stdlib.h>
#include "pico/stdlib.h"
#include <string.h>
#include "graphics/render_types.h"
#include "graphics/renderer.h"
#include "io/io_types.h"
#include "io/button.h"

#define TEST_W 240
#define TEST_H 240
 
int main(void)
{
    stdio_init_all();
    
    button_context_t push_button;
    memset(&push_button, 0, sizeof(push_button));

    button_init_context(&push_button, 2);

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    while (true) {
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());

        button_update(&push_button, now_ms);
        if (button_pressed(&push_button))
        {
            gpio_put(25, 1);
        }
        else if (button_released(&push_button))
        {
            gpio_put(25, 0);
        }
    }
}
