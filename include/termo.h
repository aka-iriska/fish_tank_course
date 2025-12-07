// termometer DS18B20
#ifndef TERMO_H
#define TERMO_H

#include <OneWire.h>

#define ONE_WIRE_PIN 7

/*
 * ALARM SEARCH 0xEC
 * 0x44 измерение, сохранение в TH, TL, конвертация, переход в неактивный режим
 * 0xBE чтение памяти
 * 0x4E запись в память
*/
#define CONVERT_T  0x44
#define READ_MEM 0xBE
#define WRITE_MEM 0x4E
#define SKIP_ROM   0xCC
#define ALARM_SEARCH 0xEC

extern OneWire termo;

void termo_init();

float read_temp();

#endif // TERMO_H

