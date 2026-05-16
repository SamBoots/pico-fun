#include <stdlib.h>
#include "pico/stdlib.h"
#include "screendrivers/st7789.h"
#include "Graphics/font8x16.h"

#define TEST_W 240
#define TEST_H 240
 
int main(void)
{
    stdio_init_all();
    uint16_t row_pixels[TEST_W];
 
    st7789_init(SPI_PORT, TEST_W, TEST_H, 125);

    st7789_fill(0x0000, TEST_W, TEST_H);

    const uint8_t* fonts = font8x16[0];

    while (1) {
        int rand_y = rand() % TEST_H;
        uint16_t rand_color = rand() % fonts[0];
        
        st7789_set_cursor(0, rand_y, TEST_W, TEST_H);

        for (int i = 0; i < TEST_W; i++) {
            row_pixels[i] = rand_color;
        }

        st7789_write(row_pixels, sizeof(row_pixels));
    }
}
