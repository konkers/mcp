/*
 * Copyright 2014 Erik Gilling <konkers@konkers.net>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the Licene.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h>

#include <owslave.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

const PROGMEM uint8_t ows_addr[8] =
{0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02};

struct pin_cfg {
    volatile uint8_t *port;
    volatile uint8_t *pin;
    volatile uint8_t *ddr;
    uint8_t mask;
} __attribute__((packed));

struct pin_cfg pins[] = {
    {&PORTC, &PINC, &DDRC, _BV(0)},
    {&PORTC, &PINC, &DDRC, _BV(1)},
    {&PORTC, &PINC, &DDRC, _BV(2)},
    {&PORTC, &PINC, &DDRC, _BV(3)},
    {&PORTC, &PINC, &DDRC, _BV(4)},
    {&PORTC, &PINC, &DDRC, _BV(5)},
    {&PORTD, &PIND, &DDRD, _BV(0)},
    {&PORTD, &PIND, &DDRD, _BV(1)},
};

#define OWDIO_REG_PWM0      0
#define OWDIO_REG_PWM1      1
#define OWDIO_REG_PWM2      2
#define OWDIO_REG_PWM3      3
#define OWDIO_REG_PWM4      4
#define OWDIO_REG_PWM5      5
#define OWDIO_REG_PWM6      6
#define OWDIO_REG_PWM7      7
#define OWDIO_REG_DIR       8

uint8_t in_registers_staging[9];
uint8_t in_registers[9];
volatile uint8_t in_registers_update;

uint8_t out_register;

void ows_dev_scratchpad_update(uint8_t idx, uint8_t data)
{
    if (idx < ARRAY_SIZE(in_registers_staging)) {
        in_registers_staging[idx] = data;

        if (idx == (ARRAY_SIZE(in_registers_staging) - 1)) {
            in_registers_update = 1;
        }
    }
}

uint8_t ows_dev_get_scratchpad_read_size(void)
{
    return 1;
}

uint8_t ows_dev_scratchpad_read(uint8_t idx)
{
    if (idx == 0) {
        return out_register;
    } else {
        return 0x0;
    }
}

int main(void)
{
    DDRC |= _BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5);
    DDRD |= _BV(0) | _BV(1);

    ows_init(&PINB, &PORTB, &DDRB, _BV(0));

    sei();
    while (1) {
        uint8_t mask;
        uint8_t i;
        if (in_registers_update) {
            memcpy(in_registers, in_registers_staging, sizeof(in_registers));
            in_registers_update = 0;

            mask = 0x1;
            for (i = 0; i < ARRAY_SIZE(pins); i++) {
                if (in_registers[OWDIO_REG_DIR] & mask) {
                    *pins[i].ddr |= pins[i].mask;
                } else {
                    *pins[i].ddr &= ~pins[i].mask;
                }

                if (in_registers[i]) {
                    *pins[i].port |= pins[i].mask;
                } else {
                    *pins[i].port &= ~pins[i].mask;
                }

                mask = mask << 1;
            }
        }

        uint8_t temp_reg = 0x0;
        mask = 0x1;

        for (i = 0; i < ARRAY_SIZE(pins); i++) {
            if (*pins[i].pin & pins[i].mask) {
                temp_reg |= mask;
            }

            mask = mask << 1;
        }
        out_register = temp_reg;
    }
}
