#include <Arduino.h>
#include <avr/sleep.h> // Для функций сна
#include <avr/power.h> // Для отключения периферии

#include "oled_display.h"
#include "termo.h"
#include "util.h"
#include "settings.h"

/*
 * Analog - PC
 * digital - PD
 * остальные - PB
 */

#define F_CPU 16000000UL // 16 МГц
#define BUZZER_PIN 3     // PD3
#define BTN_TURN 4       // PD4
#define TDS A0           // PC0
#define TURBIDITY A1     // PC1
#define HEATER_RELAY 0   // PB0
#define T_LED 2          // PB2

unsigned long last_read_time = 0;
unsigned long last_interrupt_time = 0;     // Время последнего прерывания
const unsigned long timer_duration = 1000; // как долго будет пищать баззер при alarm

// put function declarations here:
void setup_timer2_ctc();
void setup_interrupts();
void setup_int0();

void work();
void make_sleep();

void check_termometer();
void check_turbidity();
void check_tds();
void check_timer2(unsigned long current_time);

ISR(INT0_vect)
{
  if (millis() - last_interrupt_time > 400)
  {
    last_interrupt_time = millis();
    settings_mode = !settings_mode;
    currentMenuState = MenuState::STATE_MAIN_MENU;
    Serial.print("SETTINGS ");
    if (settings_mode)
      Serial.println("ON");
    else
      Serial.println("OFF");
  }
}

// прерывание на любой пин из группы PCINT2 (мне надо PD4)
ISR(PCINT2_vect)
{
  if (millis() - last_interrupt_time > 200)
  {
    settings_mode = false;
    last_interrupt_time = millis();
    Serial.println("TURN ON/OFF");
  }
}

void setup()
{

  DDRB = 0x00;
  DDRB |= (1 << T_LED) | (1 << HEATER_RELAY); // для тестового LED на вывод
  PORTB &= ~(1 << HEATER_RELAY);
  PORTB |= (1 << T_LED);

  DDRD = 0b00000000;                                                               // D0–D7, на ввод
  PORTD = (1 << BTN_SETTINGS) | (1 << BTN_TURN) | (1 << BTN_NAV) | (1 << BTN_YES); // кнопка нажата - 0, кнопка отжата - 1

  Serial.begin(115200);

  setup_timer2_ctc();
  setup_interrupts();

  oled_init(); // Инициализация дисплея
  oled_clear();

  termo_init();

  settings_init();
}

void loop()
{
  if (!(PIND & (1 << BTN_TURN))) // make on (PD2 = 0)
  {
    PORTB &= ~(1 << T_LED);
    if (settings_mode)
    {
      setup_settings();
    }
    else
    {
      work();
    }
  }
  else // make off
  {
    make_sleep();
  }
}

void work()
{
  PORTB |= (1 << PB1);
  unsigned long current_time = millis();
  if (current_time - last_read_time >= currentSettings.read_interval)
  {
    last_read_time = current_time;

    check_termometer();
    check_turbidity();
    check_tds();
  }

  check_timer2(current_time);
}

// put function definitions here:

void setup_interrupts()
{
  PCICR |= (1 << PCIE2);    // Включаем группу 2 (в том числе PD4)
  PCMSK2 |= (1 << PCINT20); // Включаем маску для PD4

  EICRA &= ~((1 << ISC00) | (1 << ISC01)); // очистить
  EICRA |= (1 << ISC00) | (1 << ISC01);    // установить RISING EDGE

  setup_int0();
}

void setup_int0()
{
  EIMSK |= (1 << INT0); // включаем прерывание INT0
}

void setup_timer2_ctc()
{
  DDRD |= (1 << BUZZER_PIN); // PD3 = OC2B — как выход

  TCCR2A = (1 << COM2B0); // Toggle OC2B on compare match
  TCCR2A |= (1 << WGM21); // CTC mode (Clear Timer on Compare Match)

  OCR2A = 255; // Значение сравнения (макс = 255)

  TCCR2B = 0; // Пока не включаем такймер
}

void check_termometer()
{
  // Считываем температуру с DS18B20
  float temperature = read_temp();

  display_measurement("T", temperature, "\x7F C", 0);

  if (temperature < currentSettings.min_temp)
  {
    PORTB |= (1 << HEATER_RELAY); // ВКЛЮЧАЕМ НАГРЕВАТЕЛЬ
    if (!alarm_temp)
    {
      alarm_temp = true;
      Serial.println(F("[WARN] ALARM: Too cold!"));
    }
    oled_print_text("COLD!!", SCREEN_WIDTH / 2, 1);
  }
  else if (temperature >= (currentSettings.min_temp + 0.5))
  {
    PORTB &= ~(1 << HEATER_RELAY); // Подаем LOW на D8 (выключаем реле)

    if (alarm_temp)
    {
      oled_print_text("", 0, 1);
      alarm_temp = false;
    }
  }
}

void check_turbidity()
{
  uint16_t sensorValue = analogRead(TURBIDITY); // 0-1023, где 0-0V, 1023-5V, clear - 473, 474

  // float voltage = sensorValue * (5.0 / 1024.0); // 2.31 Convert the analog reading (which goes from 0 – 1023) to a voltage (0 – 5V):
  sensorValue = constrain(sensorValue, 300, 474);
  uint8_t unturbidity = (uint8_t)map(sensorValue, 300, 474, 0, 100); // проценты чистоты
  display_measurement("Clean", unturbidity, "%", 2);

  // Проверка порога
  if (unturbidity < currentSettings.min_clean)
  {
    if (!alarm_clean)
    {
      alarm_clean = true;
      Serial.println(F("[WARN] ALARM: Too dirty!"));
    }

    oled_print_text("DIRTY!!", SCREEN_WIDTH / 2, 3);
  }
  else if (alarm_clean)
  {
    oled_print_text("", 0, 3);
    alarm_clean = false;
  }
}

void check_tds()
{
  uint16_t sensorValue = analogRead(TDS);       // 0-1023, где 0-0V, 1023-5V
  float voltage = sensorValue * (5.0 / 1024.0); // Переводим 0-1023 в 0-5.0 Вольт

  // uint16_t tds_value = map(sensorValue, 0, 471, 0, 1000);
  // Если 2.3В = 1000 ppm, то коэффициент = 1000 / 2.3 ≈ 434.7
  float tdsFactor = 434.7;
  uint16_t tds_value = (uint16_t)(voltage * tdsFactor);

  display_measurement("TDS", tds_value, "ppm", 4);

  Serial.print("[DEBUG] Raw TDS: ");
  Serial.println(sensorValue);

  // Проверка порога
  if (tds_value > currentSettings.max_tds)
  {
    if (!alarm_tds) // не спамим в monitor
    {
      alarm_tds = true;
      Serial.println(F("[WARN] ALARM: High TDS!"));
    }

    oled_print_text("HIGH!!", SCREEN_WIDTH / 2, 5);
  }
  else if (tds_value < currentSettings.min_tds)
  {
    if (!alarm_tds) // не спамим в monitor
    {
      alarm_tds = true;
      Serial.println(F("[WARN] ALARM: LOW TDS!"));
    }

    oled_print_text("LOW!!", SCREEN_WIDTH / 2, 5);
  }
  else if (alarm_tds)
  {
    oled_print_text("", 0, 5);
    alarm_tds = false;
  }
}

void check_timer2(unsigned long current_time)
{
  if ((alarm_temp || alarm_clean || alarm_tds) && (current_time - last_read_time <= timer_duration) && (!settings_mode))
  {
    // Предделитель: 64 → частота ≈ 16_000_000 / (2 * 64 * (255 + 1)) ≈ 490 Гц
    TCCR2B = (1 << CS22); // Предделитель = 64 (CS22=1, CS21=0, CS20=0)
  }
  else
  {
    // остановка таймера
    TCCR2B = 0;
  }
}

void make_sleep()
{
  oled_turnoff();
  PORTB |= (1 << T_LED);
  PORTB &= ~(1 << HEATER_RELAY);

  TCCR2B = 0;

  EIMSK &= ~((1 << INT1) | (1 << INT0)); // Отключение прерывания INT0 (Кнопка настроек)

  ADCSRA &= ~(1 << ADEN); // Выключить АЦП полностью
  power_adc_disable();    // PRR
  power_timer0_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_spi_disable();
  power_usart0_disable();
  power_twi_disable(); // i2c - oled

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Самый глубокий сон - SMCR
  sleep_enable();                      // Разрешаем сон
  sei();                               // Разрешаем прерывания (для кнопки ON/OFF на PD4)
  sleep_cpu();                         // Сон :)

  // Точка, куда возвращаемся после TURN ON
  sleep_disable(); // Запрещаем сон

  // PCMSK2 &= ~(1 << PCINT20); // Выключаем прерывание PCINT
  setup_int0();

  power_all_enable(); // Включаем всё обратно
  ADCSRA |= (1 << ADEN);
  oled_turnon();

  PORTB &= ~(1 << T_LED);
}
