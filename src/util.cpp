#include "util.h"

void display_measurement(const char *label, float value, const char *unit, uint8_t page_index)
{
    char buffer[7];               // Буфер для преобразования float
    dtostrf(value, 5, 2, buffer); // 4 - минимальная ширина (с учетом знака и точки), 1 - знак после запятой

    char message[32];
    snprintf(message, sizeof(message), "%s: %s %s", label, buffer, unit);

    // Вывод Serial
    Serial.print("[INFO] ");
    Serial.println(message);
    oled_print_text(message, 0, page_index);
}

void display_measurement(const char *label, uint16_t value, const char *unit, uint8_t page_index, bool monitor)
{
    char message[32];
    snprintf(message, sizeof(message), "%s: %u %s", label, value, unit);
    if (monitor)
    {
        Serial.print("[INFO] ");
        Serial.println(message);
    }
    oled_print_text(message, 0, page_index);
}

void display_measurement(const char *label, uint8_t value, const char *unit, uint8_t page_index, bool monitor)
{
    char message[32];
    snprintf(message, sizeof(message), "%s: %u %s", label, value, unit);
    if (monitor)
    {
        Serial.print("[INFO] ");
        Serial.println(message);
    }
    oled_print_text(message, 0, page_index);
}