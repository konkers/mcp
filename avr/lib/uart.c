/*
 * Copyright 2009, 2013 Erik Gilling
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

#ifdef NEW_UART
#define UDR	UDR0
#define UCSRA	UCSR0A
#define UCSRB	UCSR0B
#define UCSRC	UCSR0C
#define UBRRL	UBRR0L
#define UBRRH	UBRR0H

/* UCSRA */
#define RXC	RXC0
#define TXC	RXC0
#define UDRE	UDRE0
#define FE	FE0
#define DOR	DOR0
#define PE	UPE0
#define U2X	U2X0
#define MPCM	MPCM0

/* UCSRB */
#define RXCIE	RXCIE0
#define TXCIE	TXCIE0
#define UDRIE	UDRIE0
#define RXEN	RXEN0
#define TXEN	TXEN0
#define UCSZ2	UCSZ02
#define RXB8	RXB80
#define TXB8	TXB80

/* UCSRC */
#define URSEL	UMSEL01
#define UMSEL	UMSEL00
#define UPM1	UPM01
#define UPM0	UPM00
#define USBS	USBS0
#define UCSZ1	UCSZ01
#define UCSZ0	UCSZ00
#define UCPOL	UCPOL0

#endif

struct ringbuf {
    uint8_t		buf[128];
    uint16_t	rd;
    uint16_t	wr;
};

static inline uint16_t ringbuf_inc(struct ringbuf* rb, uint16_t index)
{
    index++;
    if (index >= sizeof(rb->buf))
        index = 0;
    return index;
}

static inline uint8_t ringbuf_empty(struct ringbuf *rb)
{
    return (rb->rd == rb->wr);
}

static inline uint8_t ringbuf_full(struct ringbuf *rb)
{
    return (rb->rd == ringbuf_inc(rb, rb->wr));
}

static inline void ringbuf_push(struct ringbuf *rb, uint8_t data)
{
    if (!ringbuf_full(rb)) {
        rb->buf[rb->wr] = data;
        rb->wr = ringbuf_inc(rb, rb->wr);
    }
}

static inline uint8_t ringbuf_pop(struct ringbuf *rb)
{
    uint8_t data = 0;
    if (!ringbuf_empty(rb)) {
        data = rb->buf[rb->rd];
        rb->rd = ringbuf_inc(rb, rb->rd);

    }
    return data;
}

static struct ringbuf uart_rb;
static uint8_t uart_busy;

void uart_init(uint16_t ubrr)
{
    /* atmegaX8 style uart */
    UBRRH = (uint8_t)(ubrr >> 8);
    UBRRL = (uint8_t)ubrr;

    /* enable receiver, transmitter, and reciever interrupt */
    // UCSRB = _BV(RXEN) | _BV(TXEN) | _BV(RXCIE);

    /* Set frame format: 8data, 1stop bit */
#ifdef NEW_UART
    UCSRC = _BV(UCSZ1) | _BV(UCSZ0);
#else
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
#endif
}

#ifdef NEW_UART
ISR( USART0_TX_vect )
#else
ISR( USART_TXC_vect )
#endif
{
    if (!ringbuf_empty(&uart_rb)) {
        UDR = ringbuf_pop(&uart_rb);
    } else {
        uart_disable_tx();
        uart_busy = 0;
    }
}


void uart_send(unsigned char data)
{
    while (!(UCSRA & _BV(UDRE))) {
    }

    UDR = data;
}

static void uart_kick(void)
{
    //	cli();
    if (!uart_busy) {
        UDR = ringbuf_pop(&uart_rb);
        uart_enable_tx();
        uart_busy = 1;
    }
    //	sei();
}

void uart_putc(unsigned char data)
{
    ringbuf_push(&uart_rb, data);
    uart_kick();
}

void uart_puts(const char *s)
{
    while (*s) {
        ringbuf_push(&uart_rb, *s);
        s++;
    }
    uart_kick();
}

void uart_puts_P(const char *s)
{
    char c;

    while ((c = pgm_read_byte(s)) != '\0') {
        ringbuf_push(&uart_rb, c);
        s++;
    }
    uart_kick();
}

const uint8_t PROGMEM hexchars[] = "0123456789ABCDEF";

void uart_printhex(uint8_t val)
{
    ringbuf_push(&uart_rb, pgm_read_byte(hexchars + (val >> 4)));
    ringbuf_push(&uart_rb, pgm_read_byte(hexchars + (val & 0xf)));
}

int uart_has_data(void)
{
    return UCSRA & _BV(RXC);
}

unsigned char uart_poll_getchar(void)
{
    while (!(UCSRA & _BV(RXC))) {
    }

    return UDR;
}
