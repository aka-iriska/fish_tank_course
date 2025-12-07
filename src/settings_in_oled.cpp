#include "settings.h"

unsigned long last_btn_press = 0;

static uint8_t current_setting = 0; // выбор между temp, clean, tds, read, show
MenuState currentMenuState = STATE_MAIN_MENU;

// Проверка нажатия кнопки
bool is_button_pressed(uint8_t pin)
{
    if (!(PIND & (1 << pin)))
    {
        if (millis() - last_btn_press > 200)
        {
            last_btn_press = millis();
            settings_timer = millis(); // Сбрасываем таймер автовыхода
            return true;
        }
    }
    return false;
}

void check_display()
{
    switch (currentMenuState)
    {
    case STATE_MAIN_MENU:
        display_menu();   // Рисуем меню
        check_nav_menu(); // Слушаем кнопки навигации по меню
        check_yes_menu();
        break;

    case STATE_EDIT_TEMP:
        change_temp();
        break;

    case STATE_EDIT_CLEAN:
        change_clean();
        break;

    case STATE_EDIT_TDS:
        change_tds();
        break;

    case STATE_EDIT_READ:
        change_read_interval();
        break;

    case STATE_SHOW_INFO:
        show_settings();
        break;
    }
}
void display_menu()
{
    oled_print_text("SETTINGS", 0, 0);
    if (current_setting == 0)
        oled_print_text("! Temp", 0, 1);
    else
        oled_print_text("Temp", 0, 1);
    if (current_setting == 1)
        oled_print_text("! Clearness", 0, 2);
    else
        oled_print_text("Clearness", 0, 2);
    if (current_setting == 2)
        oled_print_text("! TDS", 0, 3);
    else
        oled_print_text("TDS", 0, 3);
    if (current_setting == 3)
        oled_print_text("! Read time", 0, 4);
    else
        oled_print_text("Read time", 0, 4);
    if (current_setting == 4)
        oled_print_text("! SHOW", 0, 5);
    else
        oled_print_text("SHOW", 0, 5);
}

void check_nav_menu()
{
    if (is_button_pressed(BTN_NAV))
    {
        current_setting = (current_setting + 1) % 5;
    }
}

void check_yes_menu()
{
    if (is_button_pressed(BTN_YES))
    {
        switch (current_setting)
        {
        case 0:
            currentMenuState = STATE_EDIT_TEMP;
            break;
        case 1:
            currentMenuState = STATE_EDIT_CLEAN;
            break;
        case 2:
            currentMenuState = STATE_EDIT_TDS;
            break;
        case 3:
            currentMenuState = STATE_EDIT_READ;
            break;
        case 4:
            currentMenuState = STATE_SHOW_INFO;
            break;
        }
    }
}

void change_temp()
{
    uint8_t temp = currentSettings.min_temp;
    char message[32];

    snprintf(message, sizeof(message), "%d", temp);
    oled_print_text(message, 0, 1);

    if (is_button_pressed(BTN_NAV))
    {
        temp += 5;
        if (temp > TEMP_NAX)
            temp = TEMP_MIN;
    }

    currentSettings.min_temp = temp;
    changed = true;

    if (is_button_pressed(BTN_YES))
    {
        currentMenuState = STATE_MAIN_MENU;
    }
}
void change_clean()
{
    uint8_t temp = currentSettings.min_clean;
    char message[32];

    snprintf(message, sizeof(message), "%d", temp);
    oled_print_text(message, 0, 2);

    if (is_button_pressed(BTN_NAV))
    {
        temp += 10;
        if (temp > 100)
            temp = 0;
    }

    currentSettings.min_clean = temp;
    changed = true;

    if (is_button_pressed(BTN_YES))
    {
        currentMenuState = STATE_MAIN_MENU;
    }
}
void change_tds()
{
    uint16_t temp = currentSettings.max_tds;

    switch (temp)
    {
    case SOFT_MAX:
        oled_print_text("SOFT", 0, 3);
        if (is_button_pressed(BTN_NAV))
            temp = HARD_MAX;
        break;
    case HARD_MAX:
        oled_print_text("HARD", 0, 3);
        if (is_button_pressed(BTN_NAV))
            temp = SALTY_MAX;
        break;
    case SALTY_MAX:
        oled_print_text("SALTY", 0, 3);
        if (is_button_pressed(BTN_NAV))
            temp = SOFT_MAX;
        break;
    default:
        break;
    }

    switch (temp)
    {
    case SOFT_MAX:
        currentSettings.min_tds = TDS_MIN;
        currentSettings.max_tds = temp;
        break;
    case HARD_MAX:
        currentSettings.min_tds = TDS_MIN;
        currentSettings.max_tds = temp;
        break;
    case SALTY_MAX:
        currentSettings.min_tds = SALTY_MIN;
        currentSettings.max_tds = temp;
        break;
    default:
        break;
    }
    changed = true;

    if (is_button_pressed(BTN_YES))
    {
        currentMenuState = STATE_MAIN_MENU;
    }
}
void change_read_interval()
{
    uint16_t temp = currentSettings.read_interval;
    char message[32];

    snprintf(message, sizeof(message), "%u", temp);
    oled_print_text(message, 0, 4);

    if (is_button_pressed(BTN_NAV))
    {
        temp += 5000;
        if (temp > 60000)
            temp = READ_MIN;
    }

    currentSettings.read_interval = temp;
    changed = true;

    if (is_button_pressed(BTN_YES))
    {
        currentMenuState = STATE_MAIN_MENU;
    }
}
void show_settings()
{

    display_measurement("MIN T", currentSettings.min_temp, "", 1, false);
    display_measurement("MIN CLEAN", currentSettings.min_clean, "", 2, false);
    switch (currentSettings.max_tds)
    {
    case SOFT_MAX:
        oled_print_text("TDS MODE: SOFT", 0, 3);
        break;
    case HARD_MAX:
        oled_print_text("TDS MODE: HARD", 0, 3);
        break;
    case SALTY_MAX:
        oled_print_text("TDS MODE: SALT", 0, 3);
        break;
    default:
        break;
    }
    display_measurement("READ INT", currentSettings.read_interval, "", 4, false);

    if (is_button_pressed(BTN_YES))
    {
        currentMenuState = STATE_MAIN_MENU;
    }
}
