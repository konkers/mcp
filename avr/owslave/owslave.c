

#include <avr/interrupt.h>
#include <avr/wdt.h>


#include <uart.h>

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

static uint8_t ows_dev_handle_byte(uint8_t byte)
{
	return 0;
}

static uint8_t ows_dev_get_write_data(void)
{
	return 0;
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
{0xe0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

static uint8_t ows_net_cur_bit;
static uint8_t ows_net_bit;
static uint8_t ows_net_byte;
static uint8_t ows_net_addressed;
static enum ows_net_state ows_net_state;

static void ows_net_reset(void)
{
	ows_net_bit = 0;
	ows_net_byte = 0;
	ows_net_state = OWS_NET_COMMAND;
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
	ows_net_byte |= bit << 7;
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
		ows_net_addressed |= 1;
	else
		ows_net_addressed = 0;

	if (++ows_net_bit == 64)
		ows_net_state = ows_net_addressed ? OWS_NET_ADDRESSED : OWS_NET_COMMAND;

	return 0;
}

static uint8_t ows_net_handle_search_rom(uint8_t bit)
{
	if (!!ows_net_cur_bit == bit)
		ows_net_addressed |= 1;
	else
		ows_net_addressed = 0;

	if (++ows_net_bit == 64) {
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

static uint8_t ows_phy_read(void)
{
	return PINL & _BV(1);
}

static void ows_phy_drive(uint8_t level)
{
	if (level) {
		DDRL &= ~_BV(1);
//		PORTL |= _BV(0);
	} else {
		PORTL &= ~(_BV(1) /*| _BV(0)*/);
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

static void ows_phy_trigger_edge(uint8_t edge)
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
}

static void ows_phy_trigger_counter(uint16_t counter)
{
	uint8_t timsk;

	OCRA = counter;

	timsk = TIMSK;
	timsk &= ~_BV(ICIE1);
	timsk |= _BV(OCIE1A);
	TIMSK = timsk;
}

static void ows_phy_trigger_time(uint16_t when)
{
#ifdef OWS_PHY_DEBUG
	uart_puts_P(PSTR("t "));
	uart_printhex(when >> 8);
	uart_printhex(when & 0xff);
	uart_puts_P(PSTR("\n"));
#endif

	ows_phy_trigger_counter(ows_phy_cycle_start + when);
}

static void ows_phy_trigger_delay(uint16_t delay)
{
#ifdef OWS_PHY_DEBUG
	uart_puts_P(PSTR("d "));
	uart_printhex(delay >> 8);
	uart_printhex(delay & 0xff);
	uart_puts_P(PSTR("\n"));
#endif

	ows_phy_trigger_counter(TCNT + delay);
}

static void ows_phy_trigger_both(uint8_t edge, uint16_t when)
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
}


static void ows_phy_handle_reset0(uint8_t capture, uint8_t timeout);
static void ows_phy_reset(void)
{
	ows_net_reset();
	ows_phy_write_bytes = 0;
	ows_phy_state = OWS_PHY_RESET0;
	ows_phy_trigger_edge(1);
	if (ows_phy_read())
		ows_phy_handle_reset0(0, 0);
}

static void ows_phy_handle_idle(uint8_t capture, uint8_t timeout)
{
	ows_phy_cycle_start = ICR;

	/* debounce */
	if (ows_phy_read())
		return;


	if (ows_phy_write_bytes > 0) {
		ows_phy_drive(ows_net_get_write_data());
		PORTL |= _BV(0);
		ows_phy_state = OWS_PHY_WRITE;
		ows_phy_trigger_time(USEC(20));
		ows_phy_write_bytes--;
	} else {
		ows_phy_state = OWS_PHY_SAMPLE0;
		/* between 15 and 60 uS */
		ows_phy_trigger_time(USEC(37));
	}
}

static void ows_phy_handle_reset0(uint8_t capture, uint8_t timeout)
{
	ows_phy_state = OWS_PHY_RESET1;
	ows_phy_trigger_delay(USEC(37));
}

static void ows_phy_handle_reset1(uint8_t capture, uint8_t timeout)
{
	ows_phy_drive(0);
	ows_phy_state = OWS_PHY_RESET2;
	ows_phy_trigger_delay(USEC(150));
}

static void ows_phy_handle_reset2(uint8_t capture, uint8_t timeout)
{
	ows_phy_drive(1);
	ows_phy_state = OWS_PHY_IDLE;
	ows_phy_trigger_edge(0);
}

static void ows_phy_handle_sample0(uint8_t capture, uint8_t timeout)
{
	if (ows_phy_read()) {
		ows_phy_trigger_edge(0);
		ows_phy_write_bytes = ows_net_handle_data(1);
		ows_phy_state = OWS_PHY_IDLE;
	} else {
		ows_phy_state = OWS_PHY_SAMPLE1;
		/* between 120 and 480 uS */
		ows_phy_trigger_both(1, USEC(300));
	}
}

static void ows_phy_handle_sample1(uint8_t capture, uint8_t timeout)
{
	if (capture) {
		ows_phy_trigger_edge(0);
		ows_phy_write_bytes = ows_net_handle_data(0);
		ows_phy_state = OWS_PHY_IDLE;
	} else {
		ows_phy_reset();
	}
}

static void ows_phy_handle_write(uint8_t capture, uint8_t timeout)
{
	ows_phy_drive(1);
	ows_phy_state = OWS_PHY_IDLE;
	ows_phy_trigger_edge(0);
}

void ows_phy_handle_state(uint8_t capture, uint8_t timeout)
{
#ifdef OWS_PHY_DEBUG
	uint8_t prev_state = ows_phy_state;
#endif

	switch (ows_phy_state) {
	case OWS_PHY_IDLE:
		ows_phy_handle_idle(capture, timeout);
		break;

	case OWS_PHY_RESET0:
		ows_phy_handle_reset0(capture, timeout);
		break;

	case OWS_PHY_RESET1:
		ows_phy_handle_reset1(capture, timeout);
		break;

	case OWS_PHY_RESET2:
		ows_phy_handle_reset2(capture, timeout);
		break;

	case OWS_PHY_SAMPLE0:
		ows_phy_handle_sample0(capture, timeout);
		break;

	case OWS_PHY_SAMPLE1:
		ows_phy_handle_sample1(capture, timeout);
		break;

	case OWS_PHY_WRITE:
		ows_phy_handle_write(capture, timeout);
		break;

	}
	PORTL |= _BV(0);


#ifdef OWS_PHY_DEBUG
	uart_printhex((capture << 4) | timeout);
	uart_puts_P(PSTR(":"));
	uart_printhex(prev_state);
	uart_puts_P(PSTR(">"));
	uart_printhex(ows_phy_state);
	uart_puts_P(PSTR("\n"));
#endif

}

ISR(TIMER_CAPT_vect)
{
	PORTL &= ~_BV(0);
	ows_phy_handle_state(1, 0);
}

ISR(TIMER_COMPA_vect)
{
	PORTL &= ~_BV(0);
	ows_phy_handle_state(0, 1);
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

	DDRL |= _BV(0);

	ows_phy_reset();
}


int main(void)
{
	uart_init(uart_baud(FOSC, 9600));
	uart_puts_P(PSTR("poop\n"));
	ows_init();
	sei();
	while (1) {
		
	}
}
