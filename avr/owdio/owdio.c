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

#include <owslave.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

const PROGMEM uint8_t ows_addr[8] =
{0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};

struct output_cfg {
    volatile uint8_t *port;
    uint8_t mask;
} __attribute__((packed));

struct output_cfg output[] = {
    {&PORTC, _BV(0)},
    {&PORTC, _BV(1)},
    {&PORTC, _BV(2)},
    {&PORTC, _BV(3)},
    {&PORTC, _BV(4)},
    {&PORTC, _BV(5)},
    {&PORTD, _BV(0)},
    {&PORTD, _BV(1)},
};

static void output_set(uint8_t data)
{
    uint8_t mask;
    uint8_t i;

    for(i = 0, mask = 0x01; i < ARRAY_SIZE(output); i++, mask <<= 1) {
        if (data & mask)
            *output[i].port |= output[i].mask;
        else
            *output[i].port &= ~output[i].mask;
    }
}

void ows_dev_scratchpad_update(uint8_t idx, uint8_t data)
{
    switch(idx) {
        case 0:
            output_set(data);
            break;
    }
}

int main(void)
{
    DDRC |= _BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5);
    DDRD |= _BV(0) | _BV(1);

    ows_init(&PINB, &PORTB, &DDRB, _BV(0));

    sei();
    while (1) {
    }
}
