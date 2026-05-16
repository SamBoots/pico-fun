#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "screendrivers/st7789.h"
#include "Graphics/font8x16.h"
#include "types.h"

#define TEST_W 240
#define TEST_H 240
 
int main(void)
{
    stdio_init_all();
    uint16_t row_pixels[TEST_W];
    
    render_context_t render_context;
    memset(&render_context, 0, sizeof(render_context));
    st7789_init(&render_context, TEST_W, TEST_H, 125);

    st7789_fill(&render_context, 0x0000);

    while (1) {
        int rand_y = rand() % TEST_H;
        uint16_t rand_color = rand() % 0xffff;
        
        st7789_set_cursor(&render_context, 0, rand_y, TEST_W, TEST_H);

        for (int i = 0; i < TEST_W; i++) {
            row_pixels[i] = rand_color;
        }

        st7789_write(&render_context, row_pixels, sizeof(row_pixels));
    }
}
