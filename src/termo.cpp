#include <termo.h>

// https://arduinomaster.ru/datchiki-arduino/arduino-ds18b20/

/*
 * Формат кадра
 * LSB | 2^3 | 2^2 | 2^1 | 2^0 | 2^-1 | 2^-2 | 2^-3 | 2^-4 |
 * MSB |  S  |  S  |  S  |  S  |  S   |  2^6 |  2^5 |  2^4 |
 * 
 * Необходима только одна цифра после запятой и 1 NEGATIVE bit -> 11 бит -> R1=0, R0 = 1, коэффициент = 0.25
*/
OneWire termo(ONE_WIRE_PIN);

void termo_init()
{

    termo.reset();         // сброс шины: всех предыдущих команд и параметров
    termo.write(SKIP_ROM); // пропустить поиск ROM

    // Настройка разрешения (R1=0, R0=1 → 10 бит)
    termo.write(WRITE_MEM);
    termo.write(0x4B); // TH (не используется)
    termo.write(0x46); // TL (не используется)
    termo.write(0x3F); // Конфигурация: 0(01)1 1111b → 10 бит

    termo.reset();

    Serial.println("[DEBUG] DS18B20 initialized (10-bit mode).");
}

float read_temp()
{
    byte data[2];

    termo.reset();
    termo.skip();               // Один датчик — адрес можно пропустить
    termo.write(CONVERT_T); // Начало измерения
    delay(190);               // ждём 10-битное преобразование (макс. 187.5 мс)

    termo.reset();
    termo.skip();
    termo.write(READ_MEM);
    data[0] = termo.read(); // Читаем младший байт значения температуры
    data[1] = termo.read(); // А теперь старший
    termo.reset();
    
    int16_t raw = (data[1] << 8) | data[0];
    // для 10 бит (LSB = 0.25°C)
    float celsius = (raw >> 2) * 0.25;
    return celsius;
}