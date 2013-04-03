#include <stdarg.h>
#include <stdio.h>

#include "SimDongle.hpp"

using namespace std;

#define CMD_CONVERT_T		0x44
#define CMD_WRITE_SCRATCHPAD	0x4e
#define CMD_READ_SCRATCHPAD	0xbe
#define CMD_COPY_SCRATCHPAD	0x48
#define CMD_RECAL_E2		0xb8
#define CMD_READ_PS		0xb4

SimDongle::SimDongle() : state(RESET), simTemp(65), simTempDelta(0.1), simTempMin(60), simTempMax(70)
{
	addrs.push_back(Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01));
	addrs.push_back(Addr(0x28, 0x77, 0x02, 0x8d, 0x02, 0x00, 0x00, 0x8b));
	addrs.push_back(Addr(0x28, 0x55, 0x33, 0x8d, 0x02, 0x00, 0x00, 0x1a));
	addrs.push_back(Addr(0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));
}

SimDongle::~SimDongle()
{
}

int SimDongle::enumerate(void)
{
	return addrs.size();
}

int SimDongle::reset(void)
{
	state = RESET;
	return 0;
}

int SimDongle::matchRom(const Addr addr)
{
	if (state == RESET) {
		state = MATCH_ROM;
		matchAddr = addr;
	}
	return 0;
}

int SimDongle::skipRom(void)
{
	if (state == RESET)
		state = SKIP_ROM;
	return 0;
}

int SimDongle::read(void)
{
	if (state == CONVERTING) {
		chrono::system_clock::time_point now =
			chrono::system_clock::now();
		if (now - conversionStart > chrono::milliseconds(750)) {
			state = RESET;
			return 1;
		} else {
			return 0;
		}

	}
	return 0;
}

int SimDongle::readByte(void)
{
	int data = 0;
	if (state == READ_SCRATCHPAD) {
		uint16_t val = tempTo18b20(simTemp);

		if (scratchPadAddr == 0)
			data = val & 0xff;
		else if (scratchPadAddr == 1)
			data = val >> 8;
		else
			data = 0;

		scratchPadAddr++;
	}

	return data;
}

int SimDongle::writeByte(uint8_t data)
{
	switch(state) {
	case SKIP_ROM:
		if (data == CMD_CONVERT_T)
			startConversion();

		break;
	case MATCH_ROM:
		if (data == CMD_CONVERT_T) {
			startConversion();
		} else if (data == CMD_READ_SCRATCHPAD) {
			scratchPadAddr = 0;
			state = READ_SCRATCHPAD;
		}
		break;

	default:
		break;
	}
	return 0;
}

/* HACK until OW pwm device is made */
int SimDongle::setPower(uint8_t power)
{
	return 0;
}

bool SimDongle::connect(void)
{
	return true;
}

SimDongle::Addr SimDongle::getAddr(unsigned n)
{
	if (n < addrs.size())
		return addrs[n];
	return Addr();
}
