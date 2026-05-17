#ifndef SSD1306T_H
#define SSD1306T_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
 
// ── Display geometry ─────────────────────────────────────────────────────────
#define SSD1306_W       64
#define SSD1306_H       48
#define SSD1306_PAGES   (SSD1306_H / 8)   // 6 pages
#define SSD1306_ADDR    0x3C               // common default; try 0x3D if nothing shows
 
// ── Internal framebuffer ─────────────────────────────────────────────────────
// One bit per pixel, organised as pages (rows of 8 pixels).
// Buffer layout: [page0: 64 bytes] [page1: 64 bytes] … [page5: 64 bytes]
static uint8_t ssd1306_buf[SSD1306_PAGES * SSD1306_W];
 
// ── Low-level I2C helpers ────────────────────────────────────────────────────
 
static inline void ssd1306_send_cmd(uint8_t cmd) {
    uint8_t buf[2] = { 0x00, cmd };   // 0x00 = Co=0, D/C#=0 (command)
    i2c_write_blocking(i2c0, SSD1306_ADDR, buf, 2, false);
}
 
static inline void ssd1306_send_cmd2(uint8_t cmd, uint8_t arg) {
    uint8_t buf[3] = { 0x00, cmd, arg };
    i2c_write_blocking(i2c0, SSD1306_ADDR, buf, 3, false);
}
 
// ── Initialisation ───────────────────────────────────────────────────────────
// Call once after i2c_init() and gpio_set_function() for SDA/SCL.
 
static inline void ssd1306_init(void) {
    printf("Scanning I2C...\n");
    for (int addr = 0; addr < 128; addr++) {
        uint8_t buf;
        int ret = i2c_read_blocking(i2c0, addr, &buf, 1, false);
        if (ret >= 0)
            printf("Found device at 0x%02X\n", addr);
    }
    // Minimal init sequence for 64×48.
    // The SSD1306 silicon is always 128×64; we window it to our panel size.
    ssd1306_send_cmd(0xAE);           // display off
    ssd1306_send_cmd2(0xD5, 0x80);   // clock divide ratio / oscillator
    ssd1306_send_cmd2(0xA8, SSD1306_H - 1); // mux ratio = 47
    ssd1306_send_cmd2(0xD3, 0x00);   // display offset = 0
    ssd1306_send_cmd(0x40);           // display start line = 0
    ssd1306_send_cmd2(0x8D, 0x14);   // charge pump ON (needed for most modules)
    ssd1306_send_cmd2(0x20, 0x00);   // horizontal addressing mode
    ssd1306_send_cmd(0xA1);           // segment remap (flip horizontal — remove if mirrored)
    ssd1306_send_cmd(0xC8);           // COM output scan direction (flip vertical)
    ssd1306_send_cmd2(0xDA, 0x12);   // COM pins config (alt, no remap)
    ssd1306_send_cmd2(0x81, 0xCF);   // contrast
    ssd1306_send_cmd2(0xD9, 0xF1);   // pre-charge period
    ssd1306_send_cmd2(0xDB, 0x40);   // VCOMH deselect level
    ssd1306_send_cmd(0xA4);           // output follows RAM
    ssd1306_send_cmd(0xA6);           // normal display (not inverted)
    ssd1306_send_cmd(0xAF);           // display ON
}
 
// ── Flush framebuffer → display ──────────────────────────────────────────────
// Call after drawing to push ssd1306_buf to the screen.
 
static inline void ssd1306_show(void) {
    // Set column address 0..63
    uint8_t col_cmd[4] = { 0x00, 0x21, 32, 32 + SSD1306_W - 1 };
    i2c_write_blocking(i2c0, SSD1306_ADDR, col_cmd, 4, false);
    // Set page address 0..5
    uint8_t page_cmd[4] = { 0x00, 0x22, 0, SSD1306_PAGES - 1 };
    i2c_write_blocking(i2c0, SSD1306_ADDR, page_cmd, 4, false);
 
    // Send framebuffer — prepend the data control byte 0x40
    // I2C has a max payload, so send in chunks if needed.
    // Here we send the whole buffer in one shot (384 bytes + 1 prefix = 385).
    uint8_t data[1 + SSD1306_PAGES * SSD1306_W];
    data[0] = 0x40;  // D/C# = 1 (data)
    memcpy(&data[1], ssd1306_buf, sizeof(ssd1306_buf));
    i2c_write_blocking(i2c0, SSD1306_ADDR, data, sizeof(data), false);
}
 
// ── Drawing primitives ────────────────────────────────────────────────────────
 
static inline void ssd1306_clear(void) {
    memset(ssd1306_buf, 0, sizeof(ssd1306_buf));
}
 
// Set or clear a single pixel (x: 0‥63, y: 0‥47)
static inline void ssd1306_pixel(int x, int y, bool on) {
    if (x < 0 || x >= SSD1306_W || y < 0 || y >= SSD1306_H) return;
    int page = y / 8;
    int bit  = y % 8;
    if (on)
        ssd1306_buf[page * SSD1306_W + x] |=  (1 << bit);
    else
        ssd1306_buf[page * SSD1306_W + x] &= ~(1 << bit);
}
 
// Horizontal line
static inline void ssd1306_hline(int x0, int x1, int y, bool on) {
    for (int x = x0; x <= x1; x++) ssd1306_pixel(x, y, on);
}
 
// Vertical line
static inline void ssd1306_vline(int x, int y0, int y1, bool on) {
    for (int y = y0; y <= y1; y++) ssd1306_pixel(x, y, on);
}
 
// Filled rectangle
static inline void ssd1306_fill_rect(int x, int y, int w, int h, bool on) {
    for (int row = y; row < y + h; row++)
        ssd1306_hline(x, x + w - 1, row, on);
}
 
// Unfilled rectangle
static inline void ssd1306_rect(int x, int y, int w, int h, bool on) {
    ssd1306_hline(x,         x + w - 1, y,         on);
    ssd1306_hline(x,         x + w - 1, y + h - 1, on);
    ssd1306_vline(x,         y,         y + h - 1, on);
    ssd1306_vline(x + w - 1, y,         y + h - 1, on);
}


#endif // SSD1306T_H
