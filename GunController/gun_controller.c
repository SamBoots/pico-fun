#define TEST_W 240
#define TEST_H 240
 
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "graphics/drivers/ssd1306.h"
 
#define SDA_PIN  4
#define SCL_PIN  5
#define I2C_FREQ 400000   // 400 kHz fast mode
 
int main(void) {
    stdio_init_all();
    printf("Starting...\n");
 
    // I2C setup
    i2c_init(i2c0, I2C_FREQ);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);   // SSD1306 needs pull-ups; use external 4k7 for reliability
    gpio_pull_up(SCL_PIN);   // internal pull-ups are weak but fine for short wires
 
    sleep_ms(100);           // let the display power up
 
    ssd1306_init();
    ssd1306_clear();
 
    // Draw something
    ssd1306_rect(0, 0, 64, 48, true);   // border
    ssd1306_hline(0, 63, 24, true);     // horizontal line through the middle
    ssd1306_pixel(32, 24, true);        // centre dot
 
    ssd1306_show();                      // push buffer to screen
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    while (true) {
    gpio_put(25, 1); sleep_ms(250);
    gpio_put(25, 0); sleep_ms(250);
    }
}
