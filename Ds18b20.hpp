#ifndef __DS18B20_HPP__
#define __DS18B20_HPP__

#include "Dongle.hpp"

class Ds18b20 {
private:
	Dongle *dongle;
	Dongle::Addr addr;

public:
	Ds18b20(Dongle *dongle, Dongle::Addr addr);

	void startConversion(void);
	void startAllConversion(void);

	bool isConversionDone(void);

	float getTemp(void);
	Dongle::Addr getAddr(void) {
		return addr;
	}
};

#endif /* __DS18B20_HPP__ */
