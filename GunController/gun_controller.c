#include <stdlib.h>
#include "pico/stdlib.h"
#include "types.h"
#include <string.h>
#include "graphics/renderer.h"

#define TEST_W 240
#define TEST_H 240
 
int main(void)
{
    stdio_init_all();
    uint16_t row_pixels[TEST_W];
    
    render_context_t render_context;
    memset(&render_context, 0, sizeof(render_context));
    render_init_context(&render_context, TEST_W, TEST_H, 125);
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    while (true) {
        // make screen black
        render_fill_area(&render_context, 0, 0, TEST_W - 1, TEST_H - 1, 0x0000);
        gpio_put(25, 0);
        // wait 1 second
        sleep_ms(3000);

        // make screen white
        render_fill_area(&render_context, 0, 0, TEST_W - 1, TEST_H - 1, 0xffff);
        gpio_put(25, 1);
        // wait 1 second
        sleep_ms(3000);

        // make screen ???
        render_fill_area(&render_context, 0, 0, TEST_W - 1, TEST_H - 1, 0xf800);

        // wait 1 second
        sleep_ms(3000);
    }
}
