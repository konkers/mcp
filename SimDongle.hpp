#ifndef __SIMDONGLE_HPP__
#define __SIMDONGLE_HPP__

#include <string>
#include <vector>

#include <chrono>

#include "Dongle.hpp"

class SimDongle : public Dongle {
private:
	std::vector<Addr> addrs;
	enum State {
		RESET,
		SKIP_ROM,
		MATCH_ROM,
		READ_SCRATCHPAD,
		CONVERTING,
	};
	State state;
	Addr matchAddr;
	int scratchPadAddr;

	std::chrono::system_clock::time_point conversionStart;

	uint16_t tempTo18b20(float temp) {
		uint16_t val = 0x0;
		// brain dead way of handling sign extension
		if (temp < 0) {
			temp = -temp;
			val = 0xf800;
		}

		val |= ((uint16_t)temp * 16) & 0x7ff;

		return val;
	}

public:
	SimDongle();
	virtual ~SimDongle();

	virtual bool connect(void);

	virtual int enumerate(void);
	virtual int reset(void);
	virtual int matchRom(const Addr addr);
	virtual int skipRom(void);
	virtual int read(void);
	virtual int readByte(void);
	virtual int writeByte(uint8_t data);

	/* HACK until OW pwm device is made */
	virtual int setPower(uint8_t power);

	virtual Addr getAddr(unsigned n);
};


#endif /* __SIMDONGLE_HPP__ */
