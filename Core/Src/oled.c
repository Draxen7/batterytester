//
// Created by Szymon on 19.10.2026.
//

#include "oled.h"

#include "battery.h"
#include "i2c.h"

static const uint8_t width = 128;
static const uint8_t height = 64;
static const uint16_t oled_addr = 0x3C << 1;
static const uint8_t command_byte = 0x00;
static const uint8_t pixels_byte = 0x40;
static uint8_t pixels[1024];
static const uint8_t init_cmds[] = {
    0xAE,       //OFF
    0xD5, 0x80, //CLOCK, 80
    0xA8, 0x3F, //ROWS, HOW MANY
    0xD3, 0x00, //VERTICAL SHIFT, NONE
    0x40,       //START AT ROW 0
    0x8D, 0x14, //CHARGE PUMP, ON
    0x20, 0x00, //ADDRESS MODE, HORIZONTAL
    0xA1,       //FLIP LEFT-RIGHT
    0xC8,       //FLIP UP-DOWN
    0xDA, 0x12, //ROW DRIVERS, 128x64
    0x81, 0xCF, //CONTRAST, 207/255
    0xA4,       //SHOW PIXEL MEMORY
    0xA6,       //NON INVERT
    0xAF        //ON
    };
static const uint8_t redefine_start_cmd[] = {
    0x21, 0x00, 0x7F,
    0x22, 0x00, 0x07
    };
static const uint8_t outer_box_x_start = 5;
static const uint8_t outer_box_x_end = 88;
static const uint8_t outer_box_y_start = 16;
static const uint8_t outer_box_y_end = 47;
static const uint8_t terminal_x_start = outer_box_x_end;
static const uint8_t terminal_x_end = 93;
static const uint8_t terminal_y_start = 27;
static const uint8_t terminal_y_end = 36;
static const uint8_t inner_box_x_start = 7;
static const uint8_t inner_box_x_end = 86;
static const uint8_t inner_box_y_start = 18;
static const uint8_t inner_box_y_end = 45;
static const uint8_t text_box_x_start = 99;
// static const uint8_t text_box_x_end = 122;
// static const uint8_t text_box_y_start = 28;
// static const uint8_t text_box_y_end = 35;
static const uint8_t glyphs[10][5] = {
    /* 0
       .###.
       #...#
       #...#
       #...#
       #...#
       #...#
       .###. */
    { 0b00111110, 0b01000001, 0b01000001, 0b01000001, 0b00111110 },
    /* 1
       ..#..
       .##..
       ..#..
       ..#..
       ..#..
       ..#..
       .###. */
    { 0b01000000, 0b01000010, 0b01111111, 0b01000000, 0b01000000 },
    /* 2
       .###.
       #...#
       ....#
       ...#.
       ..#..
       .#...
       ##### */
    { 0b01100010, 0b01010001, 0b01001001, 0b01000101, 0b01000010 },
    /* 3
       .###.
       #...#
       ....#
       ..##.
       ....#
       #...#
       .###. */
    { 0b00100010, 0b01000001, 0b01001001, 0b01001001, 0b00110110 },
    /* 4
       ...#.
       ..##.
       .#.#.
       #..#.
       #####
       ...#.
       ...#. */
    { 0b00011000, 0b00010100, 0b00010010, 0b01111111, 0b00010000 },
    /* 5
       #####
       #....
       ####.
       ....#
       ....#
       #...#
       .###. */
    { 0b00100111, 0b01000101, 0b01000101, 0b01000101, 0b00111001 },
    /* 6
       .###.
       #...#
       #....
       ####.
       #...#
       #...#
       .###. */
    { 0b00111110, 0b01001001, 0b01001001, 0b01001001, 0b00110010 },
    /* 7
       #####
       ....#
       ...#.
       ..#..
       .#...
       .#...
       .#... */
    { 0b00000001, 0b01110001, 0b00001001, 0b00000101, 0b00000011 },
    /* 8
       .###.
       #...#
       #...#
       .###.
       #...#
       #...#
       .###. */
    { 0b00110110, 0b01001001, 0b01001001, 0b01001001, 0b00110110 },
    /* 9
       .###.
       #...#
       #...#
       .###.
       ....#
       #...#
       .###. */
    { 0b00100110, 0b01001001, 0b01001001, 0b01001001, 0b00110110 },
};
/* %
   #...#
   #..#.
   ...#.
   ..#..
   .#...
   .#..#
   #...# */
static const uint8_t glyph_percent[5] = {
    0b01000011, 0b00110000, 0b00001000, 0b00000110, 0b01100001
};
static const uint8_t glyph_A[5] = { 0b01111110, 0b00001001, 0b00001001, 0b00001001, 0b01111110 };
static const uint8_t glyph_L[5] = { 0b01111111, 0b01000000, 0b01000000, 0b01000000, 0b01000000 };
static const uint8_t glyph_K[5] = { 0b01111111, 0b00001000, 0b00010100, 0b00100010, 0b01000001 };
static const uint8_t glyph_N[5] = { 0b01111111, 0b00000010, 0b00000100, 0b00001000, 0b01111111 };
static const uint8_t glyph_I[5] = { 0b01000001, 0b01000001, 0b01111111, 0b01000001, 0b01000001 };
static const uint8_t glyph_M[5] = { 0b01111111, 0b00000010, 0b00000100, 0b00000010, 0b01111111 };
static const uint8_t glyph_H[5] = { 0b01111111, 0b00001000, 0b00001000, 0b00001000, 0b01111111 };
static const uint8_t glyph_V[5] = { 0b00001111, 0b00110000, 0b01000000, 0b00110000, 0b00001111 };

static const uint8_t *const alk_1v5[]  = { glyph_A, glyph_L, glyph_K, 0, glyphs[1], glyph_V, glyphs[5] };
static const uint8_t *const nimh_1v2[] = { glyph_N, glyph_I, glyph_M, glyph_H, 0, glyphs[1], glyph_V, glyphs[2] };
static const uint8_t *const alk_9v[]   = { glyph_A, glyph_L, glyph_K, 0, glyphs[9], glyph_V };
static const uint8_t *const nimh_8v4[] = { glyph_N, glyph_I, glyph_M, glyph_H, 0, glyphs[8], glyph_V, glyphs[4] };
static const uint8_t *const alk_12v[]  = { glyph_A, glyph_L, glyph_K, 0, glyphs[1], glyphs[2], glyph_V };
static const uint8_t *const li_3v6[]   = { glyph_L, glyph_I, 0, glyphs[3], glyph_V, glyphs[6] };

static const struct {
    const uint8_t *const *glyphs;
    uint8_t count;
} labels[] = {
    { alk_1v5,  7 },
    { nimh_1v2, 8 },
    { alk_9v,   6 },
    { nimh_8v4, 8 },
    { alk_12v,  7 },
    { li_3v6,   6 }
};

void oled_init(void) {
    HAL_I2C_Mem_Write(&hi2c1,
        oled_addr,
        command_byte,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t *) init_cmds
       ,
        sizeof init_cmds
       ,
        100);
}
void update_screen(void) {
    HAL_I2C_Mem_Write(&hi2c1,
        oled_addr,
        command_byte,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t *)redefine_start_cmd,
        sizeof redefine_start_cmd,
        100);
    HAL_I2C_Mem_Write(&hi2c1,
        oled_addr,
        pixels_byte,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t *) pixels,
        sizeof pixels,
        200);
}
void set_pixel(const uint8_t x, const uint8_t y){
    // 8 horizontal pages, 128 bytes/columns, each bit = pixel
    // x -> column in page,
    // y/8 -> page -> * 128 -> address of page,
    // y % 8 -> bit number
    pixels[x + (y/8)*128] |= 1 << (y % 8);
}
void erase_pixel(const uint8_t x, const uint8_t y) {
    pixels[x + (y/8)*128] &= ~(1 << (y % 8));
}
void erase_screen(void) {
    for (size_t i = 0; i < sizeof pixels; i++) {
        pixels[i] = 0;
    }
}
void draw_glyph(const uint8_t glyph[5], const uint8_t x, const uint8_t y) {
    for (uint8_t col = 0; col < 5; col++) {
        for (uint8_t row = 0; row < 8; row++) {
            if (glyph[col] & (1 << row)) {
                set_pixel(x + col, y + row);
            }
        }
    }
}
static void draw_mode_label(const uint8_t *const label[], uint8_t count) {
    const uint8_t width = count * 6 - 1;
    uint8_t cursor = (128 - width) / 2;
    const uint8_t y = (outer_box_y_start - 8) / 2;
    for (uint8_t i = 0; i < count; i++) {
        if (label[i] != 0) {
            draw_glyph(label[i], cursor, y);
        }
        cursor += 6;
    }
}
static void display_mode(const uint8_t mode) {
    draw_mode_label(labels[mode].glyphs, labels[mode].count);
}
void display_charge(const uint8_t mode, const uint8_t percent, const uint16_t pack_mv) {
    const uint8_t fill = percent * (inner_box_x_end - inner_box_x_start + 1) / 100;
    const uint8_t hundreds = percent / 100;
    const uint8_t tens = (percent / 10) % 10;
    const uint8_t ones = percent % 10;
    const uint8_t digits = percent >= 10 ? percent >= 100 ? 3 : 2 : 1;
    const uint8_t text_width = digits * 6 + 5;
    uint8_t cursor = text_box_x_start + (24 - text_width) / 2;
    const uint16_t tenths_total = (pack_mv + 50) / 100;
    const uint8_t tenths = tenths_total % 10;
    const uint8_t volts = tenths_total / 10;
    const uint8_t volt_places = volts >= 10 ? 2 : 1;
    const uint8_t volt_width = volt_places * 6 + 11;
    uint8_t volt_cursor = text_box_x_start + (24 - volt_width) / 2;
    const uint8_t block_height = 20;
    const uint8_t percent_y = outer_box_y_start
        + (outer_box_y_end - outer_box_y_start + 1 - block_height) / 2;
    const uint8_t voltage_y = percent_y + 12;
    erase_screen();
    for (uint8_t x = 0; x < width; x++) {
        for (uint8_t y = 0; y < height; y++) {
            if ((x == outer_box_x_start || x == outer_box_x_end) && y >= outer_box_y_start && y <= outer_box_y_end) {
                set_pixel(x,y);
            }
            if (x > outer_box_x_start && x < outer_box_x_end && (y == outer_box_y_start || y == outer_box_y_end)) {
                set_pixel(x,y);
            }
            if ((x == terminal_x_start || x == terminal_x_end) && y >= terminal_y_start && y <= terminal_y_end) {
                set_pixel(x,y);
            }
            if (x > terminal_x_start && x < terminal_x_end && (y == terminal_y_start || y == terminal_y_end)) {
                set_pixel(x,y);
            }
            if (x >= inner_box_x_start &&
                x < inner_box_x_start + fill &&
                y >= inner_box_y_start && y <= inner_box_y_end) {
                set_pixel(x, y);
            }
        }
    }
    switch (digits) {
        case 3:
            draw_glyph(glyphs[hundreds], cursor, percent_y);
            cursor += 6;
        case 2:
            draw_glyph(glyphs[tens], cursor, percent_y);
            cursor += 6;
        default:
            draw_glyph(glyphs[ones], cursor, percent_y);
            cursor += 6;
            draw_glyph(glyph_percent, cursor, percent_y);
    }
    switch (volt_places) {
        case 2:
            draw_glyph(glyphs[volts / 10], volt_cursor, voltage_y);
            volt_cursor += 6;
        default:
            draw_glyph(glyphs[volts % 10], volt_cursor, voltage_y);
            volt_cursor += 6;
            draw_glyph(glyph_V, volt_cursor, voltage_y);
            volt_cursor += 6;
            draw_glyph(glyphs[tenths], volt_cursor, voltage_y);
    }
    display_mode(mode);
    update_screen();
}
