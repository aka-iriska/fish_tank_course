#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

#include "oled_display.h"
#include "util.h"

#define TEMP_MIN 0
#define TEMP_NAX 40
#define TDS_MIN 0
#define SOFT_MAX 150
#define HARD_MAX 400
#define SALTY_MIN 800
#define SALTY_MAX 10000
#define READ_MIN 5000

#define BTN_YES 6      // PD6
#define BTN_NAV 5      // PD5
#define BTN_SETTINGS 2 // PD2

extern volatile bool settings_mode;
extern volatile bool changed;
extern volatile bool alarm_temp;
extern volatile bool alarm_clean;
extern volatile bool alarm_tds;
extern unsigned long settings_timer;

enum MenuState
{
  STATE_MAIN_MENU,
  STATE_EDIT_TEMP,
  STATE_EDIT_CLEAN,
  STATE_EDIT_TDS,
  STATE_EDIT_READ,
  STATE_SHOW_INFO
};

extern MenuState currentMenuState;

struct Settings
{
  uint8_t min_temp;
  uint16_t max_tds;
  uint16_t min_tds;
  uint8_t min_clean;
  uint16_t read_interval; // как часто считываем сигналы с датчиков
  uint8_t initialized;    // Флаг, чтобы понять, первый ли это запуск
};

extern Settings currentSettings;

void settings_init();
void save_settings();
void setup_settings();
void check_console_commands();
void check_display();

void display_menu();
void check_nav_menu();
void check_yes_menu();
void change_temp();
void change_clean();
void change_tds();
void change_read_interval();
void show_settings();

#endif // SETTINGS_H