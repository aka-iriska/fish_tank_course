#ifndef UTIL_H
#define UTIL_H

#include "oled_display.h"

void display_measurement(const char* label, float value, const char* unit, uint8_t page_index);
void display_measurement(const char* label, uint16_t value, const char* unit, uint8_t page_index, bool monitor = true);
void display_measurement(const char* label, uint8_t value, const char* unit, uint8_t page_index, bool monitor = true);

#endif // UTIL_H