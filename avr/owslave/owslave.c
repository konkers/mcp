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
#include <uart.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

const PROGMEM uint8_t ows_addr[8] =
{0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

struct output_cfg {
    volatile uint8_t *port;
    uint8_t mask;
} __attribute__((packed));

struct output_cfg output[] = {
    {&PORTH, _BV(4)},
    {&PORTH, _BV(3)},
    {&PORTE, _BV(3)},
    {&PORTG, _BV(5)},
    {&PORTB, _BV(6)},
    {&PORTB, _BV(7)},
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
    uart_puts_P(PSTR("w "));
    uart_printhex(idx);
    uart_puts_P(PSTR("="));
    uart_printhex(data);
    uart_puts_P(PSTR("\n"));

    switch(idx) {
        case 0:
            output_set(data);
            break;
    }
}

uint8_t ows_dev_get_scratchpad_read_size(void)
{
    return 0;
}

uint8_t ows_dev_scratchpad_read(uint8_t idx) {
    return 0;
}

int main(void)
{
    DDRE |= _BV(3);
    DDRG |= _BV(5);
    DDRH |= _BV(4) | _BV(3);
    DDRB |= _BV(4) | _BV(5) | _BV(6) | _BV(7);

    uart_init(uart_baud(FOSC, 9600));
    uart_puts_P(PSTR("poop\n"));
    ows_init(&PINL, &PORTL, &DDRL, _BV(1));
    sei();
    while (1) {
    }
}
