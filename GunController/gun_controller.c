#define TEST_W 64
#define TEST_H 48
 
#include "pico/stdlib.h"
#include "graphics/render_types.h"
#include "graphics/renderer.h"
#include "string.h"
 
int main(void)
{
    stdio_init_all();
    
    render_context_t render_context;
    render_init_context(&render_context, TEST_W, TEST_H, 125);
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    while (true) {
        // make screen black
        render_draw_rect(&render_context, 0, 0, TEST_W, TEST_H, 0xffff);
        render_flush(&render_context);
        gpio_put(25, 0);
        // wait 2 second
        sleep_ms(2000);

        // make screen white
        render_draw_rect(&render_context, 0, 0, TEST_W, TEST_H, 0x0000);
        render_flush(&render_context);
        gpio_put(25, 1);
        // wait 2 second
        sleep_ms(2000);

        // make screen white
        render_8x16numbers(&render_context, '4', 3, 255, 0, 0);
        render_8x16numbers(&render_context, '2', 3, 255, 32, 0);
        render_flush(&render_context);
        gpio_put(25, 1);
        // wait 2 second
        sleep_ms(2000);
    }
}