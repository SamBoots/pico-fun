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

    while (1) {
        // make screen black
        st7789_fill(&render_context, 0x0000);

        // wait 1 second
        sleep_ms(3000);

        // make screen white
        st7789_fill(&render_context, 0xffff);

        // wait 1 second
        sleep_ms(3000);

        // make screen ???
        st7789_fill(&render_context, 0xf800);

        // wait 1 second
        sleep_ms(3000);
    }
}
