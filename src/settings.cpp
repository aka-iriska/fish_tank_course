#include "settings.h"
#include <EEPROM.h>

Settings currentSettings;

volatile bool settings_mode = false;
volatile bool changed = false;

volatile bool alarm_temp = false;
volatile bool alarm_clean = false;
volatile bool alarm_tds = false;

unsigned long settings_timer = 0;
const unsigned long SETTINGS_TIMEOUT = 60000; // 60 секунд бездействия


void settings_init()
{
    currentSettings.initialized = false;
    EEPROM.get(0, currentSettings);

    // Первый запуск -> значения по умолчанию
    if (currentSettings.initialized != 13) // в EEPROM мусор какой-то, еси там не МОЁ значение 13, то значит настройки ещё не были инициализированы
    {                                      // не bool, потому что он 255 или 0, что как раз обычно и в EEPROM,  в общем на bool полагаться не надо
        currentSettings.min_temp = 30.0;
        currentSettings.min_tds = 0;
        currentSettings.max_tds = 400;
        currentSettings.min_clean = 20;
        currentSettings.read_interval = 5000;
        currentSettings.initialized = 13;
        save_settings(); // Записываем дефолт
        Serial.println(F("[DEBUG] Settings reset to default"));
    }
    else
    {
        Serial.println(F("[DEBUG] Settings loaded"));
    }
}

void save_settings()
{
    EEPROM.put(0, currentSettings);
    changed = false;
    Serial.println(F("[DEBUG] Settings saved to EEPROM"));
}

void setup_settings()
{
    while (settings_mode)
    {
        if (settings_timer == 0)
        {
            settings_timer = millis();
            Serial.println(F("--- ENTER SETTINGS MODE (15s timeout) ---"));
            Serial.println(F("Send: T 25, S 1, C 20, R 5000, ?"));
            Serial.println("S: 1 - soft (max: 150 ppm), 2 - hard (max: 400 ppm), 3 - salty (>1000 ppm)");
        }

        check_console_commands();
        check_display();

        if (millis() - settings_timer > SETTINGS_TIMEOUT)
        {
            settings_mode = false;
            settings_timer = 0; // Сброс
            Serial.println(F("--- EXIT SETTINGS MODE ---"));
        }
    }
    if (changed)
        {
            save_settings(); // Сохраняем в EEPROM сразу после изменения
        }
    oled_clear();
}

void check_console_commands()
{
    if (Serial.available() > 0) // проверяем получили ли мы хоть что-то
    {
        settings_timer = millis();
        char cmd = Serial.read();          // читаем первый символ (команда)
        float value = Serial.parseFloat(); // читаем число после буквы (если оно есть)

        while (Serial.available())
            Serial.read(); // Чистим буфер от лишних символов (например, переноса строки)

        changed = false;

        switch (toupper(cmd)) // to upper case
        {
        case 'T': // Температура (Min Temp)
            if (value > TEMP_NAX)
            {
                Serial.println(F("[ERROR] Temp limit cannot be > 40C!"));
            }
            else if (value < TEMP_MIN)
            {
                Serial.println(F("[ERROR] Temp cannot be negative!"));
            }
            else
            {
                currentSettings.min_temp = (uint8_t)value;
                Serial.print(F("SET Min Temp: "));
                Serial.println(currentSettings.min_temp);
                changed = true;
            }
            break;

        case 'S': // TDS (Max TDS)
            switch ((int)value)
            {
            case 1:
                currentSettings.min_tds = TDS_MIN;
                currentSettings.max_tds = SOFT_MAX;
                Serial.print(F("SET TDS mode: soft"));
                changed = true;
                break;
            case 2:
                currentSettings.min_tds = TDS_MIN;
                currentSettings.max_tds = HARD_MAX;
                Serial.print(F("SET TDS mode: hard"));
                changed = true;
                break;
            case 3:
                currentSettings.min_tds = SALTY_MIN;
                currentSettings.max_tds = SALTY_MAX;
                Serial.print(F("SET TDS mode: salty"));
                changed = true;
                break;
            default:
                Serial.println(F("[ERROR] There is no such mode."));
                break;
            }

        case 'C': // Чистота (Min Clean %)
            if (value < 0 || value > 100)
            {
                Serial.println(F("[ERROR] Clean must be 0-100%"));
            }
            else
            {
                currentSettings.min_clean = (uint8_t)value;
                Serial.print(F("SET Min Clean %: "));
                Serial.println(currentSettings.min_clean);
                changed = true;
            }
            break;

        case 'R': // Интервал чтения
            if (value < READ_MIN)
            {
                Serial.print(F("[ERROR] Interval too short! Min 5000ms"));
            }
            else
            {
                currentSettings.read_interval = (uint16_t)value;
                Serial.print(F("SET Read Interval: "));
                Serial.println(currentSettings.read_interval);
                changed = true;
            }
            break;

        case '?': // Запрос текущих настроек
            Serial.println(F("--- CURRENT SETTINGS ---"));
            Serial.print(F("Min temp (T):  "));
            Serial.println(currentSettings.min_temp);
            Serial.print(F("Max TDS (S):   "));
            Serial.println(currentSettings.max_tds);
            Serial.print(F("Min clean (C): "));
            Serial.println(currentSettings.min_clean);
            Serial.print(F("Read interval (R): "));
            Serial.print(currentSettings.read_interval);
            Serial.println("ms");
            Serial.println(F("------------------------"));
            break;

        default:
            // Игнорируем перенос строки и мусор
            if (cmd != '\n' && cmd != '\r' && cmd != ' ')
            {
                Serial.println(F("[ERROR] Unknown command. Use: T 25, S 800, C 20, R 5000, ?"));
            }
            break;
        }
    }
}