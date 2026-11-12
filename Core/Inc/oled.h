//
// Created by Szymon on 19.10.2026.
//

#ifndef OLED_H
#define OLED_H
#include <stdint.h>

void oled_init(void);
void update_screen(void);
void set_pixel(uint8_t x, uint8_t y);
void erase_pixel(uint8_t x, uint8_t y);
void erase_screen(void);
void draw_glyph(const uint8_t glyph[5], uint8_t x, uint8_t y);
void display_charge(uint8_t mode, uint8_t percent, uint16_t pack_mv);
#endif //OLED_H
