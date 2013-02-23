

#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <uart.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#undef OWS_PHY_DEBUG
#undef OWS_NET_DEBUG

#define OW_TIMER	5

#define _TCCRA(x)	TCCR ## x ## A
#define _TCCRB(x)	TCCR ## x ## B
#define _TCCRC(x)	TCCR ## x ## C
#define _TCNT(x)	TCNT ## x
#define _OCRA(x)	OCR ## x ## A
#define _OCRB(x)	OCR ## x ## B
#define _OCRC(x)	OCR ## x ## C
#define _ICR(x)		ICR ## x
#define _TIMSK(x)	TIMSK ## x
#define _TIFR(x)	TIFR ## x

#define _TIMER_CAPT_vect(x)	TIMER ## x ##_CAPT_vect
#define _TIMER_COMPA_vect(x)	TIMER ## x ##_COMPA_vect

#if (OW_TIMER == 5)
#define TCCRA	_TCCRA(5)
#define TCCRB	_TCCRB(5)
#define TCCRC	_TCCRC(5)
#define TCNT	_TCNT(5)
#define OCRA	_OCRA(5)
#define OCRB	_OCRB(5)
#define OCRC	_OCRV(5)
#define ICR	_ICR(5)
#define TIMSK	_TIMSK(5)
#define TIFR	_TIFR(5)

#define TIMER_CAPT_vect		_TIMER_CAPT_vect(5)
#define TIMER_COMPA_vect	_TIMER_COMPA_vect(5)
#else
#error "unkown OW_TIMER value"
#endif

#define USEC(us)	((us) * 16)

enum ows_dev_commands {
	OWS_DEV_CMD_WRITE_SCRATCHPAD = 0x4e,
};

enum ows_dev_state {
	OWS_DEV_CMD,
	OWS_DEV_WRITE_SCRATCHPAD,
};

uint8_t ows_dev_scratchpad[1];
uint8_t ows_dev_scratchpad_idx;
uint8_t ows_dev_byte;
uint8_t ows_dev_do_commit;
enum ows_dev_state ows_dev_state;

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

static void ows_dev_scratchpad_update(uint8_t idx, uint8_t data)
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

static uint8_t ows_dev_handle_cmd(uint8_t byte)
{
	switch(byte) {
	case OWS_DEV_CMD_WRITE_SCRATCHPAD:
		ows_dev_state = OWS_DEV_WRITE_SCRATCHPAD;
		return 0;
	}

	return 0;
}

static uint8_t ows_dev_handle_write_scratchpad(uint8_t byte)
{
	if (ows_dev_scratchpad_idx < ARRAY_SIZE(ows_dev_scratchpad))
		ows_dev_scratchpad_update(ows_dev_scratchpad_idx, byte);

	ows_dev_scratchpad_idx++;

	return 0;
}

static uint8_t ows_dev_handle_byte(uint8_t byte)
{
	switch(ows_dev_state) {
	case OWS_DEV_CMD:
		return ows_dev_handle_cmd(byte);

	case OWS_DEV_WRITE_SCRATCHPAD:
		ows_dev_byte = byte;
		ows_dev_do_commit = 1;
		return 0;
	}

	return 0;
}

static void ows_dev_commit(void)
{
	if (ows_dev_do_commit)
		ows_dev_handle_write_scratchpad(ows_dev_byte);

	ows_dev_do_commit = 0;
}

static uint8_t ows_dev_get_write_data(void)
{
	return 0;
}

static void ows_dev_reset(void)
{
	ows_dev_state = OWS_DEV_CMD;
	ows_dev_scratchpad_idx = 0;
	ows_dev_do_commit = 0;
}

enum ows_net_cmds {
	OWS_NET_CMD_READ_ROM = 0x33,
	OWS_NET_CMD_SKIP_ROM = 0xCC,
	OWS_NET_CMD_MATCH_ROM = 0x55,
	OWS_NET_CMD_SEARCH_ROM = 0xF0,
};

enum ows_net_state {
	OWS_NET_COMMAND,
	OWS_NET_SEARCH_ROM,
	OWS_NET_MATCH_ROM,
	OWS_NET_ADDRESSED,
};

// XXX: make progmem and conver all refs to readprogmem
static uint8_t ows_addr[8] =
{0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static uint8_t ows_net_cur_bit;
static uint8_t ows_net_bit;
static uint8_t ows_net_byte;
static uint8_t ows_net_addressed;
static enum ows_net_state ows_net_state;

static void ows_net_reset(void)
{
	ows_net_bit = 0;
	ows_net_byte = 0;
	ows_net_addressed = 0;
	ows_net_state = OWS_NET_COMMAND;
	ows_dev_reset();
}

static uint8_t _shift[8] = {
	1 << 0,
	1 << 1,
	1 << 2,
	1 << 3,
	1 << 4,
	1 << 5,
	1 << 6,
	1 << 7,
};

static inline uint8_t ows_net_addr_bit(uint8_t bit)
{
	uint8_t addr_bit;

	addr_bit = ows_addr[bit >> 3];
	addr_bit &= _shift[bit & 0x7];

	return addr_bit;
}

static void ows_net_handle_search_rom_data(void)
{
	if (ows_net_addressed) {
		ows_net_cur_bit = ows_net_addr_bit(ows_net_bit);
		if (ows_net_cur_bit)
			ows_net_byte = 1; /* 01b */
		else
			ows_net_byte = 2; /* 10b */
	} else {
		ows_net_byte = 3; /* 11b */
	}
}

static uint8_t ows_net_handle_data_bit(uint8_t bit)
{
	ows_net_byte >>= 1;
	ows_net_byte |= bit ? 0x80 : 0x0l;
	ows_net_bit++;

	if (ows_net_bit == 8) {
		ows_net_bit = 0;
		return 1;
	} else {
		return 0;
	}
}

static uint8_t ows_net_handle_command(int bit)
{
	if (ows_net_handle_data_bit(bit)) {
#ifdef OWS_NET_DEBUG
		uart_puts_P(PSTR("cmd "));
		uart_printhex(ows_net_byte);
		uart_puts_P(PSTR("\n"));
#endif

		switch (ows_net_byte)
		{
		case OWS_NET_CMD_READ_ROM:
			break;

		case OWS_NET_CMD_SKIP_ROM:
			ows_net_state = OWS_NET_ADDRESSED;
			break;

		case OWS_NET_CMD_MATCH_ROM:
			ows_net_bit = 0;
			ows_net_addressed = 1;
			ows_net_state = OWS_NET_MATCH_ROM;
			break;

		case OWS_NET_CMD_SEARCH_ROM:
			ows_net_bit = 0;
			ows_net_addressed = 1;
			ows_net_handle_search_rom_data();
			ows_net_state = OWS_NET_SEARCH_ROM;
			return 2;
			break;
		}
	}

	return 0;
}

static uint8_t ows_net_handle_addressed(int bit)
{
	if (ows_net_handle_data_bit(bit))
		return ows_dev_handle_byte(ows_net_byte);

	return 0;
}

static uint8_t ows_net_handle_match_rom(uint8_t bit)
{
	if (!!ows_net_addr_bit(ows_net_bit) == bit)
		ows_net_addressed &= 1;
	else
		ows_net_addressed = 0;

	if (++ows_net_bit == 64) {
		ows_net_bit = 0;
		ows_net_state = ows_net_addressed ? OWS_NET_ADDRESSED : OWS_NET_COMMAND;
	}

	return 0;
}

static uint8_t ows_net_handle_search_rom(uint8_t bit)
{
	if (!!ows_net_cur_bit == bit)
		ows_net_addressed &= 1;
	else
		ows_net_addressed = 0;

	if (++ows_net_bit == 64) {
		ows_net_bit = 0;
		ows_net_state = ows_net_addressed ? OWS_NET_ADDRESSED : OWS_NET_COMMAND;
		return 0;
	} else {
		ows_net_handle_search_rom_data();
		return 2;
	}
}

static uint8_t ows_net_get_write_data(void)
{
	uint8_t data;
	if (ows_net_state == OWS_NET_ADDRESSED) {
		data = ows_dev_get_write_data();
	} else {
		data = ows_net_byte & 0x1;
		ows_net_byte = ows_net_byte >> 1;
	}

	return data;
}

/* assumes only low bit will be set */
static uint8_t ows_net_handle_data(uint8_t bit)
{
#ifdef OWS_NET_DEBUG
	uart_puts_P(PSTR("b"));
	uart_printhex(ows_net_bit);
	uart_puts_P(PSTR(" "));
	uart_putc(bit ? '1' : '0');
	uart_puts_P(PSTR("\n"));
#endif

	switch (ows_net_state) {
	case OWS_NET_COMMAND:
		return ows_net_handle_command(bit);

	case OWS_NET_ADDRESSED:
		return ows_net_handle_addressed(bit);

	case OWS_NET_MATCH_ROM:
		return ows_net_handle_match_rom(bit);

	case OWS_NET_SEARCH_ROM:
		return ows_net_handle_search_rom(bit);
	}

	return 0;
}

static void ows_net_commit(void)
{
	if (ows_net_state == OWS_NET_ADDRESSED)
		ows_dev_commit();
}

static uint8_t ows_phy_read(void)
{
	return PINL & _BV(1);
}

static void ows_phy_drive(uint8_t level)
{
	if (level) {
		DDRL &= ~_BV(1);
	} else {
		PORTL &= ~_BV(1);
		DDRL |= _BV(1);
	}
}

enum ows_phy_state {
	OWS_PHY_IDLE = 0,
	OWS_PHY_RESET0 = 1,
	OWS_PHY_RESET1 = 2,
	OWS_PHY_RESET2 = 3,
	OWS_PHY_SAMPLE0 = 4,
	OWS_PHY_SAMPLE1 = 5,
	OWS_PHY_WRITE = 6,
};

static enum ows_phy_state ows_phy_state;
static uint16_t ows_phy_cycle_start;
static uint8_t ows_phy_write_bytes;

typedef void (*ows_phy_irq_handler)(void);

static ows_phy_irq_handler ows_phy_capture_handler;
static ows_phy_irq_handler ows_phy_compare_handler;

static void ows_phy_null_handler(void)
{
}

static void ows_phy_trigger_edge(uint8_t edge, ows_phy_irq_handler handler)
{
	uint8_t timsk;

#ifdef OWS_PHY_DEBUG
	uart_puts_P(PSTR("e "));
	uart_printhex(edge);
	uart_puts_P(PSTR("\n"));
#endif

	if (edge)
		TCCRB |= _BV(ICES1);
	else
		TCCRB &= ~_BV(ICES1);

	timsk = TIMSK;
	timsk |= _BV(ICIE1);
	timsk &= ~_BV(OCIE1A);
	TIMSK = timsk;

	ows_phy_capture_handler = handler;
	ows_phy_compare_handler = ows_phy_null_handler;
}

static void ows_phy_trigger_counter(uint16_t counter, ows_phy_irq_handler handler)
{
	uint8_t timsk;

	OCRA = counter;

	timsk = TIMSK;
	timsk &= ~_BV(ICIE1);
	timsk |= _BV(OCIE1A);
	TIMSK = timsk;

	ows_phy_capture_handler = ows_phy_null_handler;
	ows_phy_compare_handler = handler;
}

static void ows_phy_trigger_time(uint16_t when, ows_phy_irq_handler handler)
{
#ifdef OWS_PHY_DEBUG
	uart_puts_P(PSTR("t "));
	uart_printhex(when >> 8);
	uart_printhex(when & 0xff);
	uart_puts_P(PSTR("\n"));
#endif

	ows_phy_trigger_counter(ows_phy_cycle_start + when, handler);
}

static void ows_phy_trigger_delay(uint16_t delay, ows_phy_irq_handler handler)
{
#ifdef OWS_PHY_DEBUG
	uart_puts_P(PSTR("d "));
	uart_printhex(delay >> 8);
	uart_printhex(delay & 0xff);
	uart_puts_P(PSTR("\n"));
#endif

	ows_phy_trigger_counter(TCNT + delay, handler);
}

static void ows_phy_trigger_both(uint8_t edge, uint16_t when,
				 ows_phy_irq_handler capture_handler,
				 ows_phy_irq_handler compare_handler)
{
#ifdef OWS_PHY_DEBUG
	uart_puts_P(PSTR("b "));
	uart_printhex(edge);
	uart_puts_P(PSTR(" "));
	uart_printhex(when >> 8);
	uart_printhex(when & 0xff);
	uart_puts_P(PSTR("\n"));
#endif

	if (edge)
		TCCRB |= _BV(ICES1);
	else
		TCCRB &= ~_BV(ICES1);

	OCRA = ows_phy_cycle_start + when;

	TIMSK |= _BV(ICIE1) | _BV(OCIE1A);

	ows_phy_capture_handler = capture_handler;
	ows_phy_compare_handler = compare_handler;
}

static void ows_phy_handle_idle(void);
static void ows_phy_handle_reset0(void);
static void ows_phy_handle_reset1(void);
static void ows_phy_handle_reset2(void);
static void ows_phy_handle_sample0(void);
static void ows_phy_handle_sample1_capture(void);
static void ows_phy_handle_sample1_compare(void);
static void ows_phy_handle_write(void);

static void ows_phy_reset(void)
{
	ows_net_reset();
	ows_phy_write_bytes = 0;
	ows_phy_state = OWS_PHY_RESET0;
	ows_phy_trigger_edge(1, ows_phy_handle_reset0);
	if (ows_phy_read())
		ows_phy_handle_reset0();
}

static void ows_phy_handle_idle(void)
{
	ows_phy_cycle_start = ICR;

	/* debounce */
	/* XXX: this should not be needed */
	if (ows_phy_read())
		return;

	if (ows_phy_write_bytes > 0) {
		ows_phy_drive(ows_net_get_write_data());
		ows_phy_trigger_time(USEC(20), ows_phy_handle_write);
		ows_phy_write_bytes--;
	} else {
		/* between 15 and 60 uS */
		ows_phy_trigger_time(USEC(37), ows_phy_handle_sample0);
	}
}

static void ows_phy_handle_reset0(void)
{
	ows_phy_trigger_delay(USEC(37), ows_phy_handle_reset1);
}

static void ows_phy_handle_reset1(void)
{
	ows_phy_drive(0);
	ows_phy_trigger_delay(USEC(150), ows_phy_handle_reset2);
}

static void ows_phy_handle_reset2(void)
{
	ows_phy_drive(1);
	ows_phy_trigger_edge(0, ows_phy_handle_idle);
}

static void ows_phy_handle_sample0(void)
{
	if (ows_phy_read()) {
		ows_phy_trigger_edge(0, ows_phy_handle_idle);
		ows_phy_write_bytes = ows_net_handle_data(1);
		ows_net_commit();
	} else {
		ows_phy_state = OWS_PHY_SAMPLE1;
		ows_phy_write_bytes = ows_net_handle_data(0);
		/* between 120 and 480 uS */
		ows_phy_trigger_both(1, USEC(300),
				     ows_phy_handle_sample1_capture,
				     ows_phy_handle_sample1_compare);
	}
}

static void ows_phy_handle_sample1_capture(void)
{
	ows_phy_trigger_edge(0, ows_phy_handle_idle);
	ows_net_commit();
}
static void ows_phy_handle_sample1_compare(void)
{
	ows_phy_reset();
}

static void ows_phy_handle_write(void)
{
	ows_phy_drive(1);
	ows_phy_trigger_edge(0, ows_phy_handle_idle);
}

ISR(TIMER_CAPT_vect)
{
	ows_phy_capture_handler();
}

ISR(TIMER_COMPA_vect)
{
	ows_phy_compare_handler();
}

void ows_init(void)
{
	/* no output compare */
	/* normal waveform generation wgm[3:0] = 0000b */
	TCCRA = 0x0;

	/* divide the 16Mhz clock by 8 to get .5uS ticks CS[2:0] = 010b*/
	/* enable Input Capture Noise ICNC1 = 1 */
	/* set input capture edge select to falling ICES1 = 0 */
	TCCRB = _BV(ICNC1) | _BV(CS10);

	ows_phy_reset();
}


int main(void)
{
	DDRE |= _BV(3);
	DDRG |= _BV(5);
	DDRH |= _BV(4) | _BV(3);
	DDRB |= _BV(4) | _BV(5) | _BV(6) | _BV(7);

	uart_init(uart_baud(FOSC, 9600));
	uart_puts_P(PSTR("poop\n"));
	ows_init();
	sei();
	while (1) {
	}
}
