#ifndef __OWIO_HPP__
#define __OWIO_HPP__

#include "Dongle.hpp"

class OwIO {
private:
	Dongle *dongle;
	Dongle::Addr addr;

public:
	OwIO(Dongle *dongle, Dongle::Addr addr);

	void setOut(void);
	uint8_t getIn(void);

	Dongle::Addr getAddr(void) {
		return addr;
	}
};

#endif /* __OWIO_HPP__ */
