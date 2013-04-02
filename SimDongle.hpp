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
