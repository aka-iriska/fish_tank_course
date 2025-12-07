#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <avr/pgmspace.h>

// Размер дисплея
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSD1306_ADDR 0x3C

#define SSD1306_MAX_WRITE_SIZE 32

#define PAGE_COUNT (SCREEN_HEIGHT / 8)

extern const uint8_t courier_7x8_eng[];
//extern const uint8_t courier_7x8_rus[];

// Инициализация дисплея
void oled_init();

// Очистить экран
void oled_clear();

void oled_clear_buffer();

// Напечатать текст на дисплее
// x, y - координаты начала текста
void oled_print_text(const char *text, int x = -1 , uint8_t page_index = -1);

void oled_print_char(unsigned char c, int x, int y);

// Нарисовать пиксель
void oled_draw_pixel(int x, int y);

// Обновить экран (отобразить все изменения)
void oled_update();

void oled_turnon();

void oled_turnoff();

#endif // OLED_DISPLAY_H
